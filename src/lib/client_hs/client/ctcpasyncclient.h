/************************************************************************
* Author      : SoberPeng 2017-11-02
* Description :
************************************************************************/
#ifndef LIBCLIENT_HS_CTCPASYNCCLIENT_H
#define LIBCLIENT_HS_CTCPASYNCCLIENT_H

#include "vzconn/async/cevttcpclient.h"
#include "vzbase/base/boost_settings.hpp"

namespace cli {

class CTcpAsyncClient : public vzconn::CEvtTcpClient {
 protected:
  CTcpAsyncClient(const vzconn::EVT_LOOP *p_loop,
                  vzconn::CClientInterface *cli_hdl);
  virtual ~CTcpAsyncClient();

 public:
  static CTcpAsyncClient *Create(const vzconn::EVT_LOOP *p_loop,
                                 vzconn::CClientInterface *cli_hdl);

  virtual int32 AsyncWriteReq(int32 eCmd, int32 nMinor,
                              const char* pData, uint32 nData,
                              const char* pSrc = NULL, const char* pDst = NULL);

  virtual int32 AsyncWriteRet(int32 eCmd,
                              const char* pData, uint32 nData,
                              int32 nRet, int32 eMinor);

 public:
  // 链接到服务端;无需调用Open
  virtual bool  Connect(const vzconn::CInetAddr *remote_addr,
                        bool bblock, bool breuse,
                        uint32 ms_timeout = 5000);

 protected:
  static int32 EvtConnect(SOCKET       fd,
                          short        events,
                          const void  *p_usr_arg);

  int32 OnConnect(SOCKET fd);

  virtual int32 OnRecv();
};

}

#endif  // LIBCLIENT_HS_CTCPASYNCCLIENT_H
