#include "cdevicedetect.h"

#include "vzbase/helper/stdafx.h"
#include "vzbase/base/vmessage.h"

#include "json/json.h"

CDeviceDetect::CDeviceDetect(QObject *parent)
  : QThread(parent)
  , running_(false)
  , mcast_sock_(NULL) {
  devs_info_.clear();
}

CDeviceDetect::~CDeviceDetect() {

}

CDeviceDetect * CDeviceDetect::Instance() {
  VZBASE_DEFINE_STATIC_LOCAL(CDeviceDetect, device_detect, ());
  return &device_detect;
}

bool CDeviceDetect::Start() {
  running_ = true;

  start(QThread::IdlePriority);
  return true;
}

bool CDeviceDetect::Detect() {
  if (mcast_sock_) {
    Json::Value jroot;
    jroot[MSG_CMD]  = MSG_GET_DEVINFO;
    jroot[MSG_ID]   = 1234567;
    jroot[MSG_BODY] = "";

    std::string jdata = jroot.toStyledString();
    mcast_sock_->SendUdpData((uint8*)DEF_MCAST_IP,
                             DEF_MCAST_DEV_PORT,
                             (const uint8*)jdata.c_str(),
                             jdata.size());

    return true;
  }
  return false;
}

void CDeviceDetect::run() {
  vzconn::EVT_LOOP evt_loop;
  evt_loop.Start();

  mcast_sock_ = vzconn::CMCastSocket::Create(
                  &evt_loop, this);
  mcast_sock_->Open((const uint8*)DEF_MCAST_IP,
                    DEF_MCAST_CLI_PORT);

  while (running_) {
    evt_loop.RunLoop(5 * 1000);

    Detect();
  }

  evt_loop.Stop();
}

int32 CDeviceDetect::HandleRecvPacket(vzconn::VSocket *p_cli,
                                      const uint8 *p_data,
                                      uint32 n_data,
                                      uint16 n_flag) {
  std::string json_((char*)p_data, n_data);
  LOG(L_INFO) << json_.c_str();

  Json::Value  jroot;
  Json::Reader jreader;
  if (!jreader.parse(json_, jroot)) {
    LOG(L_ERROR) << jreader.getFormatedErrorMessages();
    return -1;
  }

  std::string cmd_ = jroot[MSG_CMD].asString();
  if (cmd_.compare(MSG_GET_DEVINFO)) {     // 获取设备信息
    OnGetDevInfo(jroot);
  } else if (cmd_.compare(MSG_SET_DEVINFO)) {

  }
  return 0;
}

void CDeviceDetect::OnGetDevInfo(Json::Value &jroot) {


  emit UpdateDevice();
}

