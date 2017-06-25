#include <stdio.h>
#include <unistd.h>

#include "stdafx.h"

#include "platform/hi3518e/caudio.h"
#include "platform/hi3518e/cvideocatch.h"

int main(int argc, char* argv[]) {
  InitVzLogging(argc, argv);

  CVideoCatch c_vdo_1;

  c_vdo_1.Start();
  while (true) {
    usleep(100*1000);
  }
  return 0;
}