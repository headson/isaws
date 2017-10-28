/************************************************************************
* Author      : SoberPeng 2017-10-28
* Description :
************************************************************************/
#include "cclientaccess.h"

#include "vzbase/helper/stdafx.h"

CClientAccess::CClientAccess(vzbase::Thread *pthread)
  : thread_(pthread)
  , access_host_("www.baiweixun.com.cn")
  , access_port_(6001)
  , access_connect_(NULL) {
}

CClientAccess::~CClientAccess() {
  if (thread_) {
    thread_->Clear(this);
    thread_ = NULL;
  }

  if (access_connect_) {
    delete access_connect_;
    access_connect_ = NULL;
  }
}

bool CClientAccess::Start(const std::string &sAesHost,
                          unsigned int nAesPort) {
  if (access_host_ != sAesHost ||
      access_port_ != nAesPort) {
    LOG(L_ERROR) << "user set address is not default: "
                 << sAesHost << "[" << nAesPort << "].";
    if (access_connect_) {
      delete access_connect_;
      access_connect_ = NULL;
    }
  }

  access_host_ = sAesHost;
  access_port_ = nAesPort;

  OnMessage(NULL);
  return true;
}

int32 CClientAccess::HandleConnected(vzconn::VSocket *cli) {
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

}

void CClientAccess::OnMessage(vzbase::Message *msg) {
  if (NULL == access_connect_) {
    vzconn::CInetAddr addr(
      access_host_.c_str(), access_port_);
    if (!addr.IsNull()) {
      vzconn::EventService *evt_srv = 
        thread_->socketserver()->GetEvtService();
      access_connect_ = vzconn::CTcpAsyncClient::Create(evt_srv, this);
      if (access_connect_) {
        bool res = access_connect_->Connect(&addr, false, true);
        if (false == res) {
          LOG(L_ERROR) << "connect failed. " << addr.ToString();
          delete access_connect_;
          access_connect_ = NULL;
        }
      }
    }
  }

  if (thread_) {
    thread_->PostDelayed(AES_TIMER_TIMEOUT, this);
  }
}

