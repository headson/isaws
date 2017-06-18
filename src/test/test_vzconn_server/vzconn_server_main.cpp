#include "stdafx.h"

#include "vzconn/base/pkghead.h"
#include "vzconn/async/cevtipcclient.h"
#include "vzconn/async/cevtipcserver.h"

class CServerProcess : public CTcpServerInterface {
 public:
  virtual int32 HandleNewConnection(void *p_srv, VSocket *new_sock) {
    printf("-------------------- %s[%d].\n", __FUNCTION__, __LINE__);
    return 0;
  }

  virtual void  HandleClose(void *p_srv) {
    printf("-------------------- %s[%d].\n", __FUNCTION__, __LINE__);
  }
};

class CClientProcess : public CClientInterface {
 public:
  virtual int32 HandleRecvPacket(void       *p_cli,
                                 const void *p_data,
                                 uint32      n_data) {
    NetHead* p_head = (NetHead*)p_data;
    ((char*)p_data)[n_data] = '\0';
    printf("-------------------- %s[%d] %d %s.\n",
           __FUNCTION__, __LINE__, p_head->type_flag, p_head->data);
    ((CEvtIpcClient*)p_cli)->SendPacket(p_head->data,
                                        n_data-sizeof(NetHead),
                                        &p_head->type_flag);
    return 0;
  }
  virtual int32 HandleSendPacket(void *p_cli) {
    printf("-------------------- %s[%d].\n", __FUNCTION__, __LINE__);
    return 0;
  }
  virtual void  HandleClose(void *p_cli) {
    printf("-------------------- %s[%d].\n", __FUNCTION__, __LINE__);
  }
};

int main(int argc, char* argv[]) {
  //InitSetLogging(argc, argv);
  InitVzLogging(argc, argv);

  EVT_LOOP c_loop;
  c_loop.Start();

  CServerProcess c_srv_proc;
  CClientProcess c_cli_proc;

  CInetAddr c_addr("0.0.0.0", 12345);
  CEvtIpcServer *srv_ptr = CEvtIpcServer::Create(&c_loop,
                           &c_cli_proc,
                           &c_srv_proc);
  srv_ptr->Open(&c_addr, true, true);

  int32 n_ret = 0;
  while (1) {
    n_ret = c_loop.RunLoop();
  }

  return 0;
}
