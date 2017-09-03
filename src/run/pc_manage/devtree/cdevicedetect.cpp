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
    mcast_sock_->SendUdpData(DEF_MCAST_IP,
                             DEF_MCAST_DEV_PORT,
                             jdata.c_str(), jdata.size());

    return true;
  }
  return false;
}

int CDeviceDetect::GetDevs(QVector<CDevInfo> &devs) {
  QMutexLocker ml(&mutex_);

  QMap<std::string, CDevInfo>::Iterator it;
  for (it = devs_info_.begin(); it != devs_info_.end(); ++it) {
    devs.push_back(it.value());
  }

  return devs.size();
}

bool CDeviceDetect::GetDev(CDevInfo &dev, QString id) {
  QMutexLocker ml(&mutex_);
  std::string sid = id.toLocal8Bit().data();
  if (devs_info_.contains(sid)) {
    dev = devs_info_[sid];
    return true;
  }
  return false;
}

bool CDeviceDetect::SetDev(Json::Value &jreq) {
  if (mcast_sock_) {
    std::string sjson = jreq.toStyledString();
    mcast_sock_->SendUdpData(DEF_MCAST_IP,
                             DEF_MCAST_DEV_PORT,
                             sjson.c_str(), sjson.size());
    return true;
  }
  return false;
}

void CDeviceDetect::run() {
  vzconn::EVT_LOOP evt_loop;
  evt_loop.Start();

  mcast_sock_ = vzconn::CMCastSocket::Create(
                  &evt_loop, this);
  mcast_sock_->Open(DEF_MCAST_IP, DEF_MCAST_CLI_PORT);

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
  if (0 == cmd_.compare(MSG_GET_DEVINFO)) {     // 获取设备信息
    OnGetDevInfo(jroot);
  } else if (0 == cmd_.compare(MSG_SET_DEVINFO)) {
    LOG(L_INFO) << json_.c_str();
  }
  return 0;
}

/*
{
"body" : {
"alg_version" : "",
"dev_uuid" : "adsasdas"
"dev_name" : "PC_001",
"dev_type" : 100100,
"hw_version" : "1.0.0.1001707310",
"ins_addr" : "",
"net" : {
"dhcp_en" : 0,
"dns_addr" : "192.168.1.1",
"gateway" : "192.168.1.1",
"http_port" : 80,
"ip_addr" : "192.168.1.100",
"netmask" : "255.255.255.0",
"phy_mac" : "",
"rtsp_port" : 554,
"wifi_en" : 0
},
"sw_version" : ""
},
"cmd" : "get_devinfo",
"state" : 0
}
*/
void CDeviceDetect::OnGetDevInfo(const Json::Value &jroot) {
  CDevInfo dev;
  if (jroot[MSG_BODY]["dev_uuid"].isNull()) {
    return;
  }
  dev.id_ = jroot[MSG_BODY]["dev_uuid"].asString();

  if (jroot[MSG_BODY]["dev_name"].isNull()) {
    return;
  }
  dev.name_ = jroot[MSG_BODY]["dev_name"].asString();

  if (jroot[MSG_BODY]["net"]["ip_addr"].isNull()) {
    return;
  }
  dev.ip_ = jroot[MSG_BODY]["net"]["ip_addr"].asString();

  if (jroot[MSG_BODY]["net"]["http_port"].isNull()) {
    return;
  }
  dev.port_ = jroot[MSG_BODY]["net"]["http_port"].asInt();
  dev.json_ = jroot;

  QMutexLocker ml(&mutex_);
  if (devs_info_.contains(dev.id_)
      && devs_info_[dev.id_] == dev) {
    return;
  }
  devs_info_[dev.id_] = dev;
  emit UpdateDevice();
}

