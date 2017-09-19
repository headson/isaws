/************************************************************************
*Author      : Sober.Peng 17-08-17
*Description :
************************************************************************/
#include "cvideocatch.h"

#include "vzlogging/logging/vzloggingcpp.h"

void SendUsrImageToEncode(VENC_CHN chn,
                          const void *pimg, unsigned int nimg,
                          unsigned int nwidth, unsigned int nheight) {
  int nres = 0;
  const HI_U32 IMAGE_WIDTH = SHM_VIDEO_2_W;
  const HI_U32 IMAGE_HEIGHT = SHM_VIDEO_2_H;

  VIDEO_FRAME_INFO_S vdo_frm_info_;

  /* 分配物理buffer并且映射到用户空间 */
  VB_BLK handleY = VB_INVALID_HANDLE;
  do {
    handleY = HI_MPI_VB_GetBlock(VB_INVALID_POOLID,
                                 IMAGE_WIDTH * IMAGE_HEIGHT * 3 / 2, NULL);
  } while (VB_INVALID_HANDLE == handleY);
  if (handleY == VB_INVALID_HANDLE) {
    printf("getblock for y failed\n");
    return;
  }
  VB_POOL poolID = HI_MPI_VB_Handle2PoolId(handleY);

  HI_U32 phyYaddr = HI_MPI_VB_Handle2PhysAddr(handleY);
  if (phyYaddr == 0) {
    printf("HI_MPI_VB_Handle2PhysAddr for handleY failed\n");
    return;
  }
  HI_U8 *pVirYaddr = (HI_U8*)HI_MPI_SYS_Mmap(
                       phyYaddr, IMAGE_WIDTH * IMAGE_HEIGHT * 3 / 2);

  /* 图像帧结构初始化 */
  memset(&(vdo_frm_info_.stVFrame), 0x00, sizeof(VIDEO_FRAME_S));

  vdo_frm_info_.stVFrame.u32Width = IMAGE_WIDTH;
  vdo_frm_info_.stVFrame.u32Height = IMAGE_HEIGHT;
  vdo_frm_info_.stVFrame.enPixelFormat = PIXEL_FORMAT_YUV_SEMIPLANAR_420;
  vdo_frm_info_.u32PoolId = poolID;
  vdo_frm_info_.stVFrame.u32PhyAddr[0] = phyYaddr;
  vdo_frm_info_.stVFrame.u32PhyAddr[1] = phyYaddr + IMAGE_WIDTH * IMAGE_HEIGHT;

  vdo_frm_info_.stVFrame.pVirAddr[0] = pVirYaddr;
  vdo_frm_info_.stVFrame.pVirAddr[1] = pVirYaddr + IMAGE_WIDTH * IMAGE_HEIGHT;

  vdo_frm_info_.stVFrame.u32Stride[0] = IMAGE_WIDTH;
  vdo_frm_info_.stVFrame.u32Stride[1] = IMAGE_WIDTH;
  vdo_frm_info_.stVFrame.enPixelFormat = PIXEL_FORMAT_YUV_SEMIPLANAR_420;
  vdo_frm_info_.stVFrame.u32Field = VIDEO_FIELD_FRAME;/* Intelaced D1,otherwise VIDEO_FIELD_FRAME */

  memcpy(pVirYaddr, pimg, IMAGE_WIDTH * IMAGE_HEIGHT);
  memset(pVirYaddr + IMAGE_WIDTH * IMAGE_HEIGHT,
         128, IMAGE_WIDTH * IMAGE_HEIGHT / 2);

  static HI_U32 frame = 0;
  vdo_frm_info_.stVFrame.u64pts = frame++ * 40; // (HI_U64)w_sec << 32 | w_usec;
  vdo_frm_info_.stVFrame.u32TimeRef = frame * 2;
  nres = HI_MPI_VENC_SendFrame(chn, &vdo_frm_info_, 1000);
  /*LOG_INFO("HI_MPI_VENC_SendFrame 0x%x %d %d.", nres, 
           IMAGE_WIDTH, IMAGE_HEIGHT);*/

  /* 释放掉获取的vb物理地址和虚拟地址 */
  HI_MPI_SYS_Munmap(pVirYaddr, IMAGE_WIDTH * IMAGE_HEIGHT * 3 / 2);
  HI_MPI_VB_ReleaseBlock(handleY);
}

