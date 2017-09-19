/************************************************************************/
/* Author      : SoberPeng 2017-06-24
/* Description :
/************************************************************************/
#include "cpcalgctrl.h"
#include "vzbase/helper/stdafx.h"
#include "vzbase/base/vmessage.h"

#include "json/json.h"

#ifdef _LINUX
#include <sys/ioctl.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#endif
#include <fstream>

static const char  *METHOD_SET[] = {
  MSG_REMOTE_5_IR,
  MSG_GET_ALGINFO,
  MSG_SET_ALGINFO,
  MSG_RESET_PCNUM
};
static const unsigned int METHOD_SET_SIZE = sizeof(METHOD_SET) / sizeof(char*);

CAlgCtrl::CAlgCtrl(vzbase::Thread *fast_thread)
  : fast_thread_(fast_thread)
  , remote_dp_client_(NULL)
  , alg_handle_(NULL) {
  memset(&ir_local_, 0, sizeof(ir_local_));
  memset(&ir_remote_, 0, sizeof(ir_remote_));
}

CAlgCtrl::~CAlgCtrl() {
  Stop();
}

bool CAlgCtrl::Start() {
  bool bret = false;

  /*算法创建*/
  sdk_iva_create_param param;
  memset(&param, 0, sizeof(param));
  param.debug_callback_fun  = AlgDebugCallback;
  param.output_callback_fun = AlgActionCallback;
  param.image_width         = SHM_VIDEO_2_W;
  param.image_height        = SHM_VIDEO_2_H;
  snprintf(param.config_filename, 127, DEF_ALG_CONFIG_FILE);

  LOG_INFO("iva set image size %d, %d, %s.",
           param.image_width, param.image_height, param.config_filename);

  int nret = sdk_iva_create(&alg_handle_, &param);
  if (nret != IVA_ERROR_NO_ERROR) {
    LOG_ERROR("create iva failed, %d.", nret);
    return -1;
  }

  // 连接远端星型机构
  Json::Reader jread;
  Json::Value  jinfo;
  std::ifstream ifs;
  ifs.open(UPLINK_ADDR_CONFIG);
  if (ifs.is_open() &&
      jread.parse(ifs, jinfo)) {
    if (!jinfo["ip"].isNull()
        && !jinfo["port"].isNull()) {
      int nport = jinfo["port"].asInt();
      std::string sip = jinfo["ip"].asString();
      if (nport > 1024 &&
          vzconn::CInetAddr::IsIPV4(sip.c_str())) {
        remote_dp_client_ = CDpClient::Create(
                              sip.c_str(), nport,
                              fast_thread_->socketserver()->GetEvtService());
      }
      LOG(L_INFO) << "remote address "<<sip
                  << " port "<<nport;
    }
  }
  return true;
}

void CAlgCtrl::Stop() {
  if (alg_handle_) {
    sdk_iva_destroy(alg_handle_);
    alg_handle_ = NULL;
  }
  if (remote_dp_client_) {
    delete remote_dp_client_;
    remote_dp_client_ = NULL;
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

bool CAlgCtrl::OnImage(void *pimg, unsigned int nimg) {
  iva_frame_t frame;
  // 读红外测距
#ifdef _WIN32
  for (int i = 0; i < MAX_IR_NUM; i++) {
    frame.param[i] = rand() % 100;
  }
  if (remote_dp_client_) {
    remote_dp_client_->SendDpMessage(MSG_REMOTE_5_IR, 0,
                                     (char*)&ir_local_, sizeof(ir_local_));
  }
#else
  static int fd = 0;
  if (fd == 0) {
    fd = open("/dev/i2c-2", O_RDWR);
    if(fd < 0) {
      printf("Open /dev/i2c-2 error!\n");
      return false;
    }

    int ret = ioctl(fd, I2C_SLAVE_FORCE, 0x90);
    if (ret < 0) {
      printf("CMD_SET_DEV error!\n");
      return false;
    }
  }
  {
    vzbase::CritScope cs(&ir_mutex_);
    if (fd > 0) {
      for (int i = 0; i < MAX_IR_NUM; i++) {
        ir_local_.ir[i] = frame.param[i] = chn_value(fd, chn_cmd_byte(i));

        frame.param[MAX_IR_NUM + i] = ir_remote_.ir[i];
      }
    }
    //LOG_WARNING("ir value: %d %d %d %d %d, remote ir value: %d %d %d %d %d",
    //            frame.param[0], frame.param[1], frame.param[2],
    //            frame.param[3], frame.param[4],
    //            frame.param[MAX_IR_NUM+0], frame.param[MAX_IR_NUM+1], frame.param[MAX_IR_NUM+2],
    //            frame.param[MAX_IR_NUM+3], frame.param[MAX_IR_NUM+4]);

    if (remote_dp_client_) {
      remote_dp_client_->SendDpMessage(MSG_REMOTE_5_IR, 0,
                                       (char*)&ir_local_, sizeof(ir_local_));
      memset(&ir_remote_, 0, sizeof(ir_remote_));
    }
  }

#endif
  frame.data_size_in_bytes = nimg;
  frame.data = (unsigned char*)pimg;
  if (alg_handle_) {
    int res = sdk_iva_process(alg_handle_, &frame);
    if (IVA_ERROR_NO_ERROR == res) {
      return true;
    }
  }  // if (alg_handle_)
  return false;
}
