//
// Created by SoberPeng on 2017/5/16 0016.
//

#include <stdio.h>
#include <cstring>
#include "cshmvideo.h"

namespace isaw {

CShmVideo::CShmVideo() {
}

CShmVideo::~CShmVideo() {
  vshm_.Close();
}

int32_t CShmVideo::Open(ShmKey key, ShmSize size) {
  return vshm_.Open(key, size);
}

int32_t   CShmVideo::Share(const void* data, uint32_t size)
{
  TAG_SHM_VIDEO* shm = (TAG_SHM_VIDEO*)vshm_.GetData();
  if (shm && size <= vshm_.GetSize()) {
    shm->n_size = size;
    strncpy((char*)shm->a_data, (char*)data, size);
    return size;
  }
  return 0;
}

void*     CShmVideo::GetData() {
  return (void*)((TAG_SHM_VIDEO*)vshm_.GetData())->a_data;
}

uint32_t  CShmVideo::GetSize() {
  return ((TAG_SHM_VIDEO*)vshm_.GetData())->n_size;
}

}