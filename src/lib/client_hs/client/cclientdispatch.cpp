/************************************************************************
* Author      : SoberPeng 2017-11-04
* Description :
************************************************************************/
#include "cclientdispatch.h"

#include "vzbase/helper/stdafx.h"

#include "client_hs/base/cmarkup.h"
#include "client_hs/base/basedefine.h"

CClientDispatch::CClientDispatch(CClientAccess::Ptr cli, vzbase::Thread *pthread)
  : thread_(pthread)
  , access_ptr_(cli)
  , server_host_("")
  , server_port_(4604)
  , video_channel_(0)
  , server_connect_(NULL) {
}

CClientDispatch::~CClientDispatch() {
  if (thread_) {
    thread_->Clear(this);
    thread_ = NULL;
  }
}

bool CClientDispatch::Start(int nMain, int nMinor, const std::string &sXmlReq) {
  CMarkupSTL cXml;
  cXml.SetDoc(sXmlReq.c_str());
  if (!cXml.IsWellFormed()) {
    return false;
  }

  cXml.ResetMainPos();
  if (cXml.FindChildElem("DevDataID"))
    term_id_ = cXml.GetChildData();
  else
    return false;

  cXml.ResetMainPos();
  if (cXml.FindChildElem("Ticket"))
    chn_ticket_ = cXml.GetChildData();
  else
    return false;

  cXml.ResetMainPos();
  if (cXml.FindChildElem("StreamType"))
    video_channel_ = cXml.GetChildData();
  else
    return false;

  cXml.ResetMainPos();
  if (cXml.FindChildElem("ClientIP"))
    server_host_ = cXml.GetChildData();
  else
    return false;

  cXml.ResetMainPos();
  if (cXml.FindChildElem("ClientPort"))
    server_port_ = atoi(cXml.GetChildData().c_str());
  else
    return false;

  cmd_main_ = nMain;
  cmd_minor_ = nMinor;
  //////////////////////////////////////////////////////////////////////////
  vzconn::CInetAddr addr(server_host_.c_str(), server_port_);
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

uint32 CClientDispatch::NetHeadSize() {
  return sizeof(TAG_PkgHead);
}

int32 CClientDispatch::NetHeadParse(const uint8 *pdata, uint32 ndata,
                                    uint16 *head_size) {
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

int32 CClientDispatch::NetHeadPacket(uint8 *pdata, uint32 ndata,
                                     uint32 nbody, uint16 nflag) {
  return 0;
}

int32 CClientDispatch::HandleConnected(vzconn::VSocket *cli) {
  LOG(L_INFO) << "connected.";

  // ·¢ËÍ×¢²áÏûÏ¢
  CMarkupSTL  cXml;
  cXml.SetDoc(XML_HEAD);
  cXml.AddElem(XML_ROOT);
  cXml.IntoElem();

  cXml.AddElem("Ticket", chn_ticket_.c_str());
  cXml.AddElem("StreamType", video_channel_.c_str());

  std::string sXml = cXml.GetDoc();
  if (server_connect_) {
    server_connect_->AsyncWriteReq(CMD_LINK_VDSRV, 0,
                                   sXml.c_str(), sXml.size(),
                                   NULL, NULL);
  }
  return 0;
}

int32 CClientDispatch::HandleRecvPacket(vzconn::VSocket *cli,
                                        const uint8 *pdata, uint32 ndata,
                                        uint16 head_size) {
  if (NULL == cli || NULL == pdata || ndata <= 0) {
  }
  std::string sXml;
  TAG_PkgHead *pHead = (TAG_PkgHead*)pdata;
  sXml.append((char*)pdata+head_size, ndata-head_size);
  LOG(L_INFO) << sXml;

  switch (pHead->eCmdMain) {
  case CMD_LINK_VDSRV+1:
    OnRespCreateChannel(cmd_minor_);
    break;

  default:
    break;
  }
  return 0;
}

int32 CClientDispatch::HandleSendPacket(vzconn::VSocket *cli) {
  return 0;
}

void CClientDispatch::HandleClose(vzconn::VSocket *cli) {
  LOG(L_INFO) << "disconnected.";
}

void CClientDispatch::OnMessage(vzbase::Message *msg) {
  if (thread_) {
    thread_->PostDelayed(DIS_TIMER_TIMEOUT, this);
  }
}

bool CClientDispatch::OnRespCreateChannel(int nMinor) {
  CMarkupSTL cXmlRet;
  cXmlRet.SetDoc(XML_HEAD);
  cXmlRet.AddElem(XML_ROOT);
  cXmlRet.IntoElem();

  cXmlRet.AddElem("Result", 0);
  cXmlRet.AddElem("Ticket", chn_ticket_.c_str());
  cXmlRet.AddElem("VideoPort", 80);
  cXmlRet.AddElem("Protocol", "own_protocol");
  cXmlRet.AddElem("CurrConnectNum", 1);
  cXmlRet.AddElem("ErrScription", 0);
  cXmlRet.AddElem("Version", 100);

  if (access_ptr_.get() && access_ptr_->GetCli()) {
    std::string sXmlRet = cXmlRet.GetDoc();
    access_ptr_->GetCli()->AsyncWriteRet(cmd_main_ + 1,
                                         sXmlRet.c_str(), sXmlRet.size(),
                                         0, nMinor);
  }
  return true;
}

