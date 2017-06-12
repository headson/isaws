#include <stdio.h>

#include "platform/imx6q/cplatform.h"

int main(int argc, char* argv[]) {
  CPlatform::Initinal();

  CVideoEncode c_vdo_enc;
  c_vdo_enc.SetVideo("/dev/video0");
  c_vdo_enc.SetInput(0);
  c_vdo_enc.SetViSize(720, 576);

  int32_t n_ret = c_vdo_enc.Start();

  c_vdo_enc.Process();

  while (true) {
    usleep(100*1000);
  }

  CPlatform::Release();
  return 0;
}