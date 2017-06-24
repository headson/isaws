/************************************************************************/
/* Author      : SoberPeng 2017-06-24
/* Description :
/************************************************************************/
#include "clistenmessage.h"

#include "stdafx.h"
#include "json/json.h"

#include "dispatcher/base/pkghead.h"

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

bool CListenMessage::Start(const uint8* s_ip, uint16 n_port) {
  DpClient_Init("127.0.0.1", 3730);

  int32 n_ret = DpClient_Start(0);
  if (n_ret == VZNETDP_FAILURE) {
    LOG(L_ERROR) << "dp client start failed.";
    return false;
  }

  if (p_dp_cli_ == NULL) {
    p_dp_cli_ = DpClient_CreatePollHandle();
    if (p_dp_cli_ == NULL) {
      LOG(L_ERROR) << "dp client create poll handle failed.";
      return false;
    }
  }
  return (n_ret == VZNETDP_SUCCEED);
}

int32 CListenMessage::RunLoop() {
  int32 n_ret = DpClient_PollDpMessage(p_dp_cli_,
                                       MsgFunc,
                                       this,
                                       DEF_TIMEOUT_MSEC);
  if (n_ret == VZNETDP_SUCCEED) {
    return VZNETDP_SUCCEED;
  }

  if (p_dp_cli_ != NULL) {
    DpClient_ReleasePollHandle(p_dp_cli_);
    p_dp_cli_ = NULL;
  }

  // ÖØÁ¬
  p_dp_cli_ = DpClient_CreatePollHandle();

  const int   MAX_TYPES_SIZE = 1;
  const char* MSG_TYPES[] = {
    "dp_system_config"
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

void CListenMessage::MsgFunc(const DpMessage *dmp, void* p_usr_arg) {
  if (p_usr_arg) {
    ((CListenMessage*)p_usr_arg)->OnMessage(dmp);
  }
  LOG(L_ERROR) << "param is error.";
}

void CListenMessage::OnMessage(const DpMessage *dmp) {

}

