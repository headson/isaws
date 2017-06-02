#include <stdio.h>
#include <unistd.h>

#include "platform/hi3516a/cvideo.h"
#include "platform/hi3516a/caudio.h"

int main(int argc, char* argv[]) {
  CVideo c_vdo_1;

  c_vdo_1.VideoCapture(0);
  while (true) {
    usleep(100*1000);
  }
  return 0;
}