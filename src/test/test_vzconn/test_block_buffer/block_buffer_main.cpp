#include "vzbase/helper/stdafx.h"

#include "vzconn/async/clibevent.h"
#include "vzconn/base/cblockbuffer.h"

using namespace vzconn;

#if 0
vzconn::EVT_LOOP c_evt_base;
int32 FuncTimeout(SOCKET fd, short events, const void *p_usr_arg) {
  static uint32 i = 0;
  printf("hello worlds %d.\n", ++i);
  if (i == 2) {

  }
  return 0;
}

int main(int argc, char* argv[]) {
  int32 n_ret = 0;
  n_ret = c_evt_base.Start();
  if(n_ret < 0) {
    return n_ret;
  }

  vzconn::EVT_TIMER c_timer;
  c_timer.Init(&c_evt_base, FuncTimeout, NULL);
  c_timer.Start(1000, 1000);

  c_evt_base.LoopExit(5000);

  while (1) {
    int32 n_ret = c_evt_base.RunLoop(EVLOOP_NO_EXIT_ON_EMPTY);
    if (n_ret != 0) {
      printf("n_ret %d.\n", n_ret);
    }
    usleep(1000);
  }

  return 0;
}
#endif

EVT_LOOP base;
int32 time_cb(SOCKET fd, short event, const void *p_arg) {
  printf("timer wakeup\n");
  //base.LoopExit(0);
  return 0;
}

void thread_1(void* p_arg) {
  Sleep(5*1000);
  base.LoopExit(0);
}

#include <process.h>
int main(int argc, char *argv[]) {

#ifdef WIN32
  WSADATA wsaData;
  WSAStartup(MAKEWORD(2, 2), &wsaData);
#endif

  base.Start();

  //EVT_TIMER evt_timer;
  //evt_timer.Init(&base, time_cb, NULL);
  //evt_timer.Start(1000, 1000);
  _beginthread(thread_1, 0, NULL);

  base.RunLoop(-1);
  return 0;
}