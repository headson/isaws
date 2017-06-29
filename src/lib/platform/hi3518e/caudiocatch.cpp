/************************************************************************
*Author      : Sober.Peng 17-06-27
*Description :
************************************************************************/
#include "caudiocatch.h"

#include "systemv/vzshm_c.h"
#include "vzbase/helper/stdafx.h"

HI_S32 HisiPutAudioDataToBuffer(HI_U8* p_data, HI_U32 n_data, void* p_usr_arg) {
  if (p_usr_arg) {
    return ((CAudioCatch*)p_usr_arg)->GetOneFrame(p_data, n_data);
  }
  return 0;
}

CAudioCatch::CAudioCatch()
  : p_shm_ado_(NULL) {

}

CAudioCatch::~CAudioCatch() {

}

int32 CAudioCatch::Start() {
  p_shm_ado_ = Shm_Create(SHM_VIDEO_0, SHM_VIDEO_0_SIZE);
  if (p_shm_ado_ == NULL) {
    LOG(L_ERROR) << "can't open share memory.";
    return -1;
  }

  return StartCaptureAudio(0, NULL);
}

HI_S32 CAudioCatch::GetOneFrame(HI_U8* p_data, HI_U32 n_data) {
  printf("get audio buffer %d.\n", n_data);

  return 0;
}
