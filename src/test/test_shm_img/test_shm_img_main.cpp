#include <stdio.h>

#include <iostream>

#include "vshm.h"

struct SHM_IMG {
  unsigned int    w_sec;
  unsigned int    w_usec;

  vzbase::VShm    shm_img;
  TAG_SHM_IMG    *shm_img_ptr;
} shm_img_[MAX_SHM_IMG];

bool create_share_memory() {
  bool bret = false;

  // share image
  for (int i = 0; i < MAX_SHM_IMG; i++) {
    TAG_SHM_ARG &shm_img = g_shm_img[i];

    bret = shm_img_[i].shm_img.Open(shm_img.dev_name,
                                    shm_img.shm_size);
    if (false == bret) {
      std::cout << "open share failed." << shm_img.dev_name;
      return false;
    }
    shm_img_[i].shm_img_ptr =
      (TAG_SHM_IMG*)shm_img_[i].shm_img.GetData();
    printf("%s[%d] 0x%x.\n", shm_img_[i].shm_img_ptr);
  }
  return true;
}

int main(int argc, char* argv[]) {
  create_share_memory();

  while (true) {
    for (int i = 0; i < MAX_SHM_IMG; i++) {
      if (NULL == shm_img_[i].shm_img_ptr) {
        continue;
      }

      if (shm_img_[i].w_sec == shm_img_[i].shm_img_ptr->wsec &&
          shm_img_[i].w_usec == shm_img_[i].shm_img_ptr->wusec) {
        continue;
      }
      shm_img_[i].w_sec = shm_img_[i].shm_img_ptr->wsec;
      shm_img_[i].w_usec = shm_img_[i].shm_img_ptr->wusec;

      printf("image 0x%x[%d]; width %d, height %d.\n",
             shm_img_[i].shm_img_ptr->pdata,
             shm_img_[i].shm_img_ptr->ndata,
             shm_img_[i].shm_img_ptr->width,
             shm_img_[i].shm_img_ptr->height);

      /*IVA_FRAME frame;
      frame.type = i;
      frame.data = shm_img_[i].shm_img_ptr->pdata;
      frame.datalen = shm_img_[i].shm_img_ptr->ndata;
      int res = iva_alg_read_one_frame(alg_handle_, &frame);
      if (res != IVA_NO_ERROR) {
      LOG(L_WARNING) << "iva_alg_read_one_frame " << i << " " << res;
      }*/
    }
    usleep(5*1000);
  }
  return 0;
}