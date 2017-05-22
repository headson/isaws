#include "cshmimage.h"

namespace base {

  CShmImage::CShmImage(void) {
  }

  CShmImage::~CShmImage(void) {
    Close();
  }

  int32_t CShmImage::Open(const ShmKey* s_key, ShmSize n_size)
  {
    return v_shm_.Open(s_key, n_size);
  }

  void CShmImage::Close()
  {
    v_shm_.Close();
  }

  int32_t CShmImage::Share(const void* p_data, uint32_t n_data)
  {
    if (!v_shm_.IsValid()) {
      return -1;
    }
    if (n_data > v_shm_.GetSize()) {
      return -2;
    }

    memcpy(v_shm_.GetData(), p_data, n_data);
    return 0;
  }

  const void* CShmImage::GetData()
  {
    if (v_shm_.IsValid())
    {
      return NULL;
    }
    return ((TAG_SHM_IMAGE*)v_shm_.GetData())->a_data;
  }

  uint32_t CShmImage::GetSize()
  {
    if (!v_shm_.IsValid()) {
      return 0;
    }
    return ((TAG_SHM_IMAGE*)v_shm_.GetData())->n_size;
  }

  uint32_t CShmImage::GetWidth()
  {
    if (!v_shm_.IsValid()) {
      return 0;
    }
    return ((TAG_SHM_IMAGE*)v_shm_.GetData())->n_width;
  }

  uint32_t CShmImage::GetHeight()
  {
    if (!v_shm_.IsValid()) {
      return 0;
    }
    return ((TAG_SHM_IMAGE*)v_shm_.GetData())->n_height;
  }

}  // namespace base
