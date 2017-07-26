/************************************************************************/
/* Author      : SoberPeng 2017-06-24
/* Description :
/************************************************************************/
#include "clistenmessage.h"
#include "vzbase/helper/stdafx.h"
#include "vzbase/base/vmessage.h"

#include "json/json.h"

namespace iva {

#define IVA_CFG_FILE    "./config/cfg_file_iva.xml"
#define IVA_AUX_FILE    "./config/cfg_file_aux.xml"

static const unsigned int METHOD_SET_SIZE = 3;
static const char  *METHOD_SET[] = {
  MSG_GET_IVAINFO,
  MSG_SET_IVAINFO
};

CListenMessage::CListenMessage()
  : dp_cli_(NULL)
  , main_thread_(NULL)
  , iva_Handle_(NULL) {
}

CListenMessage::~CListenMessage() {
  Stop();
}

CListenMessage *CListenMessage::Instance() {
  VZBASE_DEFINE_STATIC_LOCAL(CListenMessage, listen_message, ());
  return &listen_message;
}

bool CListenMessage::Start() {
  if (iva_Handle_) {
    iva_alg_destroy(iva_Handle_);
    iva_Handle_ = NULL;
  }

  /*算法创建*/
  ALG_CREATE_ARG alg_arg;
  alg_arg.iva_mode             = (uint8_t)5;
  alg_arg.config_filename      = (int8_t*)IVA_CFG_FILE;
  alg_arg.aux_config_filename  = (int8_t*)IVA_AUX_FILE;
  alg_arg.iva_debug_callback   = DebugCallback;
  alg_arg.iva_action_callback  = ActionCallback;

  alg_arg.face_img_w = (uint32_t)720;
  alg_arg.face_img_h = (uint32_t)576;
  alg_arg.door_img_w = (uint32_t)720;
  alg_arg.door_img_h = (uint32_t)576;
  alg_arg.env_img_w  = (uint32_t)720;
  alg_arg.env_img_h  = (uint32_t)576;

  alg_arg.user_arg = (uint32_t)this;
  LOG_INFO("iva mode %d, face w %d, h %d, door w %d, h %d.", alg_arg.iva_mode,
           alg_arg.face_img_w, alg_arg.face_img_h, alg_arg.door_img_w, alg_arg.door_img_h);

  int nret = iva_alg_create(&iva_Handle_, &alg_arg);
  if (nret != IVA_NO_ERROR) {
    LOG_ERROR("create iva failed, %d.", nret);
    return -1;
  }
  iva_alg_set_time_zone(iva_Handle_, 8);  // 传给设备时区

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
  if (main_thread_) {
    main_thread_->Release();
    main_thread_ = NULL;
  }

  if (iva_Handle_) {
    iva_alg_destroy(iva_Handle_);
    iva_Handle_ = NULL;
  }

  if (dp_cli_) {
    DpClient_ReleasePollHandle(dp_cli_);
    dp_cli_ = NULL;
  }
  DpClient_Stop();
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

}  // namespace iva
