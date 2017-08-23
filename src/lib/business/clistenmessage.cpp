/************************************************************************/
/* Author      : SoberPeng 2017-06-24
/* Description :
/************************************************************************/
#include "clistenmessage.h"
#include "vzbase/helper/stdafx.h"
#include "vzbase/base/vmessage.h"

#include "json/json.h"

namespace bs {

static const char *METHOD_SET[] = {
  MSG_CATCH_EVENT,
  MSG_GET_PCOUNTS,
  MSG_CLEAR_PCOUNT
};
static const unsigned int METHOD_SET_SIZE = sizeof(METHOD_SET) / sizeof(char*);

#define MSG_HDL_CHECK_DATABASE  0x123456
CListenMessage::CListenMessage()
  : dp_cli_(NULL)
  , main_thread_(NULL)
  , database_() {
}

CListenMessage::~CListenMessage() {
  Stop();
}

CListenMessage *CListenMessage::Instance() {
  VZBASE_DEFINE_STATIC_LOCAL(CListenMessage, listen_message, ());
  return &listen_message;
}

bool CListenMessage::Start(const char *db_path) {
  bool res = database_.InitDB(db_path);
  if (!res) {
    LOG(L_ERROR) << "database start failed.";
    return false;
  }

  //////////////////////////////////////////////////////////////////////////
  main_thread_ = vzbase::Thread::Current();
  if (dp_cli_ == NULL) {
    vzconn::EventService *p_evt_srv =
      main_thread_->socketserver()->GetEvtService();

    dp_cli_ = DpClient_CreatePollHandle(dpcli_poll_msg_cb, this,
                                        dpcli_poll_state_cb, this,
                                        p_evt_srv);
    if (dp_cli_ == NULL) {
      LOG(L_ERROR) << "dp client create poll handle failed.";

      DpClient_ReleasePollHandle(dp_cli_);
      dp_cli_ = NULL;
      return false;
    }

    DpClient_HdlAddListenMessage(dp_cli_, METHOD_SET, METHOD_SET_SIZE);
  }

  std::string last_time = "";
  database_.SelectLastPCount(&last_time);
  if (last_time.size() > 0) {
  }
  vzbase::Thread::Current()->PostDelayed(2*1000, this, MSG_HDL_CHECK_DATABASE);
  return true;
}

void CListenMessage::Stop() {
  if (main_thread_) {
    main_thread_->Release();
    main_thread_ = NULL;
  }

  if (dp_cli_) {
    DpClient_ReleasePollHandle(dp_cli_);
    dp_cli_ = NULL;
  }
  DpClient_Stop();
}

void CListenMessage::RunLoop() {
  while (true) {
    main_thread_->ProcessMessages(5*1000);
  }
}

vzbase::Thread *CListenMessage::MainThread() {
  return main_thread_;
}
void CListenMessage::dpcli_poll_msg_cb(DPPollHandle p_hdl, const DpMessage *dmp, void* p_usr_arg) {
  if (p_usr_arg) {
    ((CListenMessage*)p_usr_arg)->OnDpMessage(p_hdl, dmp);
    return;
  }
  LOG(L_ERROR) << "param is error.";
}

void CListenMessage::OnDpMessage(DPPollHandle p_hdl, const DpMessage *dmp) {
  Json::Value jreq;
  Json::Reader jread;
  if (!jread.parse(dmp->data,
                   dmp->data+dmp->data_size,
                   jreq)) {
    LOG(L_ERROR) << "Json parse failed.";
    return;
  }
  Json::Value jresp;
  jresp[MSG_CMD] = jreq[MSG_CMD].asString();
  jresp[MSG_ID] = jreq[MSG_ID].asInt();

  bool res = false;
  if (0 == strncmp(dmp->method, MSG_CATCH_EVENT, MAX_METHOD_SIZE)) {
    res = database_.ReplacePCount(jreq);
  } else if (0 == strncmp(dmp->method, MSG_GET_PCOUNTS, MAX_METHOD_SIZE)) {
    res = database_.SelectPCount(jresp, jreq);
  } else if (0 == strncmp(dmp->method, MSG_CLEAR_PCOUNT, MAX_METHOD_SIZE)) {
    res = database_.ClearPCount(jresp);
  }

  if (dmp->type != TYPE_REQUEST) {
    return;
  }

  if (true == res) {
    jresp[MSG_STATE] = 200;
  } else {
    jresp[MSG_STATE] = 201;
  }

  Json::FastWriter jfw;
  std::string sjson = jfw.write(jresp);
  DpClient_SendDpReply(dmp->method,
                       dmp->channel_id, dmp->id,
                       sjson.c_str(), sjson.size());
}

void CListenMessage::dpcli_poll_state_cb(DPPollHandle p_hdl, unsigned int n_state, void* p_usr_arg) {
  if (p_usr_arg) {
    ((CListenMessage*)p_usr_arg)->OnDpState(p_hdl, n_state);
  }
}

void CListenMessage::OnDpState(DPPollHandle p_hdl, unsigned int n_state) {
  if (n_state == DP_CLIENT_DISCONNECT) {
    int32 n_ret = DpClient_HdlReConnect(p_hdl);
    if (n_ret == VZNETDP_SUCCEED) {
      DpClient_HdlAddListenMessage(dp_cli_, METHOD_SET, METHOD_SET_SIZE);
    }
  }
}

void CListenMessage::OnMessage(vzbase::Message* p_msg) {
  if (p_msg->message_id == MSG_HDL_CHECK_DATABASE) {
    /*vzbase::TypedMessageData<std::string>::Ptr msg_ptr =
      boost::static_pointer_cast<vzbase::TypedMessageData< std::string >> (p_msg->pdata);*/

    database_.RemovePCount(90);

    vzbase::Thread::Current()->PostDelayed(2*1000, this, 0);
  }
}

}  // namespace bs
