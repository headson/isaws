/************************************************************************/
/* Author      : SoberPeng 2017-06-24
/* Description :
/************************************************************************/
#include "clistenmessage.h"

#include <fstream>
#include "json/json.h"

#include "vzbase/helper/stdafx.h"
#include "vzbase/base/vmessage.h"
#include "vzbase/system/mysystem.h"

#include "vzbase/base/helper.h"
#include "vzbase/base/base64.h"

#include "systemserver/network/net_cfg.h"

namespace sys {

static const char *K_METHOD_SET[] = {
  MSG_GET_DEVINFO,
  MSG_SET_DEVINFO,
  /*MSG_ADDR_CHANGE,*/
  MSG_SET_DEVTIME,
  MSG_GET_TIMEINFO,
  MSG_SET_TIMEINFO,
  MSG_REBOOT_DEVICE,
};

CListenMessage::CListenMessage()
  : dp_cli_(NULL)
  , mcast_dev_(NULL)
  , hw_clock_(NULL)
  , thread_slow_(NULL) {

  ip_change_ = 0;
  nickname_ = PHY_ETH_NAME;
}

CListenMessage::~CListenMessage() {
  Stop();
}

CListenMessage *CListenMessage::Instance() {
  VZBASE_DEFINE_STATIC_LOCAL(CListenMessage, listen_message, ());
  return &listen_message;
}

bool CListenMessage::Start() {
  bool b_ret = false;

  GetHwInfo();  // 获取系统信息

  // fast thread
  thread_fast_ = vzbase::Thread::Current();

  // slow thread
  thread_slow_ = new vzbase::Thread();
  if (NULL == thread_slow_) {
    LOG(L_ERROR) << "create thread failed.";
    return false;
  }
  b_ret = thread_slow_->Start();
  if (b_ret == false) {
    LOG(L_ERROR) << "slow thread start failed.";
    return false;
  }

  // net ctrl
  mcast_dev_ = CNetwork::Create(thread_fast_);
  if (NULL == mcast_dev_) {
    LOG(L_ERROR) << "create net ctrl failed.";
    return false;
  }

  b_ret = mcast_dev_->Start();
  if (b_ret == false) {
    LOG(L_ERROR) << "net ctrl start failed.";
    return false;
  }

  // hw clock
  hw_clock_ = CHwclock::Create(thread_slow_);
  if (NULL == hw_clock_) {
    LOG(L_ERROR) << "create hw clock failed.";
    return false;
  }
  b_ret = hw_clock_->Start();
  if (false == b_ret) {
    LOG(L_ERROR) << "hwclock start failed.";
    return false;
  }

  // dp client
  vzconn::EventService *p_evt_srv =
    thread_fast_->socketserver()->GetEvtService();
  dp_cli_ = DpClient_CreatePollHandle(dpcli_poll_msg_cb, this,
                                      dpcli_poll_state_cb, this,
                                      p_evt_srv);
  if (dp_cli_ == NULL) {
    LOG(L_ERROR) << "dp client create poll handle failed.";

    DpClient_ReleasePollHandle(dp_cli_);
    dp_cli_ = NULL;
    return false;
  }

  unsigned int n_method_set = sizeof(K_METHOD_SET) / sizeof(char*);
  DpClient_HdlAddListenMessage(dp_cli_, K_METHOD_SET, n_method_set);
  return true;
}

void CListenMessage::Stop() {
  if (dp_cli_) {
    DpClient_ReleasePollHandle(dp_cli_);
    dp_cli_ = NULL;
  }

  if (mcast_dev_) {
    delete mcast_dev_;
    mcast_dev_ = NULL;
  }

  if (thread_slow_) {
    thread_slow_->Release();
    thread_slow_ = NULL;
  }
  DpClient_Stop();

  thread_fast_->Release();
}

void CListenMessage::RunLoop() {
  thread_fast_->ProcessMessages(4 * 1000);

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

void CListenMessage::dpcli_poll_msg_cb(DPPollHandle p_hdl,
                                       const DpMessage *dmp,
                                       void* p_usr_arg) {
  if (p_usr_arg) {
    ((CListenMessage*)p_usr_arg)->OnDpMessage(p_hdl, dmp);
    return;
  }
  LOG(L_ERROR) << "param is error.";
}

void CListenMessage::OnDpMessage(DPPollHandle p_hdl, const DpMessage *dmp) {
  std::string   sjson;
  sjson.append(dmp->data, dmp->data_size);

  Json::Value   jreq;
  Json::Reader  jread;
  if (!jread.parse(sjson, jreq)) {
    LOG(L_ERROR) << "iva info parse failed.";
    return;
  }

  Json::Value   jresp;
  jresp[MSG_CMD]   = jreq[MSG_CMD].asString();
  jresp[MSG_ID]    = jreq[MSG_ID].asInt();
  jresp[MSG_STATE] = RET_FAILED;

  bool breply = false; // 返回
  if (0 == strncmp(dmp->method, MSG_SET_DEVINFO, dmp->method_size)) {
    breply = true;
    if (SetDevInfo(jreq[MSG_BODY])) {
      jresp[MSG_STATE] = RET_SUCCESS;
    }
  } else if (0 == strncmp(dmp->method, MSG_GET_DEVINFO, dmp->method_size)) {
    breply = true;
    Json::Value jbody;
    if (GetDevInfo(jbody)) {
      jresp[MSG_BODY] = jbody;
      jresp[MSG_STATE] = RET_SUCCESS;
    }
  } else if (0 == strncmp(dmp->method, MSG_REBOOT_DEVICE, dmp->method_size)) {
    LOG(L_WARNING) << "reboot device.";
    if (!jreq[MSG_BODY]["resion"].isNull()) {
      LOG(L_ERROR) << jreq[MSG_BODY]["resion"].asString();
    }
    vzbase::my_system("sync; sleep 5; reboot &");  // 延迟5秒钟重启
  } else if (0 == strncmp(dmp->method, MSG_GET_TIMEINFO, dmp->method_size)) {
    breply = true;
    if (hw_clock_) {
      if (hw_clock_->GetTimeInfo(jresp[MSG_BODY])) {
        jresp[MSG_STATE] = RET_SUCCESS;
      }
    }
  } else if (0 == strncmp(dmp->method, MSG_SET_TIMEINFO, dmp->method_size)) {
    breply = true;
    if (hw_clock_) {
      if (hw_clock_->SetTimeInfo(jreq[MSG_BODY])) {
        jresp[MSG_STATE] = RET_SUCCESS;
      }
    }
  } else if (0 == strncmp(dmp->method, MSG_SET_DEVTIME, dmp->method_size)) {
    breply = true;
    if (hw_clock_) {
      if (hw_clock_->SetDevTime(jreq[MSG_BODY])) {
        jresp[MSG_STATE] = RET_SUCCESS;
      }
    }
  }

  if (breply) {
    Json::FastWriter jfw;
    sjson = jfw.write(jresp);
    DpClient_SendDpReply(dmp->method,
                         dmp->channel_id,
                         dmp->id,
                         sjson.c_str(), sjson.size());
  }
}

void CListenMessage::dpcli_poll_state_cb(DPPollHandle p_hdl,
    unsigned int n_state,
    void* p_usr_arg) {
  if (p_usr_arg) {
    ((CListenMessage*)p_usr_arg)->OnDpState(p_hdl, n_state);
  }
}

void CListenMessage::OnDpState(DPPollHandle p_hdl, unsigned int n_state) {
  if (n_state == DP_CLIENT_DISCONNECT) {
    int32 n_ret = DpClient_HdlReConnect(p_hdl);
    if (n_ret == VZNETDP_SUCCEED) {
      unsigned int n_method_set = sizeof(K_METHOD_SET) / sizeof(char*);
      DpClient_HdlAddListenMessage(dp_cli_, K_METHOD_SET, n_method_set);
    }
  }
}

}  // namespace sys
