/************************************************************************/
/* Author      : SoberPeng 2017-07-24
/* Description :
/************************************************************************/
#include "cflvoverhttp.h"

#include "vzconn/base/vsocket.h"
#include "vzconn/base/clibevent.h"
#include "vzconn/buffer/cblockbuffer.h"

#include "vzbase/helper/stdafx.h"
#include "vzbase/base/vmessage.h"

#include "systemv/shm/vzshm_c.h"
#include "systemv/flvmux/cflvmux.h"
#include "web_server/clistenmessage.h"

CFlvOverHttp::CFlvOverHttp()
  : exit_flag_(0)
  , evt_loop_(NULL)
  , sock_(NULL)
  , evt_send_()
  , send_data_()
  , evt_timer_()
  , shm_vdo_()
  , flv_shm_()
  , flv_data_(NULL)
  , flv_data_size_(0)
  , file(NULL) {

}

CFlvOverHttp::~CFlvOverHttp() {
  evt_timer_.Stop();
  evt_send_.Stop();
  if (evt_loop_) {
    evt_loop_ = NULL;
  }

  if (flv_data_) {
    delete[] flv_data_;
    flv_data_ = NULL;
  }

  printf("%s[%d].\n", __FUNCTION__, __LINE__);
}

bool CFlvOverHttp::Open(SOCKET sock, vzconn::EVT_LOOP *evt_loop,
                        const char* shm_key, unsigned int shm_size) {
  sock_ = sock;
  set_socket_nonblocking(sock);

  evt_loop_ = evt_loop;

  evt_send_.Init(evt_loop_, EvtSend, this);
  evt_timer_.Init(evt_loop_, EvtTimer, this);

  bool ret = shm_vdo_.Open(shm_key, shm_size);
  if (false == ret) {
    LOG(L_ERROR) << "shm video failed.";
    return false;
  }

  // file = fopen("./test2.flv", "wb+");
  return true;
}

void CFlvOverHttp::Close() {
  exit_flag_ = 1;
  // sock_.SetSocket(INVALID_SOCKET);
}

int CFlvOverHttp::AsyncHeader(const void *phead, unsigned int nhead) {
  AsyncWrite(phead, nhead);

  unsigned int width=0, height=0;
  shm_vdo_.GetVdoSize(&width, &height);

  if (NULL == flv_data_) {
    flv_data_size_ = width * height + 1024;
    flv_data_ = new char[width * height + 1024];
  }
  if (NULL == flv_data_) {
    LOG(L_ERROR) << "create flv buffer failed.";
    return -1;
  }

  int ndata = 0;
  char sdata[2048] = {0};
  char *p_dst = flv_shm_.HeaderAndMetaDataTag(sdata,
                width, height,
                16000, 8000, 16, 1);
  ndata = p_dst - sdata;
  AsyncWrite(sdata, ndata);
  if (file) {
    fwrite(sdata, 1, ndata, file);
  }

  int n_sps = 0, n_pps = 0;
  ndata = shm_vdo_.ReadHead(sdata, 2048, &n_sps, &n_pps);
  LOG(L_INFO)<<"sps length "<<n_sps
             <<" pps length "<<n_pps
             <<" width " << width
             <<" height "<< height;

  ndata = flv_shm_.MakeAVCc(sdata, n_sps, n_pps);
  p_dst = flv_shm_.Packet(avcc_data_, NULL, 0,
                          flv_shm_.avcc_, flv_shm_.avcc_size_,
                          0x09, 0);
  avcc_data_size_ = p_dst - avcc_data_;
  AsyncWrite(avcc_data_, avcc_data_size_);
  if (file) {
    fwrite(avcc_data_, 1, avcc_data_size_, file);
  }

  int nret = evt_timer_.Start(5, 5);   //
  if (nret != RET_SUCCESS) {
    LOG(L_ERROR) << "evt timer open failed.";
    return -1;
  }
  return 0;
}

int CFlvOverHttp::AsyncWrite(const void *p_data, unsigned int n_data) {
  send_data_.WriteBytes((const uint8_t*)p_data, n_data);

  // 打开事件
  if (send_data_.UsedSize() > 0) {
    int32 n_ret = evt_send_.Start(sock_,
                                  EVT_WRITE | EVT_PERSIST);
    if (n_ret == 0) {
      evt_send_.ActiceEvent();
    }
  }
  return n_data;
}

int CFlvOverHttp::EvtSend(SOCKET fd, short events, const void *p_usr_arg) {
  int n_ret = -1;
  if (p_usr_arg) {
    n_ret = ((CFlvOverHttp*)p_usr_arg)->OnSend();
  }
  return n_ret;
}

int CFlvOverHttp::OnSend() {
  int rw_flag = 0;
#ifndef WIN32
  rw_flag = MSG_NOSIGNAL | MSG_DONTWAIT;
#endif
  int nsend = ::send(sock_,
                     (const char*)send_data_.GetReadPtr(),
                     send_data_.UsedSize(), rw_flag);
  if (nsend > 0) {
    send_data_.MoveReadPtr(nsend);
  }
  if (nsend < 0 && error_no() == XEAGAIN) {
    return 0;
  }

  int nret = 0;
  if (send_data_.UsedSize() <= 0) {
    send_data_.Recycle();             // 重置读写位置;移动为0
    evt_send_.Stop();
  }
  return nsend;
}

int CFlvOverHttp::EvtTimer(SOCKET fd, short events, const void *p_usr_arg) {
  int32 nret = -1;
  if (p_usr_arg) {
    nret = ((CFlvOverHttp*)p_usr_arg)->OnTimer();
    if (nret >= 0) {
      return 0;
    }

    delete ((CFlvOverHttp*)p_usr_arg);
    p_usr_arg = NULL;
    return -1;
  }
  LOG(L_ERROR) << "param is null.";
  return nret;
}

int32 CFlvOverHttp::OnTimer() {
  if (exit_flag_) {
    return -1;
  }

  if (flv_data_) {
    unsigned int n_sec = w_sec_, n_usec = w_usec_;
    int ndata = shm_vdo_.Read(flv_data_+flv_shm_.VdoHeadSize(),
                              flv_data_size_,
                              &n_sec, &n_usec);
    if (ndata > 0) {
      pts_ += (n_sec - w_sec_)*1000 + (n_usec - w_usec_)/1000;
      w_sec_ = n_sec;
      w_usec_ = n_usec;

      int   nal_bng = 0, frm_type = 0;
      char *p_nal = nal_parse(flv_data_+flv_shm_.VdoHeadSize(), ndata, &frm_type, &nal_bng);

      int n_flv = 0;
      if (frm_type == 5) {
        AsyncWrite(avcc_data_, avcc_data_size_);
        char *p_dst = flv_shm_.PacketVideo(flv_data_+nal_bng,
                                           p_nal + nal_bng, ndata - nal_bng,
                                           true, pts_);
        n_flv = p_dst - (flv_data_ + nal_bng);
      } else if (frm_type == 1) {
        char *p_dst = flv_shm_.PacketVideo(flv_data_+nal_bng,
                                           p_nal + nal_bng, ndata - nal_bng,
                                           false, pts_);
        n_flv = p_dst - (flv_data_ + nal_bng);
      }
      AsyncWrite(flv_data_+nal_bng, n_flv);
      if (file) {
        fwrite(flv_data_+nal_bng, 1, n_flv, file);
      }
      // printf("----------------- flv %d %d.\n", frm_type, n_flv);
    }
  }
  return 0;
}

char *CFlvOverHttp::nal_parse(const char *ph264, int nh264, int *frm_type, int *nal_bng) {
  int   n_0_cnt = 0;    // 0x00 个数
  char *p_nal = const_cast<char*>(ph264);
  while (true) {
    if ((p_nal + 1) > (ph264 + nh264)) {
      return NULL;
    }

    if (*p_nal == 0x01 && n_0_cnt >= 2) {
      p_nal -= n_0_cnt;
      *frm_type = p_nal[n_0_cnt + 1] & 0x1f;
      *nal_bng = n_0_cnt + 1;
      return p_nal;
    }

    if (*p_nal == 0x00) {
      n_0_cnt ++;
    } else {
      n_0_cnt = 0;
    }
    p_nal++;
  }
  return NULL;
}

