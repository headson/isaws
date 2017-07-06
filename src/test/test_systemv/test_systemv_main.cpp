
#include "vzbase/helper/stdafx.h"

#include "systemv/flvmux/cflvmux.h"

int main(int argc, char *argv[]) {
  InitVzLogging(argc, argv);
#ifdef WIN32
  ShowVzLoggingAlways();
#endif

  FILE *file = fopen("test.flv", "wb+");

  CFlvMux c_flv(file);

  uint8_t s_buffer[2048];
  c_flv.InitHeadTag0(s_buffer, 2048, 320, 240);

  uint64_t n_pts = 0;
  uint8_t s_data[1024] = {0};
  while (true) {
    c_flv.AudioPacket(s_buffer, 2048, n_pts, s_data, 320);

    c_flv.AudioPacket(s_buffer, 2048, n_pts, s_data, 320);

    c_flv.VideoPacket(s_buffer, 2048, true, n_pts, s_data, 1023);
    n_pts += 20;

    usleep(20*2000);
  }

  return 0;
}

