#include "base/vtypes.h"
#include "base/stdafx.h"

#include "network/vevent.h"

int32_t EvtCallback(int32_t n_evt, const void* ctx) {
  LOG(L_INFO)<<time(NULL)<<" evt callback "<<n_evt;
  return 0;
}

int main(int argc, char* argv[]) {
  app_init();
  InitVzLogging(argc, argv);

  int32_t n_ret = 0;
  EVT_LOOP c_evt_loop;
  n_ret = c_evt_loop.Start();
  if (n_ret != 0) {
    LOG(L_ERROR)<<"start failed "<<n_ret;
    return -1;
  }

  EVT_TIMER evt_timer;
  evt_timer.Init(&c_evt_loop, EvtCallback, NULL);
  evt_timer.Start(100, 1000);

  while (1) {
    c_evt_loop.Runing();
    _sleep(100*1000);
  }
  return 0;
}