#include "vzbase/base/basictypes.h"
#include "vzbase/helper/stdafx.h"

#include "vzconn/multicast/cmcastsocket.h"
using namespace vzconn;

class CClientProcess : public CClientInterface {
 public:
  virtual int32 HandleRecvPacket(VSocket      *p_cli,
                                 const uint8  *p_data,
                                 uint32        n_data,
                                 uint16        n_flag) {
    ((char*)p_data)[n_data] = '\0';
    LOG(L_INFO) << "  "<<(char*)p_data;
    return 0;
  }
  virtual int32 HandleSendPacket(VSocket *p_cli) {
    return 0;
  }
  virtual void  HandleClose(VSocket *p_cli) {
  }
};

static const uint8  DEF_CERTER_IP[] = "234.2.2.2";
static const uint16 DEF_CENTER_PORT = 12345;

int32 timer_cb(SOCKET          fd,
               short           events,
               const void      *p_usr_arg) {
  ((CMCastSocket*)p_usr_arg)->SendUdpData(DEF_CERTER_IP, DEF_CENTER_PORT,
                                          "hello wolrds\0", 13);
  return 0;
}

int main(int argc, char* argv[]) {
  InitVzLogging(argc, argv);
#ifdef WIN32
  ShowVzLoggingAlways();
#endif

  int32_t n_ret = 0;
  EVT_LOOP c_evt_loop;
  n_ret = c_evt_loop.Start();
  if (n_ret != 0) {
    LOG(L_ERROR) << "start failed " << n_ret;
    return -1;
  }

  CClientProcess c_cli_proc;
  CMCastSocket *cmcast =
    CMCastSocket::Create(&c_evt_loop, &c_cli_proc);

  n_ret = cmcast->Open(DEF_CERTER_IP, DEF_CENTER_PORT);
  if (n_ret != 0) {
    LOG(L_ERROR) << "open failed.";
    return -1;
  }

  EVT_TIMER evt_timer;
  evt_timer.Init(&c_evt_loop, timer_cb, cmcast);
  evt_timer.Start(1000, 1000);

  while (true) {
    c_evt_loop.RunLoop();
  }
  return 0;
}
