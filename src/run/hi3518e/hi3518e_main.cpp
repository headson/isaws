#include <stdio.h>
#include <unistd.h>

#include "vzbase/helper/stdafx.h"

#include "platform/hi3518e/cvideocatch.h"
#include "platform/hi3518e/caudiocatch.h"

int main(int argc, char* argv[]) {
  InitVzLogging(argc, argv);

  CVideoCatch c_vdo_1;
  //CAudioCatch c_ado_0;

  c_vdo_1.Start();
  //c_ado_0.Start();
  while (true) {
    usleep(100*1000);
  }
  return 0;
}