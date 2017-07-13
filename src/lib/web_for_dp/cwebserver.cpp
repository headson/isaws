/************************************************************************/
/* Author      : SoberPeng 2017-07-07
/* Description :
/************************************************************************/
#include "cwebserver.h"

#include "vzbase/helper/stdafx.h"

#include "web_for_dp/process/uri_handle.h"

namespace web {

static struct mg_serve_http_opts s_web_def_opts_;

/************************************************************************/
/* Description : 注册处理函数
/* Parameters  :
/* Return      :
/************************************************************************/
static void register_http_endpoint(struct mg_connection *nc) {
  /* dispatch 调试 handle */
  mg_register_http_endpoint(nc, "/set_dev_addr",    uri_hdl_set_dev_addr);
  mg_register_http_endpoint(nc, "/add_listen_msg",  uri_hdl_add_listen_msg);
  mg_register_http_endpoint(nc, "/del_listen_msg",  uri_hdl_del_listen_msg);
  mg_register_http_endpoint(nc, "/send_request",    uri_hdl_send_request);
  mg_register_http_endpoint(nc, "/send_reply",      uri_hdl_send_reply);
  mg_register_http_endpoint(nc, "/send_message",    uri_hdl_send_message);
  mg_register_http_endpoint(nc, "/get_recv_msg",    uri_hdl_get_recv_msg);
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

bool CWebServer::Start(const char *s_http_path, const char *s_http_port) {
  mg_mgr_init(&c_web_srv_, this);
  p_web_conn_ = mg_bind(&c_web_srv_, (char*)s_http_port, web_ev_handler);
  if (p_web_conn_ == NULL) {
    LOG(L_ERROR) << "mg bind failed.";
    return false;
  }

  register_http_endpoint(p_web_conn_);
  // Set up HTTP server parameters
  mg_set_protocol_http_websocket(p_web_conn_);

  s_web_def_opts_.document_root = (char*)s_http_path;
  s_web_def_opts_.enable_directory_listing = "yes";

  // 在新线程中跑
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
    mg_mgr_free(&c_web_srv_);
    b_runing_ = false;
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
  if (nc && nc->mgr && nc->mgr->user_data) {
    ((CWebServer*)nc->mgr->user_data)->OnWebEvHdl(nc, ev, ev_data);
  }
}

void CWebServer::OnWebEvHdl(struct mg_connection *nc, int ev, void *ev_data) {
  switch (ev) {
  case MG_EV_HTTP_REQUEST:
    mg_serve_http(nc, (struct http_message *) ev_data, s_web_def_opts_);
    break;
  }
}

}  // namespace web

