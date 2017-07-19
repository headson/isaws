/************************************************************************/
/* Author      : SoberPeng 2017-06-24
/* Description :
/************************************************************************/
#include "uri_handle.h"

#include "vzconn/base/vsocket.h"
#include "vzconn/base/clibevent.h"
#include "vzconn/buffer/cblockbuffer.h"

#include "vzbase/helper/stdafx.h"
#include "vzbase/core/vmessage.h"

#include "systemv/shm/vzshm_c.h"
#include "systemv/flvmux/cflvmux.h"
#include "web_server/clistenmessage.h"

typedef struct _file_writer_data {
  FILE      *fp;
  size_t     bytes_written;
} file_writer_data;

#ifdef __cplusplus
namespace web {
extern "C" {
#endif

class CFlvOverHttp
{
 public:
  CFlvOverHttp()
    : evt_loop_(NULL)
    , sock_(NULL)
    , evt_send_()
    , send_data_()
    , evt_timer_()
    , shm_video_(NULL)
    , shm_audio_(NULL)
    , flv_shm_(NULL) {
  }
  virtual ~CFlvOverHttp() {
    // sock_.Close();
    if (shm_video_) {
      Shm_Release(shm_video_);
      shm_video_ = NULL;
    }
    evt_timer_.Stop();
    evt_send_.Stop();
    if (evt_loop_) {
      evt_loop_  = NULL;
    }
  }

  bool Open(SOCKET sock, vzconn::EVT_LOOP *evt_loop,
            const char* shm_key, unsigned int shm_size) {
    sock_.SetSocket(sock);
    set_socket_nonblocking(sock);

    evt_loop_ = evt_loop;

    int32 nret = -1;

    evt_send_.Init(evt_loop_, EvtSend, this);

    evt_timer_.Init(evt_loop_, EvtTimer, this);
    nret = evt_timer_.Start(20, 20);   //
    if (nret != RET_SUCCESS) {
      LOG(L_ERROR) << "evt timer open failed.";
      return false;
    }

    shm_video_ = Shm_Create(shm_key, shm_size);
    if (NULL == shm_video_) {
      LOG(L_ERROR) << "shm video failed.";
      return false;
    }
    return true;
  }

  int32 AsyncHeader(const void *phead, int32 nhead,
                    int32 nwidth, int32 nheight) {
    char sdata[1024] = { 0 };
    int32 nret = flv_shm_.InitHeadTag0((uint8*)sdata,
                                       1023,
                                       nwidth, nheight);
    AsyncWrite(phead, nhead, 0);
    AsyncWrite(sdata, nret, 0);
    return nhead + nret;
  }

  virtual int32 AsyncWrite(const void  *p_data,
                           uint32       n_data,
                           uint16       e_flag) {
    LOG(L_ERROR) << n_data;
    sock_.Send(p_data, n_data);
    return n_data;

    send_data_.WriteBytes((const uint8_t*)p_data, n_data);

    // ���¼�
    if (send_data_.UsedSize() > 0) {
      int32 n_ret = evt_send_.Start(sock_.GetSocket(),
                                    EVT_WRITE | EVT_PERSIST);
      if (n_ret == 0) {
        evt_send_.ActiceEvent();
      }
    }
    return n_data;
  }

 protected:
  static int32 EvtSend(SOCKET      fd,
                       short       events,
                       const void *p_usr_arg) {
    int32 n_ret = -1;
    if (p_usr_arg) {
      n_ret = ((CFlvOverHttp*)p_usr_arg)->OnSend();
    }
    return n_ret;
  }

  int32 OnSend() {
    int32 nsend = sock_.Send(send_data_.GetReadPtr(),
                             send_data_.UsedSize());
    if (nsend > 0) {
      send_data_.MoveReadPtr(nsend);
    }

    int32 nret = 0;
    if (send_data_.UsedSize() <= 0) {
      send_data_.Recycle();             // ���ö�дλ��;�ƶ�Ϊ0
      evt_send_.Stop();
    }
    return 0;
  }

  static int32 EvtTimer(SOCKET fd, short events, const void *p_usr_arg) {
    int32 nret = -1;
    if (p_usr_arg) {
      nret = ((CFlvOverHttp*)p_usr_arg)->OnTimer();
      return 0;
    }
    LOG(L_ERROR) << "param is null.";
    return nret;
  }

  int32 OnTimer() {
    static char *p_data = new char[640*480];
    static uint32 n_sec=0, n_usec=0;
    if (p_data) {
      int nret = Shm_Read(shm_video_, p_data, 640*480, &n_sec, &n_usec);
      if (nret > 0) {
        AsyncWrite(p_data, nret, 0);
      }
    }
    return 0;
  }

 private:
  vzconn::EVT_LOOP      *evt_loop_;

  vzconn::VSocket        sock_;

  vzconn::EVT_IO         evt_send_;
  vzconn::CBlockBuffer   send_data_;

  vzconn::EVT_TIMER      evt_timer_;
  void                  *shm_video_;
  void                  *shm_audio_;

  CFlvMux                flv_shm_;
};

//����̫С��û��Ҫдͷ����̬����ˣ�ȫ��д���ˣ�

#define RESPONSE_HEADER_CONCLOSE "Connection: close\r\n"
#define RESPONSE_HEADER_KEEPALIVE "Connection:keep-alive\r\n"

#define RESPONSE_HEADER_NOCACHE "Cache-Control: no-cache\r\n"\
  "Pragma: no-cache\r\n"

#define RESPONSE_HEADER_START "HTTP/1.1 200 OK\r\n"\
  "Server: DesktopLiveStreaming\r\n"\

#define RESPONSE_HEADER_Allow_Origin	"Access-Control-Allow-Origin: *\r\n"

static const char *response_flv = RESPONSE_HEADER_START
                                  RESPONSE_HEADER_NOCACHE
                                  RESPONSE_HEADER_CONCLOSE
                                  RESPONSE_HEADER_Allow_Origin
                                  "Content-Type: video/x-flv\r\n"
                                  "\r\n";
static const size_t response_flv_size = strlen(response_flv);

void uri_hdl_httpflv(struct mg_connection *nc, int ev, void *p) {
  struct http_message *hm = (struct http_message*)p;

  CFlvOverHttp *pflv = new CFlvOverHttp();
  if (pflv == NULL) {
    mg_printf(nc, "HTTP/1.0 403 Unauthorized\r\n\r\ncreate class failed.\r\n");
    nc->flags |= MG_F_SEND_AND_CLOSE;
    return;
  }
  vzconn::EVT_LOOP *evt_loop =
    CListenMessage::Instance()->MainThread()->socketserver()->GetEvtService();

  //mg_get_http_var(&hm->query_string, "chn");

  bool bret = pflv->Open(nc->sock, evt_loop, SHM_VIDEO_0, SHM_VIDEO_0_SIZE);
  if (bret == false) {
    delete pflv;
    pflv = NULL;

    mg_printf(nc, "HTTP/1.0 403 Unauthorized\r\n\r\nflv open failed.\r\n");
    nc->flags |= MG_F_SEND_AND_CLOSE;
    return;
  }
  nc->user_data = pflv;

  pflv->AsyncHeader(response_flv, response_flv_size,
                    640, 480);
}

#ifdef __cplusplus
}
}  // namespace web
#endif
