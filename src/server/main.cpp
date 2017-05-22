//
// Created by SoberPeng on 2017/5/16 0016.
//

#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include "cshmvideo.h"

int main(int argc, char* argv[]) {
  int32_t ret = 0;

  isaw::CShmVideo shm_vdo;
  ret = shm_vdo.Open("video_0", 720*576*3/2+sizeof(uint32_t));

  while(true) {
    printf("[%d]%s\n", shm_vdo.GetSize(), shm_vdo.GetData());

    usleep(100*1000);
  }
  return 0;
}

