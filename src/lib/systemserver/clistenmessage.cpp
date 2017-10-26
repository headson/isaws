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
unsigned int K_METHOD_NUM = sizeof(K_METHOD_SET) / sizeof(char*);

CListenMessage::CListenMessage()
  : dp_cli_(NULL)
  , network_(NULL)
  , hwclock_(NULL)
  , thread_slow_(NULL) {
}

CListenMessage::~CListenMessage() {
  Stop();
}

CListenMessage *CListenMessage::Instance() {
  VZBASE_DEFINE_STATIC_LOCAL(CListenMessage, listen_message, ());
  return &listen_message;
}

bool CListenMessage::Start() {
  bool bres = false;

  GetHwInfo();  // 获取系统信息

  // fast thread
  thread_fast_ = vzbase::Thread::Current();

  // slow thread
  thread_slow_ = new vzbase::Thread();
  if (NULL == thread_slow_) {
    LOG(L_ERROR) << "create thread failed.";
    return false;
  }
  bres = thread_slow_->Start();
  if (bres == false) {
    LOG(L_ERROR) << "slow thread start failed.";
    return false;
  }

  // net ctrl
  network_ = CNetwork::Create(
               sys_info_.dev_uuid, thread_fast_);
  if (NULL == network_) {
    LOG(L_ERROR) << "create net ctrl failed.";
    return false;
  }
  bres = network_->Start();
  if (bres == false) {
    LOG(L_ERROR) << "net ctrl start failed.";
    return false;
  }

  // hw clock
  hwclock_ = CHwclock::Create(thread_slow_);
  if (NULL == hwclock_) {
    LOG(L_ERROR) << "create hw clock failed.";
    return false;
  }
  bres = hwclock_->Start();
  if (false == bres) {
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
  DpClient_HdlAddListenMessage(dp_cli_, K_METHOD_SET, K_METHOD_NUM);

  OnMessage(NULL);
  return true;
}

void CListenMessage::Stop() {
  if (dp_cli_) {
    DpClient_ReleasePollHandle(dp_cli_);
    dp_cli_ = NULL;
  }

  if (network_) {
    delete network_;
    network_ = NULL;
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
  if (0 == strncmp(dmp->method, MSG_SET_DEVINFO, MAX_METHOD_SIZE)) {
    breply = true;
    if (SetDevInfo(jreq[MSG_BODY])) {
      jresp[MSG_STATE] = RET_SUCCESS;
    }
  } else if (0 == strncmp(dmp->method, MSG_GET_DEVINFO, MAX_METHOD_SIZE)) {
    breply = true;
    Json::Value jbody;
    if (GetDevInfo(jbody)) {
      jresp[MSG_BODY] = jbody;
      jresp[MSG_STATE] = RET_SUCCESS;
    }
  } else if (0 == strncmp(dmp->method, MSG_SET_DEV_NET, MAX_METHOD_SIZE)) {
    breply = true;
    if (network_ &&
        network_->SetNet(jreq[MSG_BODY])) {
      jresp[MSG_STATE] = RET_SUCCESS;
    }
  } else if (0 == strncmp(dmp->method, MSG_REBOOT_DEVICE, MAX_METHOD_SIZE)) {
    LOG(L_WARNING) << "reboot device.";
    if (!jreq[MSG_BODY]["resion"].isNull()) {
      LOG(L_ERROR) << jreq[MSG_BODY]["resion"].asString();
    }
    vzbase::my_system("sync; sleep 5; reboot &");  // 延迟5秒钟重启
  } else if (0 == strncmp(dmp->method, MSG_GET_TIMEINFO, MAX_METHOD_SIZE)) {
    breply = true;
    if (hwclock_) {
      if (hwclock_->GetTimeInfo(jresp[MSG_BODY])) {
        jresp[MSG_STATE] = RET_SUCCESS;
      }
    }
  } else if (0 == strncmp(dmp->method, MSG_SET_TIMEINFO, MAX_METHOD_SIZE)) {
    breply = true;
    if (hwclock_) {
      if (hwclock_->SetTimeInfo(jreq[MSG_BODY])) {
        jresp[MSG_STATE] = RET_SUCCESS;
      }
    }
  } else if (0 == strncmp(dmp->method, MSG_SET_DEVTIME, MAX_METHOD_SIZE)) {
    breply = true;
    if (hwclock_) {
      if (hwclock_->SetDevTime(jreq[MSG_BODY])) {
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
      DpClient_HdlAddListenMessage(dp_cli_, K_METHOD_SET, K_METHOD_NUM);
    }
  }
}

void CListenMessage::OnMessage(vzbase::Message* msg) {
  static int MCAST_TIMEOUT = 5;

  if ((MCAST_TIMEOUT--) <= 0) {
    Json::Value jbody;
    GetDevInfo(jbody);
    if (network_) {
      Json::FastWriter jfw;
      Json::Value jreq;
      jreq[MSG_CMD] = MSG_GET_DEVINFO;
      jreq[MSG_BODY] = jbody;
      std::string ss = jfw.write(jreq);
      network_->Boardcast(ss.c_str(), ss.size());
    }
  }
  thread_fast_->PostDelayed(1*1000, this);
}

}  // namespace sys
