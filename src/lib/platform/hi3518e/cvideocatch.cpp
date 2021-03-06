#include "cvideocatch.h"
#include "systemv/vzshm_c.h"
#include "vzbase/helper/stdafx.h"

HI_S32 HisiPutH264DataToBuffer(HI_S32 n_chn, VENC_STREAM_S *p_stream, void* p_usr_arg) {
  if (p_usr_arg) {
    return ((CVideoCatch*)p_usr_arg)->GetOneFrame(n_chn, p_stream);
  }
  return 0;
}

CVideoCatch::CVideoCatch()
  : p_shm_vdo_(NULL) {
}

CVideoCatch::~CVideoCatch() {
}


int32 CVideoCatch::Start() {
  p_shm_vdo_ = Shm_Create(SHM_VIDEO_0, SHM_VIDEO_0_SIZE);
  if (p_shm_vdo_ == NULL) {
    LOG(L_ERROR) << "can't open share memory.";
    return -1;
  }

  pthread_create(&id, NULL, SAMPLE_VENC_720P_CLASSIC, this);
  return 0;
}

HI_S32 CVideoCatch::GetOneFrame(HI_S32 n_chn, VENC_STREAM_S *p_stream) {
  if (n_chn != 0) {
    return 0;
  }

  Shm_W_Begin(p_shm_vdo_);
  uint32 n_vdo_data = 0;
  for (uint32 i = 0; i < p_stream->u32PackCount; i++) {
    Shm_W_Write(p_shm_vdo_,
                (char*)p_stream->pstPack[i].pu8Addr[0], p_stream->pstPack[i].u32Len[0],
                n_vdo_data);
    n_vdo_data += p_stream->pstPack[i].u32Len[0];

    if (p_stream->pstPack[i].u32Len[1] > 0) {
      Shm_W_Write(p_shm_vdo_,
                  (char*)p_stream->pstPack[i].pu8Addr[1], p_stream->pstPack[i].u32Len[1],
                  n_vdo_data);
      n_vdo_data += p_stream->pstPack[i].u32Len[1];
    }
  }

  struct timeval tv;
  gettimeofday(&tv, NULL);
  Shm_W_End(p_shm_vdo_, tv.tv_sec, tv.tv_usec);
  return 0;
}

#if 0
static HI_S32 s_s32MemDev = -1;

#define MEM_DEV_OPEN()                                            \
  do {                                                            \
    if (s_s32MemDev <= 0)                                         \
    {                                                             \
      s_s32MemDev = open("/dev/mem", O_CREAT | O_RDWR | O_SYNC);  \
      if (s_s32MemDev < 0)                                        \
      {                                                           \
        perror("Open dev/mem error");                             \
        return NULL;                                              \
      }                                                           \
    }                                                             \
  } while (0)

#define MEM_DEV_CLOSE()                                           \
  do {                                                            \
    HI_S32 s32Ret;                                                \
    if (s_s32MemDev > 0)                                          \
    {                                                             \
      s32Ret = close(s_s32MemDev);                                \
      if (HI_SUCCESS != s32Ret)                                   \
      {                                                           \
        perror("Close mem/dev Fail");                             \
        return NULL;                                              \
      }                                                           \
      s_s32MemDev = -1;                                           \
    }                                                             \
  } while (0)


void* GetYUVThread(void* pArg) {
  VPSS_GRP vGrp = 0;
  VPSS_CHN vChn = 2;
  VIDEO_FRAME_INFO_S stFrame;
  HI_S32 s32MilliSec = 2000;

  MEM_DEV_OPEN();

  if (HI_MPI_VPSS_SetDepth(vGrp, vChn, 2) != HI_SUCCESS) {
    printf("set depth error!!!\n");
    return NULL;
  }
  //usleep(100000);

  HI_S32 nSeq = 0;
  HI_S32 nSize = 0;
  HI_CHAR *pUserPageAddr[2];

  while (true) {
    memset(&stFrame, 0, sizeof(stFrame));
    if (HI_MPI_VPSS_GetChnFrame(vGrp, vChn, &stFrame, s32MilliSec) != HI_SUCCESS) {
      printf("get frame error!!!\n");
      //usleep(40000);
      continue;
    }
    nSize = (stFrame.stVFrame.u32Stride[0])*(stFrame.stVFrame.u32Height) * 3 / 2;
    pUserPageAddr[0] = (HI_CHAR *)HI_MPI_SYS_Mmap(stFrame.stVFrame.u32PhyAddr[0], nSize);
    if (NULL == pUserPageAddr[0]) {
      HI_MPI_VPSS_ReleaseChnFrame(vGrp, vChn, &stFrame);

      continue;
    }
#ifdef IVA
    Update(pUserPageAddr[0], 720 * 576 * 3 / 2);
#endif
    printf("Get frame\t%d\t%d\t%d\t0x%x\t0x%x\n",
           nSeq++, stFrame.stVFrame.u32Width, stFrame.stVFrame.u32Height,
           stFrame.stVFrame.u32PhyAddr[0], stFrame.stVFrame.pVirAddr[0]);

    HI_MPI_SYS_Munmap(pUserPageAddr[0], nSize);
    HI_MPI_VPSS_ReleaseChnFrame(vGrp, vChn, &stFrame);
  }

  MEM_DEV_CLOSE();
  return NULL;
}
#endif