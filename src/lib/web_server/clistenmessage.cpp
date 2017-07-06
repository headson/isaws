/************************************************************************/
/* Author      : SoberPeng 2017-06-24
/* Description :
/************************************************************************/
#include "clistenmessage.h"
#include "vzbase/helper/stdafx.h"

#include "json/json.h"

#include "web_server/process/uri_handle.h"

CListenMessage::CListenMessage()
  : p_dp_cli_(NULL)
  , p_web_conn_(NULL) {
  memset(&c_web_srv_, 0, sizeof(c_web_srv_));
}

CListenMessage::~CListenMessage() {
  if (p_dp_cli_) {
    DpClient_ReleasePollHandle(p_dp_cli_);
    p_dp_cli_ = NULL;
  }
  DpClient_Stop();
}

/************************************************************************/
/* Description : 注册处理函数
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

#if 1
  DpClient_Init((char*)s_dp_ip, n_dp_port);
  n_ret = DpClient_Start(0);
  if (n_ret == VZNETDP_FAILURE) {
    LOG(L_ERROR) << "dp client start failed.";
    return false;
  }

  //if (p_dp_cli_ == NULL) {
  //  p_dp_cli_ = DpClient_CreatePollHandle();
  //  if (p_dp_cli_ == NULL) {
  //    LOG(L_ERROR) << "dp client create poll handle failed.";
  //    return false;
  //  }
  //}
#endif

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
  return (n_ret == VZNETDP_SUCCEED);
}

int32 CListenMessage::RunLoop() {
  mg_mgr_poll(&c_web_srv_, 10);

  int32 n_ret = DpClient_PollDpMessage(p_dp_cli_,
                                       10);
  if (n_ret == VZNETDP_SUCCEED) {
    return VZNETDP_SUCCEED;
  }

  if (p_dp_cli_ != NULL) {
    DpClient_ReleasePollHandle(p_dp_cli_);
    p_dp_cli_ = NULL;
  }

  // 重连
  p_dp_cli_ = DpClient_CreatePollHandle();

  const int MAX_TYPES_SIZE = 36;
  const char* MSG_TYPES[] = {
    "TEST_MSG_TYPE_01",
    "TEST_MSG_TYPE_02",
    "TEST_MSG_TYPE_03",
    "TEST_MSG_TYPE_04",
    "TEST_MSG_TYPE_05",
    "TEST_MSG_TYPE_06",
    "TEST_MSG_TYPE_07",
    "TEST_MSG_TYPE_08",
    "TEST_MSG_TYPE_09",
    "TEST_MSG_TYPE_10",
    "TEST_MSG_TYPE_11",
    "TEST_MSG_TYPE_12",
    "TEST_MSG_TYPE_13",
    "TEST_MSG_TYPE_14",
    "TEST_MSG_TYPE_15",
    "TEST_MSG_TYPE_16",
    "TEST_MSG_TYPE_17",
    "TEST_MSG_TYPE_18",
    "TEST_MSG_TYPE_19",
    "TEST_MSG_TYPE_20",
    "TEST_MSG_TYPE_21",
    "TEST_MSG_TYPE_22",
    "TEST_MSG_TYPE_23",
    "TEST_MSG_TYPE_24",
    "TEST_MSG_TYPE_25",
    "TEST_MSG_TYPE_26",
    "TEST_MSG_TYPE_27",
    "TEST_MSG_TYPE_28",
    "TEST_MSG_TYPE_29",
    "TEST_MSG_TYPE_30",
    "TEST_MSG_TYPE_31",
    "TEST_MSG_TYPE_32",
    "TEST_MSG_TYPE_33",
    "TEST_MSG_TYPE_34",
    "TEST_MSG_TYPE_35",
    "TEST_MSG_TYPE_36",
  };
  n_ret = DpClient_HdlAddListenMessage(p_dp_cli_, MSG_TYPES, MAX_TYPES_SIZE);
  if (n_ret == VZNETDP_FAILURE) {
    LOG(L_ERROR) << "add listen message failed.";
    DpClient_ReleasePollHandle(p_dp_cli_);
    p_dp_cli_ = NULL;

    return VZNETDP_FAILURE;
  }
  return VZNETDP_SUCCEED;
}

void CListenMessage::broadcast(const void* p_data, uint32 n_data) {
  struct mg_connection *c = mg_next(&c_web_srv_, NULL);
  for ( ; c != NULL; c = mg_next(&c_web_srv_, c)) {
    mg_send_websocket_frame(c, WEBSOCKET_OP_TEXT, p_data, n_data);
  }
}

void CListenMessage::msg_handler(const DpMessage *dmp, void* p_usr_arg) {
  if (p_usr_arg) {
    ((CListenMessage*)p_usr_arg)->OnMessage(dmp);
  }
  DpClient_SendDpReply(dmp->method,
    dmp->channel_id,
    dmp->id,
    "reply",
    5);
  //LOG(L_ERROR) << "param is error.";
}

void CListenMessage::OnMessage(const DpMessage *dmp) {

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

