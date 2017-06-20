#include "stdafx.h"

#include "vzconn/async/clibevent.h"
#include "vzconn/base/cblockbuffer.h"

vzconn::EVT_LOOP c_evt_base;

int32 FuncTimeout(SOCKET fd, short events, const void *p_usr_arg) {
  static uint32 i = 0;
  printf("hello worlds %d.\n", ++i);
  if (i == 2) {

  }
  return 0;
}

int main(int argc, char* argv[]) {
  InitVzLogging(argc, argv);
#if 0
  vzconn::CBlockBuffer c_blk;

  char s_data[] = "hello world";
  for (int32 i = 0; i < 10000; i++) {
    if (c_blk.FreeSize() >= strlen(s_data)) {
      memcpy(c_blk.GetWritePtr(), s_data, strlen(s_data));
      c_blk.MoveWritePtr(strlen(s_data));
      printf("write size %d, block used size %d, free size %d.\n",
             strlen(s_data), c_blk.UsedSize(), c_blk.FreeSize());
    }
  }

  for (int32 i = 0; i < 10000; i++) {
    //if (c_blk.UsedSize() >= 8) 
    {
      c_blk.MoveReadPtr(7);
      printf("read size 7, block used size %d, free size %d.\n", 
             c_blk.UsedSize(), c_blk.FreeSize());
    }
    if (c_blk.UsedSize() == 0) {
      continue;
    } 
  }
#endif

  int32 n_ret = 0;

  n_ret = c_evt_base.Start();
  if(n_ret < 0) {
    return n_ret;
  }

  vzconn::EVT_TIMER c_timer;
  c_timer.Init(&c_evt_base, FuncTimeout, NULL);
  c_timer.Start(1000, 1000);

  struct timeval tv;
  tv.tv_sec  = 5;
  tv.tv_usec = 0;
  c_evt_base.LoopExit(&tv);

  while (1) {
    int32 n_ret = c_evt_base.RunLoop(EVLOOP_NO_EXIT_ON_EMPTY);
    if (n_ret != 0) {
      printf("n_ret %d.\n", n_ret);
    }
    usleep(1000);
  }

  return 0;
}
