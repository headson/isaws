/************************************************************************/
/* Author      : SoberPeng 2017-06-24
/* Description :
/************************************************************************/
#include "clistenmessage.h"
#include "vzbase/helper/stdafx.h"
#include "vzbase/base/vmessage.h"

#include "json/json.h"

#include "alg/basedefine.h"

namespace hi3518e {

static const unsigned int METHOD_SET_SIZE = 3;
static const char  *METHOD_SET[] = {
  MSG_CATCH_EVENT,
  MSG_GET_I_FRAME,
  MSG_IRCUT_CTRLS,
  MSG_GET_ENC_CFG,
  MSG_SET_ENC_CFG
};

CListenMessage::CListenMessage()
  : dp_cli_(NULL)
  , main_thread_(NULL)
  , video_catch_() {
  // GPIO8_0 output
  HI_MPI_SYS_SetReg(0x200f0100, 0x1);

  HI_U32 nval = 0;
  HI_MPI_SYS_GetReg(0x201C0400, &nval);
  nval |= 0x1;
  HI_MPI_SYS_SetReg(0x201C0400, nval);
  HI_MPI_SYS_SetReg(0x201c0004, 0x0);   // default value
  
  // GPIO0_3 output
  HI_MPI_SYS_SetReg(0x200F0030, 0x0);

  nval = 0;
  HI_MPI_SYS_GetReg(0x20140400, &nval);
  nval |= 0x8;
  HI_MPI_SYS_SetReg(0x20140400, nval);
  HI_MPI_SYS_SetReg(0x20140020, 0x0);   // default value
}

CListenMessage::~CListenMessage() {
  Stop();
}

CListenMessage *CListenMessage::Instance() {
  VZBASE_DEFINE_STATIC_LOCAL(CListenMessage, listen_message, ());
  return &listen_message;
}

bool CListenMessage::Start() {
  bool bret = video_catch_.Start();
  if (false == bret) {
    LOG(L_ERROR) << "vdo encode start failed.";
    return false;
  }

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
  LOG(L_INFO) <<"\n" << jreq.toStyledString();

  Json::Value jresp;
  jresp[MSG_CMD] = jreq[MSG_CMD].asString();
  jresp[MSG_ID] = jreq[MSG_ID].asInt();

  bool reply = false;
  if (0 == strncmp(dmp->method, MSG_CATCH_EVENT, MAX_METHOD_SIZE)) {
    
    int n1=0, n2=0;
    if (jreq[MSG_BODY][ALG_POSITIVE_NUMBER].isInt()) {
      n1 = jreq[MSG_BODY][ALG_POSITIVE_NUMBER].asInt();
    }
    if (jreq[MSG_BODY][ALG_NEGATIVE_NUMBER].isInt()) {
      n2 = jreq[MSG_BODY][ALG_NEGATIVE_NUMBER].asInt();
    }
    char osd[32] = {0};
    snprintf(osd, 31, "POSI:%d NEGA:%d", n1, n2);

    video_catch_.SetOsdChn2(osd);
  } else if (0 == strncmp(dmp->method, MSG_IRCUT_CTRLS, MAX_METHOD_SIZE)) {
    reply = true;

    // GPIO8_0
    if (jreq[MSG_BODY]["switch"].asInt() > 0) {
      HI_MPI_SYS_SetReg(0x201c0004, 1);
      HI_MPI_SYS_SetReg(0x20140020, 0x8);

      VENC_COLOR2GREY_S vcs;
      vcs.bColor2Grey = HI_TRUE;
      HI_MPI_VENC_SetColor2Grey(0, &vcs);
      HI_MPI_VENC_SetColor2Grey(1, &vcs);
    } else {
      HI_MPI_SYS_SetReg(0x201c0004, 0);
      HI_MPI_SYS_SetReg(0x20140020, 0);

      VENC_COLOR2GREY_S vcs;
      vcs.bColor2Grey = HI_FALSE;
      HI_MPI_VENC_SetColor2Grey(0, &vcs);
      HI_MPI_VENC_SetColor2Grey(1, &vcs);
    }
    jresp[MSG_STATE] = RET_SUCCESS;
  } else if (0 == strncmp(dmp->method, MSG_GET_I_FRAME, MAX_METHOD_SIZE)) {
    HI_MPI_VENC_RequestIDR(jreq[MSG_BODY]["venc_chn"].asInt(), HI_TRUE);
  } else if (0 == strncmp(dmp->method, MSG_GET_ENC_CFG, MAX_METHOD_SIZE)) {
    // TODO 获取编码参数
  } else if (0 == strncmp(dmp->method, MSG_SET_ENC_CFG, MAX_METHOD_SIZE)) {
    // TODO 设置编码参数
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
}  // namespace imx6q
