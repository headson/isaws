/************************************************************************/
/* Author      : SoberPeng 2017-06-24
/* Description :
/************************************************************************/
#include "clistenmessage.h"

#include "vzbase/helper/stdafx.h"

#include "json/json.h"

CListenMessage::CListenMessage()
  : p_dp_cli_(NULL) {
}

CListenMessage::~CListenMessage() {
  if (p_dp_cli_) {
    DpClient_ReleasePollHandle(p_dp_cli_);
    p_dp_cli_ = NULL;
  }
  DpClient_Stop();
}

bool CListenMessage::Start(const uint8 *s_dp_ip,
                           uint16       n_dp_port,
                           const uint8 *s_http_port,
                           const uint8 *s_http_path) {
  int32 n_ret = VZNETDP_SUCCEED;

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
  return (n_ret == VZNETDP_SUCCEED);
}

int32 CListenMessage::RunLoop() {
  int32 n_ret = DpClient_PollDpMessage(p_dp_cli_,
                                       MsgFunc,
                                       this,
                                       10);
  if (n_ret == VZNETDP_SUCCEED) {
    return VZNETDP_SUCCEED;
  }

  if (p_dp_cli_ != NULL) {
    DpClient_ReleasePollHandle(p_dp_cli_);
    p_dp_cli_ = NULL;
  }

  // ÖØÁ¬
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

