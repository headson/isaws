/************************************************************************/
/* Author      : SoberPeng 2017-06-24
/* Description :
/************************************************************************/
#include "clistenmessage.h"
#include "vzbase/helper/stdafx.h"

#include "json/json.h"
#include "vzbase/base/base64.h"

#include "process/uri_handle.h"
#include "dispatcher/sync/cdppollclient.h"

namespace web {

const char *K_METHOD_SET[MAX_METHOD_COUNT] = {NULL};

CListenMessage::CListenMessage()
  : p_dp_cli_(NULL)
  , c_web_srv_() {
  for (int i = 0; i < MAX_METHOD_COUNT; i++) {
    K_METHOD_SET[i] = NULL;
  }
}

CListenMessage::~CListenMessage() {
  Stop();
}

CListenMessage *CListenMessage::Instance() {
  VZBASE_DEFINE_STATIC_LOCAL(CListenMessage, listen_message, ());
  return &listen_message;
}

bool CListenMessage::Start(const char     *s_dp_ip,
                           unsigned short  n_dp_port,
                           unsigned short  n_http_port,
                           const char     *s_http_path) {

  bool b_ret = false;
  char s_port[9] = {0};
  snprintf(s_port, 8, "%d", n_http_port);
  b_ret = c_web_srv_.Start(s_http_path, s_port);
  if (b_ret == false) {
    LOG(L_ERROR) << "start web server failed.";
    exit(EXIT_FAILURE);
  }

  // dp client poll
  int32 n_ret = VZNETDP_SUCCEED;
  DpClient_Init((char*)s_dp_ip, n_dp_port);
  n_ret = DpClient_Start(0);
  if (n_ret == VZNETDP_FAILURE) {
    LOG(L_ERROR) << "dp client start failed.";
    return false;
  }

  if (p_dp_cli_ == NULL) {
    p_evt_srv_ = vzbase::Thread::Current()->socketserver()->GetEvtService();

    p_dp_cli_ = DpClient_CreatePollHandle(dpcli_poll_msg_cb, this,
                                          dpcli_poll_state_cb, this,
                                          p_evt_srv_);
    if (p_dp_cli_ == NULL) {
      LOG(L_ERROR) << "dp client create poll handle failed.";

      DpClient_ReleasePollHandle(p_dp_cli_);
      p_dp_cli_ = NULL;
      return false;
    }

    DpClient_HdlAddListenMessage(p_dp_cli_, K_METHOD_SET, MAX_METHOD_COUNT);
  }
  return (n_ret == VZNETDP_SUCCEED);
}

bool CListenMessage::Restart(const char *s_dp_ip, unsigned short n_dp_port) {
  if (NULL != p_dp_cli_) {
    DpClient_ReleasePollHandle(p_dp_cli_);
    p_dp_cli_ = NULL;

    for (int i = 0; i < MAX_METHOD_COUNT; i++) {
      if (K_METHOD_SET[i] != NULL) {
        delete K_METHOD_SET[i];
        K_METHOD_SET[i] = NULL;
      }
    }
  }
  if (NULL != p_dp_cli_) {
    return true;
  }

  int32 n_ret = VZNETDP_SUCCEED;
  DpClient_Init((char*)s_dp_ip, n_dp_port);
  n_ret = DpClient_Start(0);
  if (n_ret == VZNETDP_FAILURE) {
    LOG(L_ERROR) << "dp client start failed.";
    return false;
  }

  p_dp_cli_ = DpClient_CreatePollHandle(dpcli_poll_msg_cb, this,
                                        dpcli_poll_state_cb, this,
                                        p_evt_srv_);
  if (p_dp_cli_ == NULL) {
    LOG(L_ERROR) << "dp client create poll handle failed.";

    DpClient_ReleasePollHandle(p_dp_cli_);
    p_dp_cli_ = NULL;
    return false;
  } else {
    CDpPollClient *cli_ = (CDpPollClient*)p_dp_cli_;
    if (cli_->isClose()) {
      return false;
    }
  }

  DpClient_HdlAddListenMessage(p_dp_cli_, K_METHOD_SET, MAX_METHOD_COUNT);
  return true;
}

void CListenMessage::Stop() {
  if (p_dp_cli_) {
    DpClient_ReleasePollHandle(p_dp_cli_);
    p_dp_cli_ = NULL;
  }
  DpClient_Stop();

  c_web_srv_.Stop();

  vzbase::Thread::Current()->Release();
}

void CListenMessage::RunLoop() {
  vzbase::Thread::Current()->Run();

  /*while (true) {
    DpClient_PollDpMessage(p_dp_cli_, 10);
    }*/
}

vzbase::Thread *CListenMessage::MainThread() {
  return vzbase::Thread::Current();
}

bool CListenMessage::AddListenMessage(std::string s_method) {
  int i = 0;
#if 1
  for (i = 0; i < MAX_METHOD_COUNT; i++) {
    if (K_METHOD_SET[i] == NULL) {
      continue;
    }

    if (0 == strncmp(K_METHOD_SET[i],
                     s_method.c_str(),
                     MAX_METHOD_SIZE)) {
      break;
    }
  }

  if (i == MAX_METHOD_COUNT) {
    for (i = 0; i < MAX_METHOD_COUNT; i++) {
      if (K_METHOD_SET[i] == NULL) {
        char *p_method = new char[s_method.size() + 1];
        memcpy(p_method, s_method.c_str(), s_method.size());
        p_method[s_method.size()] = '\0';

        K_METHOD_SET[i] = p_method;
        break;
      }
    }
  }

#endif
  if (i != MAX_METHOD_COUNT) {
    int n_ret = DpClient_HdlAddListenMessage(p_dp_cli_,
                K_METHOD_SET, MAX_METHOD_COUNT);
    if (n_ret == VZNETDP_SUCCEED) {
      return true;
    }
  }
  return false;
}

bool CListenMessage::DelListenMessage(std::string s_method) {
  int i = 0;
#if 1
  for (i = 0; i < MAX_METHOD_COUNT; i++) {
    if (K_METHOD_SET[i] == NULL) {
      continue;
    }

    if (0 == strncmp(K_METHOD_SET[i],
                     s_method.c_str(),
                     MAX_METHOD_SIZE)) {
      break;
    }
  }
#endif

  if (i != MAX_METHOD_COUNT) {
    int n_ret = DpClient_HdlRemoveListenMessage(
                  p_dp_cli_, &K_METHOD_SET[i], 1);
    if (n_ret == VZNETDP_SUCCEED) {
      delete K_METHOD_SET[i];
      K_METHOD_SET[i] = NULL;
      return true;
    }
  }
  return false;
}

void CListenMessage::dpcli_poll_msg_cb(DPPollHandle p_hdl, const DpMessage *dmp, void* p_usr_arg) {
  LOG(L_INFO) << "poll message.";
  if (p_usr_arg) {
    ((CListenMessage*)p_usr_arg)->OnDpCliMsg(p_hdl, dmp);
    return;
  }
  LOG(L_ERROR) << "param is error.";
}

void CListenMessage::OnDpCliMsg(DPPollHandle p_hdl, const DpMessage *dmp) {

  Json::Value j_msg;
  j_msg["method"] = dmp->method;
  j_msg["id"]     = dmp->id;
  j_msg["time"]   = GetNowTime();;

  std::string s_data;
  vzbase::Base64::EncodeFromArray(dmp->data,
                                  dmp->data_size,
                                  (std::string *)&s_data);
  j_msg["data"] = s_data;
  if (k_message.size() > 8) {
    k_message.pop_front();
  }
  k_message.push_back(j_msg);
}

void CListenMessage::dpcli_poll_state_cb(DPPollHandle p_hdl, unsigned int n_state, void* p_usr_arg) {
  if (p_usr_arg) {
    ((CListenMessage*)p_usr_arg)->OnDpCliState(p_hdl, n_state);
  }
}

void CListenMessage::OnDpCliState(DPPollHandle p_hdl, unsigned int n_state) {
  if (n_state == DP_CLIENT_DISCONNECT) {
    int32 n_ret = DpClient_HdlReConnect(p_hdl);
    if (n_ret == VZNETDP_SUCCEED) {
      DpClient_HdlAddListenMessage(p_dp_cli_, K_METHOD_SET, MAX_METHOD_COUNT);
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

}  // namespace web
