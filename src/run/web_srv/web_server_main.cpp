
#include "basictypes.h"

#include "web/base/mongoose.h"
#include "web/process/uri_handle.h"

#include "vzlogging/logging/vzwatchdog.h"
#include "vzlogging/logging/vzloggingcpp.h"

struct mg_mgr                    mgr;
static void                       *s_watchdog=NULL;
static const char                 *s_http_port="8000";
static struct mg_serve_http_opts   s_http_server_opts;

// 广播消息(WebSocket)
static void broadcast(struct mg_mgr *mgr,
                      const void* p_data,
                      uint32 n_data) {
  struct mg_connection *c;
  for (c = mg_next(mgr, NULL); c != NULL; c = mg_next(mgr, c)) {
    mg_send_websocket_frame(c, WEBSOCKET_OP_TEXT, p_data, n_data);
  }
}

/************************************************************************/
/* Description : 默认request处理
/* Parameters  :
/* Return      :
/************************************************************************/
static void ev_handler(struct mg_connection *nc, int ev, void *ev_data) {
  switch (ev) {
  case MG_EV_HTTP_REQUEST:
    mg_serve_http(nc, (struct http_message *) ev_data, s_http_server_opts);
    break;
  }
}

/************************************************************************/
/* Description : 注册处理函数
/* Parameters  :
/* Return      :
/************************************************************************/
void register_http_endpoint(struct mg_connection *nc) {
  mg_register_http_endpoint(nc, "/http_login", uri_hdl_login);
}

int main(int argc, char *argv[]) {
  struct mg_connection *nc = NULL;

  InitVzLogging(argc, argv);
#ifdef _WIN32
  ShowVzLoggingAlways();
#endif
  s_watchdog = RegisterWatchDogKey("MAIN", 4, 20);

  mg_mgr_init(&mgr, NULL);
  nc = mg_bind(&mgr, s_http_port, ev_handler);
  if (nc == NULL) {
    LOG(L_ERROR) << "mg bind failed.";
    return -1;
  }
  register_http_endpoint(nc);
  // Set up HTTP server parameters
  mg_set_protocol_http_websocket(nc);

  s_http_server_opts.document_root = "c:/tools/web";
  s_http_server_opts.enable_directory_listing = "yes";

  LOG(L_WARNING)<< "Starting web server on port "<< s_http_port;
  for (;;) {
    mg_mgr_poll(&mgr, 10*1000);

    if (s_watchdog) {
      FeedDog(s_watchdog);
    }
  }
  mg_mgr_free(&mgr);

  return 0;
}
