/************************************************************************/
/* Author      : SoberPeng 2017-07-07
/* Description :
/************************************************************************/
#include "cwebserver.h"

#include <string.h>

#include "vzbase/helper/stdafx.h"

#include "web_server/process/uri_handle.h"
#include "web_server/process/cflvoverhttp.h"

namespace web {

static struct mg_serve_http_opts s_web_def_opts_;
static struct mg_serve_http_opts s_web_log_opts_;

/************************************************************************/
/* Description : ×¢²á´¦Àíº¯Êý
/* Parameters  :
/* Return      :
/************************************************************************/
static void register_http_endpoint(struct mg_connection *nc) {
  mg_register_http_endpoint(nc, "/login_req",     uri_hdl_login);
  mg_register_http_endpoint(nc, "/upload",        uri_hdl_upload);
  mg_register_http_endpoint(nc, "/httpflv",       uri_hdl_httpflv);
  mg_register_http_endpoint(nc, "/dispatch",      uri_hdl_dispatch);
}
CWebServer::CWebServer()
  : vzbase::Runnable()
  , c_web_srv_()
  , p_web_conn_(NULL)
  , b_runing_(false)
  , p_web_thread_(NULL) {
}

CWebServer::~CWebServer() {
  Stop();
}

bool CWebServer::Start(const char *s_web_path, const char *s_http_port) {
  mg_mgr_init(&c_web_srv_, this);
  p_web_conn_ = mg_bind(&c_web_srv_, (char*)s_http_port, web_ev_handler);
  if (p_web_conn_ == NULL) {
    LOG(L_ERROR) << "mg bind failed.";
    return false;
  }

  register_http_endpoint(p_web_conn_);
  // Set up HTTP server parameters
  mg_set_protocol_http_websocket(p_web_conn_);
  
  s_web_def_opts_.document_root = (char*)s_web_path;
  s_web_def_opts_.enable_directory_listing = "yes";
  s_web_def_opts_.index_files = "index.html";

  // mg_set_timer(p_web_conn_, mg_time() + SESSION_CHECK_INTERVAL);

  p_web_thread_ = new vzbase::Thread();
  if (p_web_thread_) {
    b_runing_ = true;
    return p_web_thread_->Start(this);
  }
  LOG(L_ERROR) << "create web thread failed.";
  return false;
}

void CWebServer::Stop() {
  if (b_runing_) {
    b_runing_ = false;
    p_web_thread_->Release();
    p_web_thread_ = NULL;
    
    usleep(1000*1000);
    mg_mgr_free(&c_web_srv_);
  }
}

void CWebServer::Run(vzbase::Thread* thread) {
  while (b_runing_) {
    mg_mgr_poll(&c_web_srv_, 1000);
  }
}

void CWebServer::Broadcast(const void* p_data, unsigned int n_data) {
  struct mg_connection *c = mg_next(&c_web_srv_, NULL);
  for ( ; c != NULL; c = mg_next(&c_web_srv_, c)) {
    mg_send_websocket_frame(c, WEBSOCKET_OP_TEXT, p_data, n_data);
  }
}

void CWebServer::web_ev_handler(struct mg_connection *nc, int ev, void *ev_data) {
  switch (ev) {
  case MG_EV_HTTP_REQUEST:
    mg_serve_http(nc, (struct http_message*)ev_data, s_web_def_opts_);
    break;

  /*case MG_EV_CLOSE:
    if (nc->user_data) {
      url_hdl_httpflv_release(nc);
      nc->user_data = NULL;
    }
    break;
  }*/
}

} //  namespace web

