/************************************************************************/
/* Author      : SoberPeng 2017-06-24
/* Description :
/************************************************************************/
#include "clistenmessage.h"
#include "vzbase/helper/stdafx.h"
#include "vzbase/base/vmessage.h"

#include "json/json.h"

namespace cli {

static const unsigned int METHOD_SET_SIZE = 3;
static const char  *METHOD_SET[] = {
  MSG_GET_I_FRAME,
  MSG_GET_ENC_CFG,
  MSG_SET_ENC_CFG
};

CListenMessage::CListenMessage()
  : dp_cli_(NULL)
  , main_thread_(NULL) {
}

CListenMessage::~CListenMessage() {
  Stop();
}

CListenMessage *CListenMessage::Instance() {
  VZBASE_DEFINE_STATIC_LOCAL(CListenMessage, listen_message, ());
  return &listen_message;
}

bool CListenMessage::Start() {

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
  return true;
}

void CListenMessage::Stop() {
  if (dp_cli_) {
    DpClient_ReleasePollHandle(dp_cli_);
    dp_cli_ = NULL;
  }
  DpClient_Stop();

  if (main_thread_) {
    main_thread_->Release();
    main_thread_ = NULL;
  }
}

void CListenMessage::RunLoop() {
  main_thread_->ProcessMessages(4 * 1000);

  static void *hdl_watchdog = NULL;
  if (hdl_watchdog == NULL) {
    hdl_watchdog = RegisterWatchDogKey(
                     "MAIN", 4, DEF_WATCHDOG_TIMEOUT);
  }

  static time_t old_time = time(NULL);
  time_t now_time = time(NULL);
  if (abs(now_time - old_time) >= DEF_FEEDDOG_TIME) {
    old_time = now_time;
    if (hdl_watchdog) {
      FeedDog(hdl_watchdog);
    }
  }
}

vzbase::Thread *CListenMessage::MainThread() {
  return main_thread_;
}

void CListenMessage::dpcli_poll_msg_cb(DPPollHandle p_hdl, 
                                       const DpMessage *dmp, void* p_usr_arg) {
  if (p_usr_arg) {
    ((CListenMessage*)p_usr_arg)->OnDpMessage(p_hdl, dmp);
    return;
  }
  LOG(L_ERROR) << "param is error.";
}

void CListenMessage::OnDpMessage(DPPollHandle p_hdl, const DpMessage *dmp) {

}

void CListenMessage::dpcli_poll_state_cb(DPPollHandle p_hdl, 
                                         unsigned int n_state, void* p_usr_arg) {
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
  //if (p_msg->message_id == THREAD_MSG_SET_DEV_ADDR) {
  /*vzbase::TypedMessageData<std::string>::Ptr msg_ptr =
    boost::static_pointer_cast<vzbase::TypedMessageData< std::string >> (p_msg->pdata);*/

  //Restart("127.0.0.1", 5291);
  //vzbase::Thread::Current()->PostDelayed(2*1000, this, THREAD_MSG_SET_DEV_ADDR);
  //}
}
}  // namespace cli
