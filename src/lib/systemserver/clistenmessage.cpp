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

#include "systemserver/system/csystem.h"
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
unsigned int K_METHOD_SIZE = sizeof(K_METHOD_SET) / sizeof(char*);

CListenMessage::CListenMessage()
  : dp_cli_(NULL)
  , thread_fast_(NULL)
  , thread_slow_(NULL)
  , system_(NULL)
  , network_(NULL)
  , hwclock_(NULL) {
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

  // system
  system_ = CSystem::Create();
  if (NULL == system_) {
    LOG(L_ERROR) << "create system failed.";
    return false;
  }
  bres = system_->Start();
  if (false == bres) {
    LOG(L_ERROR) << "system start failed.";
    return false;
  }

  // net ctrl
<<<<<<< HEAD
  mcast_dev_ = CNetwork::Create(thread_fast_);
  if (NULL == mcast_dev_) {
=======
  network_ = CNetwork::Create(thread_fast_);
  if (NULL == network_) {
>>>>>>> 7bf3daa37cb02fa856739e7bd691a4a2bfd6f2cf
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
  DpClient_HdlAddListenMessage(dp_cli_, K_METHOD_SET, K_METHOD_SIZE);

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
  Json::Reader  jread;
  Json::Value   jreq, jret;

  std::string   sjson;
  sjson.append(dmp->data, dmp->data_size);
  if (!jread.parse(sjson, jreq)) {
    LOG(L_ERROR) << "req parse failed.";
    return;
  }
  try {
    jret[MSG_CMD] = jreq[MSG_CMD].asString();
    jret[MSG_ID] = jreq[MSG_ID].asInt();
    jret[MSG_STATE] = RET_FAILED;
  } catch (...) {
    LOG(L_ERROR) << "get req key failed.";
    return;
  }

  bool breply = false; // ·µ»Ø
  if (0 == strncmp(dmp->method, DP_CMD_DEV, MAX_METHOD_SIZE)) {
    if (system_) {
      breply = system_->OnDpMessage(dmp, jreq, jret);
    }
  } else if (0 == strncmp(dmp->method, DP_CMD_NET, MAX_METHOD_SIZE)) {
    if (network_) {
      breply = network_->OnDpMessage(dmp, jreq, jret);
    }
  } else if (0 == strncmp(dmp->method, DP_CMD_TIME, MAX_METHOD_SIZE)) {
    if (hwclock_) {
      hwclock_->OnDpMessage(dmp, jreq, jret);
    }
  }
  if (breply || dmp->type == TYPE_REQUEST) {
    Json::FastWriter jfw;
    sjson = jfw.write(jret);
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

void CListenMessage::OnDpState(DPPollHandle phdl, unsigned int nstate) {
  if (nstate == DP_CLIENT_DISCONNECT) {
    int32 n_ret = DpClient_HdlReConnect(phdl);
    if (n_ret == VZNETDP_SUCCEED) {
      DpClient_HdlAddListenMessage(phdl, K_METHOD_SET, K_METHOD_SIZE);
    }
  }/* else if (nstate == DP_POLL_ISNOT_REG_MSG) {
    DpClient_HdlAddListenMessage(phdl, K_METHOD_SET, K_METHOD_SIZE);
    }*/
}

}  // namespace sys
