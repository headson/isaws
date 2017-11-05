/************************************************************************
* Author      : SoberPeng 2017-10-28
* Description :
************************************************************************/
#include "cclientaccess.h"

#include "vzbase/helper/stdafx.h"

#include "client_hs/base/cmarkup.h"
#include "client_hs/base/basedefine.h"

CClientAccess::CClientAccess(vzbase::Thread *pthread)
  : thread_(pthread)
  , server_host_("www.baiweixun.com.cn")
  , server_port_(6001)
  , connect_state_(DISCONNECT)
  , server_connect_(NULL) {
}

CClientAccess::~CClientAccess() {
  if (thread_) {
    thread_->Clear(this);
    thread_ = NULL;
  }
}

bool CClientAccess::Start(const std::string &sAesHost,
                          unsigned int nAesPort) {
  if (server_host_ != sAesHost ||
      server_port_ != nAesPort) {
    LOG(L_ERROR) << "user set address is not default: "
                 << sAesHost << "[" << nAesPort << "].";
    if (server_connect_) {
      server_connect_->Remove();
      server_connect_ = NULL;
    }
  }

  server_host_ = sAesHost;
  server_port_ = nAesPort;

  OnMessage(NULL);
  return true;
}

uint32 CClientAccess::NetHeadSize() {
  return sizeof(TAG_PkgHead);
}

int32 CClientAccess::NetHeadParse(const uint8 *pdata, uint32 ndata, uint16 *head_size) {
  if (ndata < sizeof(TAG_PkgHead)) {
    return 0;
  }

  int32 pkg_size = 0;
  TAG_PkgHead *pHead = (TAG_PkgHead*)pdata;
  if (pHead->nMark != HEAD_MARK) {
    LOG(L_ERROR) << "packet head is error.";
    return -1;
  }

  if (PKG_ENC_STREAM == pHead->eCmdMain ||
      PKG_TALK_STREAM == pHead->eCmdMain) {
    if (head_size) {
      *head_size = HEAD_LEN_STR;
    }
    pkg_size = pHead->nPkgLen + HEAD_LEN_STR;
  } else if (0 == (pHead->eCmdMain % 2)) {
    if (head_size) {
      *head_size = HEAD_LEN_REQ;
    }
    pkg_size = pHead->nPkgLen + HEAD_LEN_REQ;
  } else {
    if (head_size) {
      *head_size = HEAD_LEN_RET;
    }
    pkg_size = pHead->nPkgLen + HEAD_LEN_RET;
  }
  return pkg_size;
}

int32 CClientAccess::NetHeadPacket(uint8 *pdata, uint32 ndata,
                                   uint32 nbody, uint16 nflag) {
  return 0;
}

int32 CClientAccess::HandleConnected(vzconn::VSocket *cli) {
  LOG(L_INFO) << "connected.";

  // 发送注册消息
  CMarkupSTL cXml;
  cXml.SetDoc(XML_HEAD);
  cXml.AddElem(XML_ROOT);
  cXml.AddAttrib("ID", "DS011505192021");
  cXml.AddAttrib("Times", 1);
  cXml.AddAttrib("DeviceLinkType", 1);
  cXml.AddAttrib("DeviceMaxConnect", 6);
  cXml.IntoElem();

  cXml.AddElem("DevType", 1310);
  cXml.AddElem("SrvIP", "192.168.1.11");
  cXml.AddElem("WanIp", "192.168.1.101");
  cXml.AddElem("LanIp", "192.168.1.101");
  cXml.AddElem("SVersion", 10000);
  cXml.AddElem("Hardware", 10000);
  cXml.AddElem("Timezone", 8);
  cXml.AddElem("MsgPort", 80);
  cXml.AddElem("VideoPort", 80);
  cXml.AddElem("AudioPort", 80);
  cXml.AddElem("UpdatePort", 80);
  cXml.AddElem("WebPort", 0);
  cXml.AddElem("Protocol", "DS");
  cXml.AddElem("Company", "thinkwatch");
  cXml.AddElem("IsNAT", "true");
  cXml.AddElem("MainStream", 2);

  std::string sXml = cXml.GetDoc();
  if (server_connect_) {
    server_connect_->AsyncWriteReq(CMD_TERM_LOGIN, 0,
                                   sXml.c_str(), sXml.size(),
                                   NULL, NULL);
    UpdateState(CONNECT);
  }
  return 0;
}

int32 CClientAccess::HandleRecvPacket(vzconn::VSocket *cli,
                                      const uint8 *pdata, uint32 ndata,
                                      uint16 head_size) {
  if (NULL == cli || NULL == pdata || ndata <= 0) {
  }
  std::string sXml;
  TAG_PkgHead *pHead = (TAG_PkgHead*)pdata;
  sXml.append((char*)pdata+head_size, ndata-head_size);
  LOG(L_INFO) << sXml;

  switch (pHead->eCmdMain) {
  case CMD_TERM_LOGIN+1:
    if (!OnConnected(sXml)) {
      UpdateState(DISCONNECT);
    }
    break;

  case CMD_HEART_BAET+1:
    OnHeartbeated(sXml);
    break;

  case CMD_STR_TICKET:            // 建立视频通道
    OnCreateChannel(sXml);
    break;

  case CMD_STR_TRANS:             // 传输视频请求
    OnTransStream(sXml);
    break;

  default:
    break;
  }
  return 0;
}

int32 CClientAccess::HandleSendPacket(vzconn::VSocket *cli) {
  return 0;
}

void CClientAccess::HandleClose(vzconn::VSocket *cli) {
  LOG(L_INFO) << "disconnected.";
  UpdateState(DISCONNECT);
}

void CClientAccess::OnMessage(vzbase::Message *msg) {
  switch (connect_state_) {
  case DISCONNECT:
    OnDisconnect();
    break;
  case CONNECT:
    break;
  case CONNECTED:
    break;
  case HEARTBEAT:
    break;
  case HEARTBEATED:
    OnHeartbeat();
    break;
  default:
    break;
  }

  // 非断开连接状态, 判断是否状态时间更新超时
  if (DISCONNECT != connect_state_) {
    int nDiffTime =
      vzbase::CurrentTimet() - last_state_time_;
    if (abs(nDiffTime) > 120) {
      UpdateState(DISCONNECT);
    }
  }

  if (thread_) {
    thread_->PostDelayed(AES_TIMER_TIMEOUT, this);
  }
}

void CClientAccess::UpdateState(CLI_CONN_STATE state) {
  connect_state_ = state;
  last_state_time_ = vzbase::CurrentTimet();
}

bool CClientAccess::OnDisconnect() {
  vzconn::CInetAddr addr(
    server_host_.c_str(), server_port_);
  if (!addr.IsNull()) {
    vzconn::EventService *evt_srv =
      thread_->socketserver()->GetEvtService();
    server_connect_ = cli::CTcpAsyncClient::Create(evt_srv, this);
    if (server_connect_) {
      bool res = server_connect_->Connect(&addr, false, true);
      if (false == res) {
        LOG(L_ERROR) << "connect failed. " << addr.ToString();
        server_connect_->Remove();
        server_connect_ = NULL;
      }
      return res;
    }
  }
  LOG(L_ERROR) << "create server connect failed.";
  return false;
}

/*<? xml version = "1.0" encoding = "GBK" ?>
<Message
  Note = "1"
  DispatchId = "DISPATCH00000000000001"
  Id = "DS011505192021"
  Organ = "1"
  CaptainId = ""
  HeartCycle = "null"
  Name = "rrr"
  Address = "rr"
  DevType = "1310"
  CheckCode = "12345678">
  <Store
    Ip = "182.149.198.210"
    Port = "5603"
    Id = "STORE00000000000001" />
  <Result>0</Result>
</Message>*/
bool CClientAccess::OnConnected(const std::string &sXml) {
  CMarkupSTL cXml;
  if (!cXml.SetDoc(sXml.c_str())) {
    LOG(L_ERROR) << "xml parse failed.";
    return false;
  }

  if (sXml.npos != sXml.find("<Result>0</Result>")) {
    LOG(L_INFO) << "connected success.";

    // 解析出存储服务器处理之

    UpdateState(CONNECTED);
    OnHeartbeat();
    return true;
  }
  return false;
}

bool CClientAccess::OnHeartbeat() {
  CMarkupSTL cXml;
  cXml.SetDoc(XML_HEAD);
  cXml.AddElem(XML_ROOT);
  cXml.AddAttrib("ID", "DS011505192021");
  cXml.AddAttrib("Name", "rrr");
  cXml.AddAttrib("DevType", 100);
  cXml.IntoElem();

  cXml.AddElem("DevType", 100);
  cXml.AddElem("Version", 1000);
  cXml.AddElem("Hardware", 1000);
  cXml.AddElem("Expires", 120);
  cXml.AddElem("CurrentTime", vzbase::Time());

  std::string sXml = cXml.GetDoc();
  if (server_connect_) {
    server_connect_->AsyncWriteReq(CMD_HEART_BAET, 0,
                                   sXml.c_str(), sXml.size(),
                                   NULL, NULL);
    // UpdateState(HEARTBEAT);
  }
  return true;
}

/*<?xml version="1.0" encoding="GBK"?>
<Message
  CurrentDateTime="2017-11-04 14:54:08"/>*/
bool CClientAccess::OnHeartbeated(const std::string &sXml) {
  UpdateState(HEARTBEATED);
  return true;
}

bool CClientAccess::OnCreateChannel(const std::string &sXml) {

  return false;
}

bool CClientAccess::OnTransStream(const std::string &sXml) {
  return false;
}

