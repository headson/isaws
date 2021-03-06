/************************************************************************/
/* Author      : SoberPeng 2017-06-24
/* Description :
/************************************************************************/
#include "clistenmessage.h"
#include "vzbase/helper/stdafx.h"
#include "vzbase/base/vmessage.h"
#include "client_hs/base/cmarkup.h"

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
  main_thread_ = vzbase::Thread::Current();

  if (!CreateAccessConnector(main_thread_)) {
    return false;
  }

  //////////////////////////////////////////////////////////////////////////
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

bool CListenMessage::CreateAccessConnector(vzbase::Thread *thread) {
  client_access_.reset(new CClientAccess(thread));

  std::string sAddr;
  int res = Kvdb_GetKeyToString(KVDB_ACCESS_ADDRESS,
                                strlen(KVDB_ACCESS_ADDRESS),
                                &sAddr);
  Json::Value jaddr;
  Json::Reader jread;
  if (res <= 0 || !jread.parse(sAddr, jaddr)) {
    jaddr.clear();
    jaddr["host"] = "www.baiweixun.com.cn";
    jaddr["port"] = 6001;

    Json::FastWriter jfw;
    sAddr = jfw.write(jaddr);
    Kvdb_SetKey(KVDB_ACCESS_ADDRESS,
                strlen(KVDB_ACCESS_ADDRESS),
                sAddr.c_str(), sAddr.size());
  }

  int nPort;
  std::string sHost;
  try {
    nPort = jaddr["port"].asInt();
    sHost = jaddr["host"].asString();
  } catch (...) {
  }
  sHost = "127.0.0.1";
  nPort = 6001;
  return client_access_->Start(sHost, nPort);
}

bool CListenMessage::CreateDispatchConnector(int nMain, int nMinor, const std::string &sXml) {
  std::string chn = "";
  CMarkupSTL cXml;
  cXml.SetDoc(sXml.c_str());
  if (!cXml.IsWellFormed()) {
    return false;
  }
  cXml.ResetMainPos();
  if (cXml.FindChildElem("StreamType"))
    chn = cXml.GetChildData();
  else
    return false;

  MAP_DISP::iterator it = client_dispatch_.find(chn);
  if (it != client_dispatch_.end()) {
    it->second->OnRespCreateChannel(nMinor);
    return true;
  } else {
    CClientDispatch::Ptr cli(new CClientDispatch(client_access_, main_thread_));
    if (cli.get()) {
      bool res = cli->Start(nMain, nMinor, sXml);
      if (res) {
        client_dispatch_[cli->GetChannel()] = cli;
        return true;
      }
    }
  }
  
  LOG(L_ERROR) << "create dispatch failed. " << sXml;
  return false;
}

bool CListenMessage::CreateStorageConnector(std::string sHost, int nPort) {
  return false;
}

}  // namespace cli
