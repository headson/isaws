#include "vzbase/helper/stdafx.h"

#include "vzconn/base/basedefines.h"
#include "vzconn/sync/ctcpclient.h"
//#include "vzconn/async/cevtipcclient.h"

static int i = 0;
class CSocketProcess : public vzconn::CClientInterface {
 public:
   virtual int32 HandleRecvPacket(vzconn::VSocket  *p_cli,
                                  const uint8      *p_data,
                                  uint32            n_data,
                                  uint16            n_flag) {
    vzconn::NetHead* p_head = (vzconn::NetHead*)p_data;
    printf("data length [%d] flag %d\n", n_data, (uint32)p_head->type_flag);
    return 0;
  }

  virtual int32 HandleSendPacket(vzconn::VSocket *p_cli) {
    //printf("-------------------- %s[%d].\n", __FUNCTION__, __LINE__);
    return 0;
  }

  virtual void HandleClose(vzconn::VSocket *p_cli) {
    printf("-------------------- %s[%d].\n", __FUNCTION__, __LINE__);
  }
};

int32 TimerEvent(SOCKET fd, short events, const void* p_usr_arg) {
  vzconn::CTcpClient* p_cli = (vzconn::CTcpClient*)p_usr_arg;

  static char *p_data = new char[1024*1024];
  p_cli->AsyncWrite(p_data, rand()%1024*1024, 1);
  return 0;
}

int main(int argc, char* argv[]) {
  //InitSetLogging(argc, argv);
  InitVzLogging(argc, argv);
  ShowVzLoggingAlways();
#if 0
  vzconn::EVT_LOOP c_loop;
  vzconn::EVT_TIMER c_timer;

  c_loop.Start();

  int32 n_ret = 0;
  CSocketProcess c_cli_proc;

  vzconn::CInetAddr c_addr("192.168.6.8", 12345);
  c_tcp = vzconn::CEvtIpcClient::Create(&c_loop, &c_cli_proc);

  c_tcp->Connect(&c_addr, false, true, 5000);

  p_data = new char[1024*1024];

  //c_tcp->CloseSocket();
  c_timer.Init(&c_loop, TimerEvent, c_tcp);
  c_timer.Start(1000, 100);

  while (1) {
    n_ret = c_loop.RunLoop();
    if (i >= 4) {
      break;
    }
  }
#else
  CSocketProcess c_cli_proc;

  vzconn::EVT_LOOP c_loop;
  vzconn::EVT_TIMER c_timer;

  c_loop.Start();

  vzconn::CTcpClient * c_sync = vzconn::CTcpClient::Create(&c_loop, &c_cli_proc);
  if (c_sync)
  {
    vzconn::CInetAddr c_addr("127.0.0.1", 12345);
    bool b_ret = c_sync->Connect(&c_addr, false, true);

    c_timer.Init(&c_loop, TimerEvent, c_sync);
    c_timer.Start(1000, 100);

    
    while (b_ret) {
      c_loop.RunLoop(100);
    }
  }

  getchar();
  getchar();

#endif
  return 0;
}
