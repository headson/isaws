/************************************************************************/
/* Author      : SoberPeng 2017-06-24
/* Description :
/************************************************************************/
#include "uri_handle.h"

#include "vzbase/helper/stdafx.h"

#include "web_server/clistenmessage.h"

#include "web_server/process/cflvoverhttp.h"

#ifdef __cplusplus
namespace web {
extern "C" {
#endif

//程序太小，没必要写头部动态组合了，全部写死了，

#define RESPONSE_HEADER_CONCLOSE "Connection: close\r\n"
#define RESPONSE_HEADER_KEEPALIVE "Connection:keep-alive\r\n"

#define RESPONSE_HEADER_NOCACHE "Cache-Control: no-cache\r\n"\
  "Pragma: no-cache\r\n"

#define RESPONSE_HEADER_START "HTTP/1.1 200 OK\r\n"\
  "Server: web_server_app\r\n"\

#define RESPONSE_HEADER_Allow_Origin	"Access-Control-Allow-Origin: *\r\n"

static const char *response_flv = RESPONSE_HEADER_START
                                  RESPONSE_HEADER_NOCACHE
                                  RESPONSE_HEADER_CONCLOSE
                                  RESPONSE_HEADER_Allow_Origin
                                  "Content-Type: video/x-flv\r\n"
                                  "\r\n";
static const size_t response_flv_size = strlen(response_flv);

void uri_hdl_httpflv(struct mg_connection *nc, int ev, void *p) {
  printf("event %d.\n", ev);
  struct http_message *hm = (struct http_message*)p;

  CFlvOverHttp *pflv = new CFlvOverHttp();
  if (pflv == NULL) {
    mg_printf(nc, "HTTP/1.0 403 Unauthorized\r\n\r\ncreate class failed.\r\n");
    nc->flags |= MG_F_SEND_AND_CLOSE;
    return;
  }
  vzconn::EVT_LOOP *evt_loop =
    CListenMessage::Instance()->MainThread()->socketserver()->GetEvtService();

  char chn[128] = {0};
  mg_get_http_var(&hm->query_string, "chn", chn, 123);

  bool bres = false;
  if (0 == strncmp(chn, "video0", 7)) {
    bres = pflv->Open(nc->sock, evt_loop, SHM_VIDEO_0, SHM_VIDEO_0_SIZE);
  } else if (0 == strncmp(chn, "video1", 7)) {
    bres = pflv->Open(nc->sock, evt_loop, SHM_VIDEO_1, SHM_VIDEO_1_SIZE);
  } else if (0 == strncmp(chn, "video2", 7)) {
    bres = pflv->Open(nc->sock, evt_loop, SHM_VIDEO_2, SHM_VIDEO_2_SIZE);
  } else {
    bres = false;
  }
  if (bres == false) {
    delete pflv;
    pflv = NULL;

    mg_printf(nc, "HTTP/1.0 403 Unauthorized\r\n\r\nflv open failed.\r\n");
    nc->flags |= MG_F_SEND_AND_CLOSE;
    return;
  }
  nc->user_data = pflv;

  pflv->AsyncHeader(response_flv, response_flv_size);
}

void url_hdl_httpflv_release(struct mg_connection *nc) {
  char *p = strstr(nc->recv_mbuf.buf, "httpflv");
  if (p != NULL) {
    ((CFlvOverHttp*)nc->user_data)->Close();
  }
}

#ifdef __cplusplus
}
}  // namespace web
#endif
