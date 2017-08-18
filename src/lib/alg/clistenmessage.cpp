/************************************************************************/
/* Author      : SoberPeng 2017-06-24
/* Description :
/************************************************************************/
#include "clistenmessage.h"
#include "vzbase/helper/stdafx.h"
#include "vzbase/base/vmessage.h"

#include "json/json.h"

#ifdef _LINUX
#include <sys/ioctl.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#endif

namespace alg {

#define IVA_CFG_FILE    "./config/cfg_file_iva.xml"
#define IVA_AUX_FILE    "./config/cfg_file_aux.xml"

static const char  *METHOD_SET[] = {
  MSG_GET_IVAINFO,
  MSG_SET_IVAINFO,
  MSG_RESET_PCNUM
};
static const unsigned int METHOD_SET_SIZE = sizeof(METHOD_SET) / sizeof(char*);

CListenMessage::CListenMessage()
  : dp_cli_(NULL)
  , main_thread_(NULL)
  , alg_handle_(NULL)
  , image_buffer_(NULL)
  , last_read_sec_(0)
  , last_read_usec_(0) {
}

CListenMessage::~CListenMessage() {
  Stop();
}

CListenMessage *CListenMessage::Instance() {
  VZBASE_DEFINE_STATIC_LOCAL(CListenMessage, listen_message, ());
  return &listen_message;
}

bool CListenMessage::Start() {
  bool bret = false;

  /*内存共享*/
  bret = shm_alg_image_.Open(SHM_IMAGE_0, SHM_IMAGE_0_SIZE);
  if (false == bret) {
    LOG(L_ERROR) << "open share failed.";
    return false;
  }

  image_buffer_ = new char[SHM_IMAGE_0_SIZE];
  if (NULL == image_buffer_) {
    LOG(L_ERROR) << "new image buffer failed.";
    return false;
  }

  bret = shm_dbg_image_.Create(SHM_IMAGE_1, SHM_IMAGE_1_SIZE);
  if (false == bret) {
    LOG(L_ERROR) << "open share failed.";
    return false;
  }
  shm_dbg_image_.SetWidth(SHM_IMAGE_1_W);
  shm_dbg_image_.SetHeight(SHM_IMAGE_1_H);

  /*算法创建*/
  sdk_iva_create_param param;
  param.debug_callback_fun  = AlgDebugCallback;
  param.output_callback_fun = AlgActionCallback;
  param.image_width         = SHM_IMAGE_0_W;
  param.image_height        = SHM_IMAGE_0_H;

  LOG_INFO("iva set image size %d, %d.",
           param.image_width, param.image_height);

  int nret = sdk_iva_create(&alg_handle_, &param);
  if (nret != IVA_ERROR_NO_ERROR) {
    LOG_ERROR("create iva failed, %d.", nret);
    return -1;
  }

  /* 消息poll */
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

  main_thread_->PostDelayed(POLL_TIMEOUT, this, CATCH_IMAGE);
  return true;
}

void CListenMessage::Stop() {
  if (main_thread_) {
    main_thread_->Release();
    main_thread_ = NULL;
  }

  if (alg_handle_) {
    sdk_iva_destroy(alg_handle_);
    alg_handle_ = NULL;
  }

  if (dp_cli_) {
    DpClient_ReleasePollHandle(dp_cli_);
    dp_cli_ = NULL;
  }
  DpClient_Stop();

  if (image_buffer_) {
    delete[] image_buffer_;
    image_buffer_ = NULL;
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
  int nret = IVA_ERROR_NULL_HANDLE;

  Json::Value jresp;
  jresp[MSG_CMD]  = dmp->method;
  //jresp[MSG_BODY] = "";

  if (strncmp(dmp->method, MSG_GET_IVAINFO, MAX_METHOD_SIZE) == 0) {
    // 获取算法参数
    char sver[64] = {0};
    if (IVA_ERROR_NO_ERROR == sdk_iva_get_version(sver)) {
      nret = RET_SUCCESS;

      std::string ss = sver;
      jresp[MSG_BODY]["version"] = ss;
    }
  } else if (strncmp(dmp->method, MSG_SET_IVAINFO, MAX_METHOD_SIZE) == 0) {
    // 配置算法参数

  } else if (strncmp(dmp->method, MSG_RESET_PCNUM, MAX_METHOD_SIZE) == 0) {
    // 重置统计数
    if (alg_handle_) {
      nret = sdk_iva_set_control_command(alg_handle_,
                                         CONTROL_COMMAND_RESET_COUNTER);
    }
  }

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

static unsigned char chn_cmd_byte(int ch) {
  unsigned char cmd = (((ch >> 1) | (ch & 0x01) << 2) << 4);
  cmd |= 0x8c;
  return cmd;
}

static int chn_value(int fd, unsigned char nreg) {
#if 0
  if (write(fd, &nreg, 1) != 1) {
    LOG_ERROR("write i2c reg %d failed.\n", nreg);
    return -1;
  }

  unsigned char aval[2] = { 0 };
  if (read(fd, aval, 2) != 2) {
    LOG_ERROR("read i2c reg %d failed.\n", nreg);
    return -1;
  }
#else
#ifndef _WIN32
  ioctl(fd, 0x709, 0);
  ioctl(fd, 0x70A, 1);
#endif // !_WIN32

  unsigned char aval[2] = { 0xff,0xff };

  aval[0] = nreg & 0xff;
  if (read(fd, aval, 2) != 2) {
    LOG_ERROR("read i2c reg %d failed.\n", nreg);
    return -1;
  }
#endif
  int nval = (aval[1] << 8) + aval[0];
  return nval;
}

void CListenMessage::OnMessage(vzbase::Message* p_msg) {
  if (p_msg->message_id == CATCH_IMAGE) {
    main_thread_->PostDelayed(POLL_TIMEOUT, this, CATCH_IMAGE);

    int nlen = shm_alg_image_.Read(image_buffer_, SHM_IMAGE_0_SIZE,
                                 &last_read_sec_, &last_read_usec_);
    if (nlen <= 0) {
      return;
    }
    //LOG_INFO("get one frame image. length %d, %u %u.",
    //         nlen,
    //         last_read_sec_, last_read_usec_);

    iva_frame_t frame;

    // 读红外测距
#ifdef _WIN32
    for (int i = 0; i < 3; i++) {
      frame.param[i] = rand() % 100;
    }
#else
    static int fd = 0;
    if (fd == 0) {
      fd = open("/dev/i2c-2", O_RDWR);
      if(fd < 0) {
        printf("Open /dev/i2c-2 error!\n");
        return;
      }

      int ret = ioctl(fd, I2C_SLAVE_FORCE, 0x90);
      if (ret < 0) {
        printf("CMD_SET_DEV error!\n");
        return;
      }
    }

    if (fd > 0) {
      for (int i = 0; i < 5; i++) {
        frame.param[i] = chn_value(fd, chn_cmd_byte(i));
      }
    }
    LOG_WARNING("ir value: %d %d %d %d %d.",
                frame.param[0], frame.param[1], frame.param[2],
                frame.param[3], frame.param[4]);
#endif
    frame.data_size_in_bytes = nlen;
    frame.data = (unsigned char*)image_buffer_;
    if (alg_handle_) {
      sdk_iva_process(alg_handle_, &frame);
    }  // if (alg_handle_)
  }
}

}  // namespace alg
