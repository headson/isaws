/************************************************************************/
/* Author      : SoberPeng 2017-06-24
/* Description :
/************************************************************************/
#include "clistenmessage.h"
#include "vzbase/helper/stdafx.h"

#include "json/json.h"

#include "web_server/process/uri_handle.h"

static const uint32 METHOD_SET_SIZE = 3;
static const char  *METHOD_SET[] = {
  "TEST_MSG_TYPE_01",
  "TEST_MSG_TYPE_02",
  "TEST_MSG_TYPE_03",
};

CListenMessage::CListenMessage()
  : p_dp_cli_(NULL)
  , p_web_conn_(NULL)
  , p_web_thread_(NULL) {
  memset(&c_web_srv_, 0, sizeof(c_web_srv_));
}

CListenMessage::~CListenMessage() {
  if (p_dp_cli_) {
    DpClient_ReleasePollHandle(p_dp_cli_);
    p_dp_cli_ = NULL;
  }
  DpClient_Stop();

  if (p_web_thread_) {
    delete p_web_thread_;
    p_web_thread_ = NULL;
  }
}

/************************************************************************/
/* Description : ×¢²á´¦Àíº¯Êý
/* Parameters  :
/* Return      :
/************************************************************************/
static void register_http_endpoint(struct mg_connection *nc) {
  mg_register_http_endpoint(nc, "/http_login", uri_hdl_login);
}

static struct mg_serve_http_opts s_web_opts_;

bool CListenMessage::Start(const uint8 *s_dp_ip,
                           uint16       n_dp_port,
                           const uint8 *s_http_port,
                           const uint8 *s_http_path) {
  int32 n_ret = VZNETDP_SUCCEED;

  // web server
  mg_mgr_init(&c_web_srv_, this);
  p_web_conn_ = mg_bind(&c_web_srv_,
    (char*)s_http_port,
    ev_handler);
  if (p_web_conn_ == NULL) {
    LOG(L_ERROR) << "mg bind failed.";
    return -1;
  }

  register_http_endpoint(p_web_conn_);
  // Set up HTTP server parameters
  mg_set_protocol_http_websocket(p_web_conn_);

  s_web_opts_.document_root = (char*)s_http_path;
  s_web_opts_.enable_directory_listing = "yes";

  p_web_thread_ = new vzbase::Thread();
  if (p_web_thread_ == NULL) {
    LOG(L_ERROR) << "create thread failed.";
    return false;
  }
  p_web_thread_->Start(this);

  // dp client poll
  DpClient_Init((char*)s_dp_ip, n_dp_port);
  n_ret = DpClient_Start(0);
  if (n_ret == VZNETDP_FAILURE) {
    LOG(L_ERROR) << "dp client start failed.";
    return false;
  }

  if (p_dp_cli_ == NULL) {
    p_dp_cli_ = DpClient_CreatePollHandle(msg_handler, this,
                                          state_handler, this,
                                         vzbase::Thread::Current()->socketserver()->GetEvtService());
    if (p_dp_cli_ == NULL) {
      LOG(L_ERROR) << "dp client create poll handle failed.";
      return false;
    }
    
    //DpClient_HdlAddListenMessage(p_dp_cli_, METHOD_SET, METHOD_SET_SIZE);
  }
  return (n_ret == VZNETDP_SUCCEED);
}

int32 CListenMessage::RunLoop() {
  vzbase::Thread::Current()->Run();
  int32 n_ret = DpClient_PollDpMessage(p_dp_cli_,
                                       10);
  if (n_ret == VZNETDP_SUCCEED) {
    return VZNETDP_SUCCEED;
  }
  return VZNETDP_SUCCEED;
}

void CListenMessage::broadcast(const void* p_data, uint32 n_data) {
  struct mg_connection *c = mg_next(&c_web_srv_, NULL);
  for ( ; c != NULL; c = mg_next(&c_web_srv_, c)) {
    mg_send_websocket_frame(c, WEBSOCKET_OP_TEXT, p_data, n_data);
  }
}

void CListenMessage::msg_handler(DPPollHandle p_hdl, const DpMessage *dmp, void* p_usr_arg) {
  if (p_usr_arg) {
    ((CListenMessage*)p_usr_arg)->OnMessage(p_hdl, dmp);
    return;
  }
  LOG(L_ERROR) << "param is error.";
}

void CListenMessage::OnMessage(DPPollHandle p_hdl, const DpMessage *dmp) {

}

void CListenMessage::state_handler(DPPollHandle p_hdl, uint32 n_state, void* p_usr_arg) {
  if (p_usr_arg) {
    ((CListenMessage*)p_usr_arg)->OnState(p_hdl, n_state);
  }
}

void CListenMessage::OnState(DPPollHandle p_hdl, uint32 n_state) {
  if (n_state == DP_CLIENT_DISCONNECT) {
    int32 n_ret = DpClient_HdlReConnect(p_hdl);
    if (n_ret == VZNETDP_SUCCEED) {
      DpClient_HdlAddListenMessage(p_dp_cli_, METHOD_SET, METHOD_SET_SIZE);
    }
  }
}

void CListenMessage::ev_handler(struct mg_connection *nc,
                                int ev,
                                void *ev_data) {
  if (nc && nc->mgr && nc->mgr->user_data) {
    ((CListenMessage*)nc->mgr->user_data)->OnEvHdl(nc, ev, ev_data);
  }
}

void CListenMessage::OnEvHdl(struct mg_connection *nc, int ev, void *ev_data) {
  switch (ev) {
  case MG_EV_HTTP_REQUEST:
    mg_serve_http(nc, (struct http_message *) ev_data, s_web_opts_);
    break;
  }
}

void CListenMessage::Run(vzbase::Thread* thread) {
  while (p_dp_cli_) {
    mg_mgr_poll(&c_web_srv_, 1000);
  }
}

