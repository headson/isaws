/************************************************************************/
/* Author      : SoberPeng 2017-07-24
/* Description :
/************************************************************************/
#include "cflvoverhttp.h"

#include "vzconn/base/vsocket.h"
#include "vzconn/base/clibevent.h"
#include "vzconn/buffer/cblockbuffer.h"

#include "vzbase/helper/stdafx.h"
#include "vzbase/base/timeutils.h"
#include "vzbase/base/vmessage.h"

#include "cflvmux.h"
#include "web_server/clistenmessage.h"

CFlvOverHttp::CFlvOverHttp()
  : thiz_ptr_()
  , exit_flag_(0)
  , evt_loop_(NULL)
  , sock_(NULL)
  , evt_send_()
  , send_data_()
  , evt_timer_()
  , shm_vdo_()
  , flv_shm_()
  , flv_data_(NULL)
  , flv_data_size_(0) {
  pts_ = vzbase::CurrentSystemTicket();
}

CFlvOverHttp::~CFlvOverHttp() {
  printf("%s[%d].\n", __FUNCTION__, __LINE__);
  evt_timer_.Stop();
  evt_send_.Stop();
  if (evt_loop_) {
    evt_loop_ = NULL;
  }
  if (flv_data_) {
    delete[] flv_data_;
    flv_data_ = NULL;
  }
}

bool CFlvOverHttp::Open(SOCKET sock, vzconn::EVT_LOOP *evt_loop, int dev_num) {
  dev_num_ = dev_num;

  sock_ = sock;
  set_socket_nonblocking(sock);

  TAG_SHM_ARG &shm_vdo = g_shm_avdo[dev_num_];
  LOG_INFO("video %s, w %d, h %d, size %d.\n",
           shm_vdo.dev_name,
           shm_vdo.width, shm_vdo.height,
           shm_vdo.shm_size);

  bool ret = shm_vdo_.Open(shm_vdo.dev_name, shm_vdo.shm_size);
  if (false == ret) {
    LOG(L_ERROR) << "shm video failed.";
    return false;
  }
  shm_vdo_ptr_ = (TAG_SHM_VDO *)shm_vdo_.GetData();

  evt_loop_ = evt_loop;
  thiz_ptr_.reset(this);
  evt_send_.Init(evt_loop_, EvtSend, &thiz_ptr_);
  evt_timer_.Init(evt_loop_, EvtTimer, &thiz_ptr_);
  // file = fopen("./test2.flv", "wb+");
  return true;
}

void CFlvOverHttp::Close() {
  exit_flag_ = 1;
  // sock_.SetSocket(INVALID_SOCKET);
}

int CFlvOverHttp::AsyncHeader(const void *phead, unsigned int nhead) {
  AsyncWrite(phead, nhead);

  TAG_SHM_ARG &shm_vdo = g_shm_avdo[dev_num_];
  if (NULL == flv_data_) {
    flv_data_size_ = shm_vdo.shm_size;
    flv_data_ = new char[flv_data_size_];
  }
  if (NULL == flv_data_) {
    LOG(L_ERROR) << "create flv buffer failed.";
    return -1;
  }

  int ndata = 0;
  char sdata[1024] = {0};
  char *p_dst = NULL;
  p_dst = flv_shm_.HeaderAndMetaDataTag(sdata,
                                        shm_vdo.width, shm_vdo.height,
                                        16000, 8000, 16, 1);
  ndata = p_dst - sdata;
  AsyncWrite(sdata, ndata);

  int n_sps = 14, n_pps = 9;
  memcpy(sdata, shm_vdo_ptr_->shead, shm_vdo_ptr_->nhead);

  ndata = flv_shm_.MakeAVCc(sdata, n_sps, n_pps);
  p_dst = flv_shm_.Packet(avcc_data_, NULL, 0,
                          flv_shm_.avcc_, flv_shm_.avcc_size_,
                          0x09, 0);
  avcc_data_size_ = p_dst - avcc_data_;
  AsyncWrite(avcc_data_, avcc_data_size_);

  int nret = evt_timer_.Start(5, 5);   //
  if (nret != 0) {
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

int CFlvOverHttp::EvtSend(SOCKET fd, short events, const void *usr_arg) {
  int res = -1;
  if (usr_arg) {
    CFlvOverHttp::Ptr ptr =
      *((CFlvOverHttp::Ptr*)usr_arg);
    res = ptr->OnSend();
    if (res < 0) {
      ((CFlvOverHttp::Ptr*)usr_arg)->reset();
    }
  }
  return res;
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

int CFlvOverHttp::EvtTimer(SOCKET fd, short events, const void *usr_arg) {
  int32 res = -1;
  if (usr_arg) {
    CFlvOverHttp::Ptr ptr =
      *((CFlvOverHttp::Ptr*)usr_arg);
    res = ptr->OnTimer();
    if (res < 0) {
      ((CFlvOverHttp::Ptr*)usr_arg)->reset();
    }
  }
  return res;
}

int32 CFlvOverHttp::OnTimer() {
  if (exit_flag_) {
    return -1;
  }

  if (flv_data_) {
    if (shm_vdo_ptr_->wsec == w_sec_ &&
        shm_vdo_ptr_->wusec == w_usec_) {
      if (vzconn::VSocket::IsSocketClosed(sock_)) {
        return -1;
      }
      return 0;
    }
    w_sec_ = shm_vdo_ptr_->wsec;
    w_usec_ = shm_vdo_ptr_->wusec;
    // LOG_INFO("sec %d, usec %d.", w_sec_, w_usec_);

    int nflv_data = shm_vdo_ptr_->ndata;
    memcpy(flv_data_+flv_shm_.VdoHeadSize(),
           shm_vdo_ptr_->pdata, shm_vdo_ptr_->ndata);

    uint32 pts = vzbase::CurrentSystemTicket() - pts_;
    // LOG(L_INFO) << "pts "<< pts;

    int   nal_bng = 0, frm_type = 0;
    char *p_nal = nal_parse(flv_data_+flv_shm_.VdoHeadSize(), nflv_data, &frm_type, &nal_bng);

    int n_flv = 0;
    if (frm_type == 5) {
      AsyncWrite(avcc_data_, avcc_data_size_);
      char *p_dst = flv_shm_.PacketVideo(flv_data_+nal_bng,
                                         p_nal + nal_bng, nflv_data - nal_bng,
                                         true, pts);
      n_flv = p_dst - (flv_data_ + nal_bng);
    } else if (frm_type == 1) {
      char *p_dst = flv_shm_.PacketVideo(flv_data_+nal_bng,
                                         p_nal + nal_bng, nflv_data - nal_bng,
                                         false, pts);
      n_flv = p_dst - (flv_data_ + nal_bng);
    }
    AsyncWrite(flv_data_+nal_bng, n_flv);
    // printf("----------------- flv %d %d.\n", frm_type, n_flv);
  }
  return 0;
}

char *CFlvOverHttp::nal_parse(const char *ph264, int nh264,
                              int *frm_type, int *nal_bng) {
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

