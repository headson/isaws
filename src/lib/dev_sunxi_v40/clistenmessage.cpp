/************************************************************************/
/* Author      : SoberPeng 2017-06-24
/* Description :
/************************************************************************/
#include "clistenmessage.h"
#include "vzbase/helper/stdafx.h"
#include "vzbase/base/vmessage.h"

#include "json/json.h"

namespace platform {

static const char  *METHOD_SET[] = {
  MSG_GET_I_FRAME,        //
  MSG_IRCUT_CTRLS,        //
  MSG_GET_ENC_CFG,        //
  MSG_SET_ENC_CFG,        //
  MSG_GET_ENC_OSD,        //
  MSG_SET_ENC_OSD         //
};
static const unsigned int METHOD_SET_SIZE = sizeof(METHOD_SET) / sizeof(char*);

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
  // ��ʼ��
  video_catch_.Start(0);

  // �����¼�
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
  bool bret = false;
  return true;
}

void CListenMessage::Stop() {
  video_catch_.Stop();

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
                   dmp->data + dmp->data_size,
                   jreq)) {
    LOG(L_ERROR) << "Json parse failed.";
    return;
  }
  LOG(L_INFO) <<dmp->method
              << "\n" << jreq.toStyledString();

  Json::Value jresp;
  jresp[MSG_CMD] = jreq[MSG_CMD].asString();
  jresp[MSG_ID] = jreq[MSG_ID].asInt();

  bool reply = false;
  if (0 == strncmp(dmp->method, MSG_IRCUT_CTRLS, MAX_METHOD_SIZE)) {
    reply = true;

    // GPIO8_0
    //if (jreq[MSG_BODY]["switch"].asInt() > 0) {
    //  CGpioEvent::IRCutOpen();
    //} else {
    //  CGpioEvent::IRCutClose();
    //}
    jresp[MSG_STATE] = RET_SUCCESS;
  } else if (0 == strncmp(dmp->method, MSG_GET_I_FRAME, MAX_METHOD_SIZE)) {
    // HI_MPI_VENC_RequestIDR(jreq[MSG_BODY]["venc_chn"].asInt(), HI_TRUE);
  } else if (0 == strncmp(dmp->method, MSG_GET_ENC_CFG, MAX_METHOD_SIZE)) {
    // TODO ��ȡ�������
  } else if (0 == strncmp(dmp->method, MSG_SET_ENC_CFG, MAX_METHOD_SIZE)) {
    // TODO ���ñ������
  } else if (0 == strncmp(dmp->method, MSG_GET_ENC_OSD, MAX_METHOD_SIZE)) {
    reply = true;
    jresp[MSG_STATE] = RET_KVDB_READ_FAILED;

    std::string sbody = "";
    Kvdb_GetKeyToString(KVDB_ENC_OSD, strlen(KVDB_ENC_OSD), &sbody);
    if (!sbody.empty()) {
      LOG(L_INFO) << sbody;
      Json::Value jbody;
      Json::Reader jread;
      if (jread.parse(sbody, jbody)) {
        jresp[MSG_STATE] = RET_SUCCESS;
        jresp[MSG_BODY] = jbody;
      }
    }
  } else if (0 == strncmp(dmp->method, MSG_SET_ENC_OSD, MAX_METHOD_SIZE)) {
    reply = true;
    jresp[MSG_STATE] = RET_FAILED;

    /*bool res = vdo_catch_->OSDAdjust(jreq[MSG_BODY]);
    if (res) {
    Json::Value josd = jreq[MSG_BODY];
    std::string sosd = josd.toStyledString();
    LOG(L_INFO) << sosd;
    int res = Kvdb_SetKey(KVDB_ENC_OSD, strlen(KVDB_ENC_OSD),
    sosd.c_str(), sosd.size());
    if (res == KVDB_RET_SUCCEED) {
    jresp[MSG_STATE] = RET_SUCCESS;
    }
    }*/
  }

  if (reply) {
    Json::FastWriter jfw;
    std::string sjson = jfw.write(jresp);
    DpClient_SendDpReply(dmp->method, dmp->channel_id, dmp->id,
                         sjson.c_str(), sjson.size());
  }
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
  //if (p_msg->message_id == THREAD_MSG_SET_DEV_ADDR) {
  /*vzbase::TypedMessageData<std::string>::Ptr msg_ptr =
    boost::static_pointer_cast<vzbase::TypedMessageData< std::string >> (p_msg->pdata);*/

  //Restart("127.0.0.1", 5291);
  //vzbase::Thread::Current()->PostDelayed(2*1000, this, THREAD_MSG_SET_DEV_ADDR);
  //}
}
}  // namespace platform
