#include "stdafx.h"

#include "vzconn/base/connhead.h"
#include "vzconn/async/cevtipcclient.h"

static int i = 0;
char* p_data = NULL;
vzconn::CEvtIpcClient *c_tcp = NULL;
class CSocketProcess : public vzconn::CClientInterface {
 public:
   virtual int32 HandleRecvPacket(vzconn::VSocket  *p_cli,
                                  const uint8      *p_data,
                                  uint32            n_data,
                                  uint16            n_flag) {
    NetHead* p_head = (NetHead*)p_data;
    printf("data length [%d] flag %d\n", n_data, (uint32)p_head->type_flag);
    return 0;
  }

  virtual int32 HandleSendPacket(vzconn::VSocket *p_cli) {
    printf("-------------------- %s[%d].\n", __FUNCTION__, __LINE__);
    return 0;
  }

  virtual void HandleClose(vzconn::VSocket *p_cli) {
    c_tcp = NULL;
    printf("-------------------- %s[%d].\n", __FUNCTION__, __LINE__);
  }
};

int32 TimerEvent(SOCKET fd, short events, const void* p_usr_arg) {
  if (c_tcp) {
    static uint16 i = 0;
    i++;

    int32 n_rand = rand() % 1024*256;
    int32 n_send = c_tcp->AsyncWrite(p_data, n_rand, i);
    if (n_send < 0) {
      LOG(L_ERROR) << "async write failed "<<n_rand;
    }
  }
  return 0;
}

int main(int argc, char* argv[]) {
  //InitSetLogging(argc, argv);
  InitVzLogging(argc, argv);

#if 1
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
  vzconn::CSocketProcess c_cli_proc;

  vzconn::CTcpClient* c_sync = vzconn::CTcpClient::Create(&c_cli_proc);
  if (c_sync)
  {
    vzconn::CInetAddr c_addr("192.168.6.8", 12345);
    bool b_ret = c_sync->Connect(&c_addr, false, true);
    if (b_ret) {
      c_sync->Send("hello worlds.", 9, 1, 500);

      char s_data[1024] = {0};
      int32 n_recv = c_sync->Recv(s_data, 1024, 500);
      if (n_recv > 0) {
        printf("recv data length %d.\n", n_recv);
      }
    }
  }

  getchar();
  getchar();

#endif
  return 0;
}
