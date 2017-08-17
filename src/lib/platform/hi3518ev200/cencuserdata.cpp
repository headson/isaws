/************************************************************************
*Author      : Sober.Peng 17-08-17
*Description :
************************************************************************/
#include "cencuserdata.h"

CEncUserData::CEncUserData(VENC_CHN chn)
  : venc_chn_(chn) {
}

CEncUserData::~CEncUserData() {

}

int CEncUserData::Start(const char* skey, int shm_size,
                        unsigned int width, unsigned int height, unsigned int stride) {
  HI_U32             u32LStride;
  HI_U32             u32CStride;
  HI_U32             u32LumaSize;
  HI_U32             u32ChrmSize;
  HI_U32             u32Size;

  u32Stride  = stride;
  u32Width   = width;
  u32Height  = height;

  u32LStride = u32Stride;
  u32CStride = u32Stride;

  u32LumaSize = (u32LStride * height);
  u32ChrmSize = (u32CStride * height) >> 2;/* YUV 420 */
  u32Size = u32LumaSize + (u32ChrmSize << 1);

  /* alloc video buffer block ---------------------------------------------------------- */
  VbBlk = HI_MPI_VB_GetBlock(VB_INVALID_POOLID, u32Size, NULL);
  if (VB_INVALID_HANDLE == VbBlk) {
    printf("HI_MPI_VB_GetBlock err! size:%d\n", u32Size);
    return false;
  }
  u32PhyAddr = HI_MPI_VB_Handle2PhysAddr(VbBlk);
  if (0 == u32PhyAddr) {
    return false;
  }

  pVirAddr = (HI_U8 *)HI_MPI_SYS_Mmap(u32PhyAddr, u32Size);
  if (NULL == pVirAddr) {
    return false;
  }

  vdo_frm_info_.u32PoolId = HI_MPI_VB_Handle2PoolId(VbBlk);
  if (VB_INVALID_POOLID == vdo_frm_info_.u32PoolId) {
    return false;
  }

  printf("pool id :%d, phyAddr:%x,virAddr:%x\n", vdo_frm_info_.u32PoolId, u32PhyAddr, (int)pVirAddr);

  return true;
}

void CEncUserData::Stop() {

}

void * CEncUserData::ThreadProcess(void *pArg) {
  if (pArg) {
    ((CEncUserData*)pArg)->OnProcess();
  }
  return NULL;
}

void CEncUserData::OnProcess() {
  HI_U32             u32LStride;
  HI_U32             u32CStride;
  HI_U32             u32LumaSize;
  HI_U32             u32ChrmSize;
  HI_U32             u32Size;

  u32LStride = u32Stride;
  u32CStride = u32Stride;

  u32LumaSize = (u32LStride * u32Height);
  u32ChrmSize = (u32CStride * u32Height) >> 2;/* YUV 420 */
  u32Size = u32LumaSize + (u32ChrmSize << 1);

  while (true) {
    vdo_frm_info_.stVFrame.u32PhyAddr[0] = u32PhyAddr;
    vdo_frm_info_.stVFrame.u32PhyAddr[1] = vdo_frm_info_.stVFrame.u32PhyAddr[0] + u32LumaSize;
    vdo_frm_info_.stVFrame.u32PhyAddr[2] = vdo_frm_info_.stVFrame.u32PhyAddr[1] + u32ChrmSize;

    vdo_frm_info_.stVFrame.pVirAddr[0] = pVirAddr;
    vdo_frm_info_.stVFrame.pVirAddr[1] = vdo_frm_info_.stVFrame.pVirAddr[0] + u32LumaSize;
    vdo_frm_info_.stVFrame.pVirAddr[2] = vdo_frm_info_.stVFrame.pVirAddr[1] + u32ChrmSize;

    vdo_frm_info_.stVFrame.u32Width = u32Width;
    vdo_frm_info_.stVFrame.u32Height = u32Height;
    vdo_frm_info_.stVFrame.u32Stride[0] = u32LStride;
    vdo_frm_info_.stVFrame.u32Stride[1] = u32CStride;
    vdo_frm_info_.stVFrame.u32Stride[2] = u32CStride;
    vdo_frm_info_.stVFrame.enPixelFormat = PIXEL_FORMAT_YUV_SEMIPLANAR_420;
    vdo_frm_info_.stVFrame.u32Field = VIDEO_FIELD_INTERLACED;/* Intelaced D1,otherwise VIDEO_FIELD_FRAME */

    HI_MPI_VENC_SendFrame(venc_chn_, &vdo_frm_info_, 1);
  }
}

