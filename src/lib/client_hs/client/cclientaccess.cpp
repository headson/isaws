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
  , server_connect_(NULL) {
}

CClientAccess::~CClientAccess() {
  if (thread_) {
    thread_->Clear(this);
    thread_ = NULL;
  }

  if (server_connect_) {
    delete server_connect_;
    server_connect_ = NULL;
  }
}

bool CClientAccess::Start(const std::string &sAesHost,
                          unsigned int nAesPort) {
  if (server_host_ != sAesHost ||
      server_port_ != nAesPort) {
    LOG(L_ERROR) << "user set address is not default: "
                 << sAesHost << "[" << nAesPort << "].";
    if (server_connect_) {
      delete server_connect_;
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

int32 CClientAccess::NetHeadParse(const uint8 *pdata, uint32 ndata, uint16 *nflag) {
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
    pkg_size = pHead->nPkgLen + HEAD_LEN_STR;
  } else if (0 == (pHead->eCmdMain % 2)) {
    pkg_size = pHead->nPkgLen + HEAD_LEN_REQ;
  } else {
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

  // ����ע����Ϣ
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
  cXml.AddElem("WebPort", 80);
  cXml.AddElem("Protocol", "DS");
  cXml.AddElem("Company", "thinkwatch");
  cXml.AddElem("IsNAT", "true");
  cXml.AddElem("MainStream", 2);

  std::string sXml = cXml.GetDoc();
  return 0;
}

int32 CClientAccess::HandleRecvPacket(vzconn::VSocket *cli,
                                      const uint8 *pdata, uint32 ndata,
                                      uint16 nflag) {
  return 0;
}

int32 CClientAccess::HandleSendPacket(vzconn::VSocket *cli) {
  return 0;
}

void CClientAccess::HandleClose(vzconn::VSocket *cli) {
  LOG(L_INFO) << "disconnected.";
}

void CClientAccess::OnMessage(vzbase::Message *msg) {
  if (NULL == server_connect_) {
    vzconn::CInetAddr addr(
      server_host_.c_str(), server_port_);
    if (!addr.IsNull()) {
      vzconn::EventService *evt_srv =
        thread_->socketserver()->GetEvtService();
      server_connect_ = vzconn::CTcpAsyncClient::Create(evt_srv, this);
      if (server_connect_) {
        bool res = server_connect_->Connect(&addr, false, true);
        if (false == res) {
          LOG(L_ERROR) << "connect failed. " << addr.ToString();
          delete server_connect_;
          server_connect_ = NULL;
        }
      }
    }
  } else {
    // ��������

  }

  if (thread_) {
    thread_->PostDelayed(AES_TIMER_TIMEOUT, this);
  }
}

