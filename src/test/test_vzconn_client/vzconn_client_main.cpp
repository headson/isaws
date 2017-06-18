#include "stdafx.h"

#include "vzconn/async/cevtipcclient.h"

static int i = 0;
EVT_LOOP c_loop;
EVT_TIMER c_timer;
CEvtIpcClient *c_tcp = NULL;

class CSocketProcess : public CClientInterface {
 public:
  virtual int32 HandleRecvPacket(void *p_cli, const void *p_data, uint32 n_data) {
    printf("[%d]%s\n", n_data, p_data);
    return 0;
  }

  virtual int32 HandleSendPacket(void *p_cli) {
    printf("-------------------- %s[%d].\n", __FUNCTION__, __LINE__);
    return 0;
  }

  virtual void HandleClose(void *p_cli) {
    //c_tcp = NULL;
    printf("-------------------- %s[%d].\n", __FUNCTION__, __LINE__);
  }
};

int32 TimerEvent(SOCKET fd, short events, const void* p_usr_arg) {
  if (c_tcp) {
    static uint16 i = 0;
    i++;
    c_tcp->SendPacket("hello worlds.", 9, &i);
    //c_tcp->Remove();
  }
  return 0;
}

int main(int argc, char* argv[]) {
  //InitSetLogging(argc, argv);
  InitVzLogging(argc, argv);

  c_loop.Start();

  int32 n_ret = 0;
  CSocketProcess c_cli_proc;

  CInetAddr c_addr("192.168.1.106", 12345);
  c_tcp = CEvtIpcClient::Create(&c_loop, &c_cli_proc);

  c_tcp->Connect(&c_addr, false, true, 5000);

  //c_tcp->CloseSocket();
  c_timer.Init(&c_loop, TimerEvent, c_tcp);
  c_timer.Start(1000, 1000);

  while (1) {
    n_ret = c_loop.RunLoop();
    if (i >= 4) {
      break;
    }
  }

  return 0;
}
