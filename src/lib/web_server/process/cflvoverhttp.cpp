/************************************************************************/
/* Author      : SoberPeng 2017-07-24
/* Description :
/************************************************************************/
#include "cflvoverhttp.h"

#include "vzconn/base/vsocket.h"
#include "vzconn/base/clibevent.h"
#include "vzconn/buffer/cblockbuffer.h"

#include "vzbase/helper/stdafx.h"
#include "vzbase/core/vmessage.h"

#include "systemv/shm/vzshm_c.h"
#include "systemv/flvmux/cflvmux.h"
#include "web_server/clistenmessage.h"

CFlvOverHttp::CFlvOverHttp()
  : evt_loop_(NULL)
  , sock_(NULL)
  , evt_send_()
  , send_data_()
  , evt_timer_()
  , shm_vdo_()
  , flv_shm_()
  , file(NULL) {

}

CFlvOverHttp::~CFlvOverHttp() {
  evt_timer_.Stop();
  evt_send_.Stop();
  if (evt_loop_) {
    evt_loop_ = NULL;
  }

  printf("%s[%d].\n", __FUNCTION__, __LINE__);
}

bool CFlvOverHttp::Open(SOCKET sock, vzconn::EVT_LOOP *evt_loop, const char* shm_key, unsigned int shm_size) {
  sock_.SetSocket(sock);
  set_socket_nonblocking(sock);

  evt_loop_ = evt_loop;

  evt_send_.Init(evt_loop_, EvtSend, this);
  evt_timer_.Init(evt_loop_, EvtTimer, this);

  bool ret = shm_vdo_.Open(shm_key, shm_size);
  if (false == ret) {
    LOG(L_ERROR) << "shm video failed.";
    return false;
  }

  file = fopen("./test2.flv", "wb+");
  return true;
}

void CFlvOverHttp::Close() {
  sock_.SetSocket(INVALID_SOCKET);
}

int CFlvOverHttp::AsyncHeader(const void *phead, unsigned int nhead,
                              unsigned int nwidth, unsigned int nheight) {
  AsyncWrite(phead, nhead);

  int ndata = 0;
  char sdata[2048] = {0};
  char *p_dst = flv_shm_.HeaderAndMetaDataTag(sdata,
                nwidth, nheight,
                16000, 8000, 16, 1);
  ndata = p_dst - sdata;
  AsyncWrite(sdata, ndata);
  if (file) {
    fwrite(sdata, 1, ndata, file);
  }

  int n_sps = 0, n_pps = 0;
  ndata = shm_vdo_.ReadHead(sdata, 2048, &n_sps, &n_pps);
  LOG(L_ERROR)<<"sps length "<<n_sps
              <<" pps length "<<n_pps;
  ndata = flv_shm_.SetSps(sdata, n_sps);
  ndata += flv_shm_.SetPps(sdata + n_sps, n_pps);

  p_dst = flv_shm_.Packet(avcc_data_, NULL, 0,
                          flv_shm_.sps_pps_, flv_shm_.sps_size_ + flv_shm_.pps_size_,
                          0x09, 0);
  avcc_data_size_ = p_dst - avcc_data_;
  AsyncWrite(avcc_data_, avcc_data_size_);
  if (file) {
    fwrite(avcc_data_, 1, avcc_data_size_, file);
  }

  int nret = evt_timer_.Start(5, 5);   //
  if (nret != RET_SUCCESS) {
    LOG(L_ERROR) << "evt timer open failed.";
    return false;
  }
  return 0;
}

int CFlvOverHttp::AsyncWrite(const void *p_data, unsigned int n_data) {
  sock_.Send(p_data, n_data);
  return n_data;

  send_data_.WriteBytes((const uint8_t*)p_data, n_data);

  // 打开事件
  if (send_data_.UsedSize() > 0) {
    int32 n_ret = evt_send_.Start(sock_.GetSocket(),
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
  int nsend = sock_.Send(send_data_.GetReadPtr(),
                         send_data_.UsedSize());
  if (nsend > 0) {
    send_data_.MoveReadPtr(nsend);
  }

  int nret = 0;
  if (send_data_.UsedSize() <= 0) {
    send_data_.Recycle();             // 重置读写位置;移动为0
    evt_send_.Stop();
  }
  return 0;
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
  if (sock_.isClose()) {
    return -1;
  }
  static char *p_data = new char[640 * 480];
  static char *p_flv = new char[640 * 480 + 1024];

  static unsigned int npts = 0;
  static unsigned int n_sec = 0, n_usec = 0;

  if (p_data) {
    int ndata = shm_vdo_.Read(p_data, 640 * 480, &n_sec, &n_usec);
    if (ndata > 0) {
      npts += 40;

      int nal_bng = 0;
      int frm_type = 0;

      char *p_nal = nal_parse(p_data, ndata, &frm_type, &nal_bng);

      int n_flv = 0;
      if (frm_type == 5) {
        AsyncWrite(avcc_data_, avcc_data_size_);
        char *p_dst = flv_shm_.PacketVideo(p_flv, p_nal + nal_bng, ndata - nal_bng, true, npts);
        n_flv = p_dst - p_flv;
      } else if (frm_type == 1) {
        char *p_dst = flv_shm_.PacketVideo(p_flv, p_nal + nal_bng, ndata - nal_bng, false, npts);
        n_flv = p_dst - p_flv;
      }
      AsyncWrite(p_flv, n_flv);
      if (file) {
        fwrite(p_flv, 1, n_flv, file);
      }
      printf("----------------- flv %d %d.\n", frm_type, n_flv);
    }
  }
  return 0;
}

char * CFlvOverHttp::nal_parse(const char *ph264, int nh264, int *frm_type, int *nal_bng) {
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

