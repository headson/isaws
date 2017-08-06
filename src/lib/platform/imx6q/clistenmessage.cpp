/************************************************************************/
/* Author      : SoberPeng 2017-06-24
/* Description :
/************************************************************************/
#include "clistenmessage.h"
#include "vzbase/helper/stdafx.h"
#include "vzbase/base/vmessage.h"

#include "json/json.h"

#include "alg/basedefine.h"

namespace imx6q {

static const unsigned int METHOD_SET_SIZE = 3;
static const char  *METHOD_SET[] = {
  MSG_GET_I_FRAME,
  MSG_GET_ENC_CFG,
  MSG_SET_ENC_CFG,
  MSG_CATCH_EVENT
};

CListenMessage::CListenMessage()
  : dp_cli_(NULL)
  , main_thread_(NULL)
  , vdo_enc_() {
}

CListenMessage::~CListenMessage() {
  Stop();
}

CListenMessage *CListenMessage::Instance() {
  VZBASE_DEFINE_STATIC_LOCAL(CListenMessage, listen_message, ());
  return &listen_message;
}

bool CListenMessage::Start(const char *dev_vdo) {
  vdo_enc_.SetVideo(dev_vdo);
  vdo_enc_.SetInput(0);
  vdo_enc_.SetViSize(SHM_VIDEO_0_W, SHM_VIDEO_0_H);
  vdo_enc_.SetEncSize(SHM_VIDEO_0_W, SHM_VIDEO_0_H);
  bool bret = vdo_enc_.Start(SHM_VIDEO_0, SHM_VIDEO_0_SIZE,
                             SHM_IMAGE_0, SHM_IMAGE_0_SIZE);
  if (false == bret) {
    LOG(L_ERROR) << "vdo encode start failed.";
    return false;
  }

  vdo_thread_ = new vzbase::Thread();
  if (NULL == vdo_thread_) {
    LOG(L_ERROR) << "Create vdo thread failed.";
    return false;
  }
  vdo_thread_->Start(&vdo_enc_);

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
  vdo_enc_.Stop();

  if (dp_cli_) {
    DpClient_ReleasePollHandle(dp_cli_);
    dp_cli_ = NULL;
  }
  DpClient_Stop();

  if (vdo_thread_) {
    vdo_thread_->Release();
    vdo_thread_ = NULL;
  }

  if (main_thread_) {
    main_thread_->Release();
    main_thread_ = NULL;
  }
}

void CListenMessage::RunLoop() {
  main_thread_->Run();
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
    dmp->data + dmp->data_size,
    jreq)) {
    LOG(L_ERROR) << "Json parse failed.";
    return;
  }
  Json::Value jresp;
  jresp[MSG_CMD] = jreq[MSG_CMD].asString();
  jresp[MSG_ID] = jreq[MSG_ID].asInt();

  bool res = false;
  if (0 == strncmp(dmp->method, MSG_CATCH_EVENT, MAX_METHOD_SIZE)) {
    jreq[MSG_BODY][ALG_POSITIVE_NUMBER].asInt();
    jreq[MSG_BODY][ALG_NEGATIVE_NUMBER].asInt();
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
}  // namespace imx6q
