#include "vzbase/helper/stdafx.h"

#include "vzconn/base/connhead.h"
#include "vzconn/async/cevtipcclient.h"
#include "vzconn/async/cevtipcserver.h"
using namespace vzconn;

class CServerProcess : public vzconn::CTcpServerInterface {
 public:
  virtual bool HandleNewConnection(vzconn::VSocket *p_srv,
                                   vzconn::VSocket *new_sock) {
    printf("-------------------- %s[%d].\n", __FUNCTION__, __LINE__);
    return true;
  }

  virtual void HandleServerClose(vzconn::VSocket *p_srv) {
    printf("-------------------- %s[%d].\n", __FUNCTION__, __LINE__);
  }
};

class CClientProcess : public vzconn::CClientInterface {
 public:
  virtual int32 HandleRecvPacket(vzconn::VSocket *p_cli,
                                 const char      *p_data,
                                 unsigned int     n_data,
                                 unsigned short   n_flag) {
    NetHead* p_head = (NetHead*)p_data;
    ((char*)p_data)[n_data] = '\0';
    //printf("-------------------- %s[%d] %d %s.\n",
    //       __FUNCTION__, __LINE__, p_head->type_flag, ((char*)p_data + 8));

    printf("-------------------- %s[%d] %d %d.\n",
           __FUNCTION__, __LINE__, n_data, (uint32)p_head->type_flag);

    p_cli->AsyncWrite(p_data,
                      n_data,
                      vzconn::NetworkToHost16(p_head->type_flag));
    return 0;
  }
  virtual int32 HandleSendPacket(vzconn::VSocket *p_cli) {
    //printf("-------------------- %s[%d].\n", __FUNCTION__, __LINE__);
    return 0;
  }
  virtual void  HandleClose(vzconn::VSocket *p_cli) {
    printf("-------------------- %s[%d].\n", __FUNCTION__, __LINE__);
  }
};

int main(int argc, char* argv[]) {
  //InitSetLogging(argc, argv);
  InitVzLogging(argc, argv);

  ShowVzLoggingAlways();

  vzconn::EVT_LOOP c_loop;
  c_loop.Start();

  CServerProcess c_srv_proc;
  CClientProcess c_cli_proc;

  vzconn::CInetAddr c_addr("0.0.0.0", 12345);
  vzconn::CEvtTcpServer *srv_ptr = vzconn::CEvtTcpServer::Create(&c_loop,
                                   &c_cli_proc,
                                   &c_srv_proc);
  srv_ptr->Open(&c_addr, true, true);

  int32 n_ret = 0;
  while (1) {
    n_ret = c_loop.RunLoop();
  }

  return 0;
}
