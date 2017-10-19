/************************************************************************/
/* Author      : SoberPeng 2017-06-24
/* Description :
/************************************************************************/
#include "clistenmessage.h"
#include "vzbase/helper/stdafx.h"
#include "vzbase/base/vmessage.h"

#include "json/json.h"

#include <fstream>

namespace alg {

static const char  *METHOD_SET[] = {
  MSG_REMOTE_5_IR,
  MSG_GET_ALGINFO,
  MSG_SET_ALGINFO,
  MSG_RESET_PCNUM
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
  bool res = false;

  res = create_share_memory();
  if (!res) {
    return res;
  }

  /* 消息poll */
  main_thread_ = vzbase::Thread::Current();
  if (dp_cli_ == NULL) {
    vzconn::EventService *evt_srv =
      main_thread_->socketserver()->GetEvtService();

    dp_cli_ = DpClient_CreatePollHandle(dpcli_poll_msg_cb, this,
                                        dpcli_poll_state_cb, this,
                                        evt_srv);
    if (dp_cli_ == NULL) {
      LOG(L_ERROR) << "dp client create poll handle failed.";

      DpClient_ReleasePollHandle(dp_cli_);
      dp_cli_ = NULL;
      return false;
    }

    DpClient_HdlAddListenMessage(dp_cli_, METHOD_SET, METHOD_SET_SIZE);
  }

  main_thread_->PostDelayed(POLL_TIMEOUT, this, CATCH_IMAGE);
  return true;
}

void CListenMessage::Stop() {
  if (main_thread_) {
    main_thread_->Release();
    main_thread_ = NULL;
  }

  if (alg_handle_) {
    iva_alg_destroy(alg_handle_);
    alg_handle_ = NULL;
  }

  if (dp_cli_) {
    DpClient_ReleasePollHandle(dp_cli_);
    dp_cli_ = NULL;
  }
  DpClient_Stop();
}

void CListenMessage::RunLoop() {
  while (true) {
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
}

vzbase::Thread *CListenMessage::MainThread() {
  return main_thread_;
}

bool CListenMessage::create_share_memory() {
  bool bret = false;

  // share image
  for (int i = 0; i < MAX_SHM_IMG; i++) {
    TAG_SHM_ARG &shm_img = g_shm_img[i];

    bret = shm_img_[i].shm_img.Open(shm_img.dev_name,
                                    shm_img.shm_size);
    if (false == bret) {
      LOG(L_ERROR) << "open share failed." << shm_img.dev_name;
      return false;
    }
    shm_img_[i].shm_img_ptr = (TAG_SHM_IMG*)shm_img_[i].shm_img.GetData();
  }

  // share video 2
  TAG_SHM_ARG &shm_vdo_2 = g_shm_avdo[2];
  bret = shm_vdo_.Open(shm_vdo_2.dev_name,
                         shm_vdo_2.shm_size);
  if (false == bret) {
    LOG(L_ERROR) << "open share failed." << shm_vdo_2.dev_name;
    return false;
  }
  shm_vdo_ptr_ = (TAG_SHM_VDO*)shm_vdo_.GetData();
  shm_vdo_ptr_->width = shm_vdo_2.width;
  shm_vdo_ptr_->height = shm_vdo_2.height;
  return true;
}

bool CListenMessage::create_alg_handle() {
  /*算法创建*/
  ALG_CREATE_ARG param;
  memset(&param, 0, sizeof(param));
  param.config_filename = (int8*)DEF_ALG_CONFIG_FILE;
  param.aux_config_filename = (int8*)DEF_AUX_CONFIG_FILE;
  param.iva_debug_callback = AlgDebugCallback;
  param.iva_action_callback = AlgActionCallback;
  param.face_img_w = g_shm_img[0].width;
  param.face_img_h = g_shm_img[0].height;
  param.door_img_w = g_shm_img[1].width;
  param.door_img_h = g_shm_img[1].height;
  param.user_arg   = (uint32)this;
  param.iva_mode   = 3;

  int nret = iva_alg_create(&alg_handle_, &param);
  if (nret != IVA_NO_ERROR) {
    LOG_ERROR("create iva failed, %d.", nret);
    return -1;
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
  int nret = IVA_NO_ERROR;

  Json::Value jresp;
  jresp[MSG_CMD]  = dmp->method;
  //jresp[MSG_BODY] = "";
  if (strncmp(dmp->method, MSG_GET_ALGINFO, MAX_METHOD_SIZE) == 0) {
    // 获取算法参数
    char sver[64] = {0};
    /*if (IVA_NO_ERROR == sdk_iva_get_version(sver)) {
      nret = RET_SUCCESS;

      std::string ss = sver;
      jresp[MSG_BODY]["version"] = ss;
      }*/
  } else if (strncmp(dmp->method, MSG_SET_ALGINFO, MAX_METHOD_SIZE) == 0) {
    // 配置算法参数
  }
  //else if (strncmp(dmp->method, MSG_RESET_PCNUM, MAX_METHOD_SIZE) == 0) {
  //  // 重置统计数
  //  if (alg_handle_) {
  //    nret = sdk_iva_set_control_command(alg_handle_,
  //                                       CONTROL_COMMAND_RESET_COUNTER);
  //    if (nret == IVA_ERROR_NO_ERROR) {
  //      nret = RET_SUCCESS;
  //    }
  //    LOG(L_INFO) << "reset pcount " << nret;
  //  }
  //}

  if (dmp->type == TYPE_REQUEST) {
    jresp[MSG_STATE] = nret;

    Json::FastWriter jout;
    std::string sjson = jout.write(jresp);
    DpClient_SendDpReply(dmp->method, 0, dmp->id,
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
    int nret = DpClient_HdlReConnect(phdl);
    if (nret == VZNETDP_SUCCEED) {
      DpClient_HdlAddListenMessage(dp_cli_, METHOD_SET, METHOD_SET_SIZE);
    }
  } else if (nstate == DP_POLL_ISNOT_REG_MSG) {
    // 未注册消息

  }
}

void CListenMessage::OnMessage(vzbase::Message* p_msg) {
  if (p_msg->message_id == CATCH_IMAGE) {
    main_thread_->PostDelayed(POLL_TIMEOUT, this, CATCH_IMAGE);

    for (int i = 0; i < MAX_SHM_IMG; i++) {
      if (shm_img_[i].w_sec == shm_img_[i].shm_img_ptr->wsec &&
          shm_img_[i].w_usec == shm_img_[i].shm_img_ptr->wusec) {
        continue;
      }
      shm_img_[i].w_sec = shm_img_[i].shm_img_ptr->wsec;
      shm_img_[i].w_usec = shm_img_[i].shm_img_ptr->wusec;

      IVA_FRAME frame;
      frame.type = i;
      frame.data = shm_img_[i].shm_img_ptr->pdata;
      frame.datalen = shm_img_[i].shm_img_ptr->ndata;
      int res = iva_alg_read_one_frame(alg_handle_, &frame);
      if (res != IVA_NO_ERROR) {
        LOG(L_WARNING) << "iva_alg_read_one_frame " << i << " " << res;
      }
    }
  }
}

}  // namespace alg

