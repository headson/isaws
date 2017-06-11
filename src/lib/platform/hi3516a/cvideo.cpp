#include "cvideo.h"

#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "inc/vdefine.h"
#include "shm/vshmvideo.h"

#ifdef __cplusplus
extern "C"
{
#endif

#include "common/sample_comm.h"

#ifdef __cplusplus
};
#endif

const VIDEO_NORM_E gs_enNorm = VIDEO_ENCODING_MODE_PAL;

CVideo::CVideo() {
}

CVideo::~CVideo() {
}

#if 1
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
#endif

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

int32_t CVideo::VideoCapture(int32_t n_chn) {
  PAYLOAD_TYPE_E enPayLoad[3]= {PT_H264, PT_H264};
  PIC_SIZE_E enSize[3] = {PIC_HD1080, PIC_D1};
  HI_U32 u32Profile = 2;

  VB_CONF_S stVbConf;
  SAMPLE_VI_CONFIG_S stViConfig;
  memset(&stViConfig, 0, sizeof(stViConfig));

  VPSS_GRP VpssGrp;
  VPSS_CHN VpssChn;
  VPSS_GRP_ATTR_S stVpssGrpAttr;
  VPSS_CHN_ATTR_S stVpssChnAttr;
  VPSS_CHN_MODE_S stVpssChnMode;

  VENC_CHN VencChn;
  SAMPLE_RC_E enRcMode= SAMPLE_RC_CBR;

  HI_S32 s32ChnNum;

  HI_S32 s32Ret = HI_SUCCESS;
  HI_U32 u32BlkSize;
  SIZE_S stSize;
  VShmVideo v_shm_video_0;

  s32ChnNum = 2;
  
  int n_ret = v_shm_video_0.Open((int8_t*)DEF_SHM_VIDEO_0, sizeof(TAG_SHM_VIDEO));
  if (n_ret != 0) {
    LOG_ERROR("share video failed %d.\n", n_ret);
    return n_ret;
  }
  /******************************************
   step  1: init sys variable
  ******************************************/
  memset(&stVbConf,0,sizeof(VB_CONF_S));

  SAMPLE_COMM_VI_GetSizeBySensor(&enSize[0]);

  stVbConf.u32MaxPoolCnt = 128;

  /*video buffer*/
  u32BlkSize = SAMPLE_COMM_SYS_CalcPicVbBlkSize(gs_enNorm, enSize[0], SAMPLE_PIXEL_FORMAT, SAMPLE_SYS_ALIGN_WIDTH);
  stVbConf.astCommPool[0].u32BlkSize = u32BlkSize;
  stVbConf.astCommPool[0].u32BlkCnt = 14;

  u32BlkSize = SAMPLE_COMM_SYS_CalcPicVbBlkSize(gs_enNorm, enSize[1], SAMPLE_PIXEL_FORMAT, SAMPLE_SYS_ALIGN_WIDTH);
  stVbConf.astCommPool[1].u32BlkSize = u32BlkSize;
  stVbConf.astCommPool[1].u32BlkCnt =14;

  /******************************************
   step 2: mpp system init.
  ******************************************/
  s32Ret = SAMPLE_COMM_SYS_Init(&stVbConf);
  if (HI_SUCCESS != s32Ret) {
    LOG_ERROR("system init failed with %d!\n", s32Ret);
    goto END_VENC_1080P_CLASSIC_0;
  }

  /******************************************
   step 3: start vi dev & chn to capture
  ******************************************/
  stViConfig.enViMode   = SENSOR_TYPE;
  stViConfig.enRotate   = ROTATE_NONE;
  stViConfig.enNorm     = VIDEO_ENCODING_MODE_AUTO;
  stViConfig.enViChnSet = VI_CHN_SET_NORMAL;
  stViConfig.enWDRMode  = WDR_MODE_NONE;
  s32Ret = SAMPLE_COMM_VI_StartVi(&stViConfig);
  if (HI_SUCCESS != s32Ret) {
    LOG_ERROR("start vi failed!\n");
    goto END_VENC_1080P_CLASSIC_1;
  }

  /******************************************
   step 4: start vpss and vi bind vpss
  ******************************************/
  s32Ret = SAMPLE_COMM_SYS_GetPicSize(gs_enNorm, enSize[0], &stSize);
  if (HI_SUCCESS != s32Ret) {
    LOG_ERROR("SAMPLE_COMM_SYS_GetPicSize failed!\n");
    goto END_VENC_1080P_CLASSIC_1;
  }

  VpssGrp = 0;
  stVpssGrpAttr.u32MaxW   = stSize.u32Width;
  stVpssGrpAttr.u32MaxH   = stSize.u32Height;
  stVpssGrpAttr.bIeEn     = HI_FALSE;
  stVpssGrpAttr.bNrEn     = HI_TRUE;
  stVpssGrpAttr.bHistEn   = HI_FALSE;
  stVpssGrpAttr.bDciEn    = HI_FALSE;
  stVpssGrpAttr.enDieMode = VPSS_DIE_MODE_NODIE;
  stVpssGrpAttr.enPixFmt  = SAMPLE_PIXEL_FORMAT;

  s32Ret = SAMPLE_COMM_VPSS_StartGroup(VpssGrp, &stVpssGrpAttr);
  if (HI_SUCCESS != s32Ret) {
    LOG_ERROR("Start Vpss failed!\n");
    goto END_VENC_1080P_CLASSIC_2;
  }

  s32Ret = SAMPLE_COMM_VI_BindVpss(stViConfig.enViMode);
  if (HI_SUCCESS != s32Ret) {
    LOG_ERROR("Vi bind Vpss failed!\n");
    goto END_VENC_1080P_CLASSIC_3;
  }

  VpssChn = 0;
  stVpssChnMode.enChnMode      = VPSS_CHN_MODE_USER;
  stVpssChnMode.bDouble        = HI_FALSE;
  stVpssChnMode.enPixelFormat  = SAMPLE_PIXEL_FORMAT;
  stVpssChnMode.u32Width       = stSize.u32Width;
  stVpssChnMode.u32Height      = stSize.u32Height;
  stVpssChnMode.enCompressMode = COMPRESS_MODE_SEG;
  memset(&stVpssChnAttr, 0, sizeof(stVpssChnAttr));
  stVpssChnAttr.s32SrcFrameRate = -1;
  stVpssChnAttr.s32DstFrameRate = -1;
  s32Ret = SAMPLE_COMM_VPSS_EnableChn(VpssGrp, VpssChn, &stVpssChnAttr, &stVpssChnMode, HI_NULL);
  if (HI_SUCCESS != s32Ret) {
    LOG_ERROR("Enable vpss chn failed!\n");
    goto END_VENC_1080P_CLASSIC_4;
  }

  VpssChn = 1;
  stVpssChnMode.enChnMode       = VPSS_CHN_MODE_USER;
  stVpssChnMode.bDouble         = HI_FALSE;
  stVpssChnMode.enPixelFormat   = SAMPLE_PIXEL_FORMAT;
  stVpssChnMode.u32Width        = 352;//stSize.u32Width;
  stVpssChnMode.u32Height       = 288;//stSize.u32Height;
  stVpssChnMode.enCompressMode  = COMPRESS_MODE_SEG;
  stVpssChnAttr.s32SrcFrameRate = -1;
  stVpssChnAttr.s32DstFrameRate = -1;
  s32Ret = SAMPLE_COMM_VPSS_EnableChn(VpssGrp, VpssChn, &stVpssChnAttr, &stVpssChnMode, HI_NULL);
  if (HI_SUCCESS != s32Ret) {
    LOG_ERROR("Enable vpss chn failed!\n");
    goto END_VENC_1080P_CLASSIC_4;
  }

  VpssChn = 2;
  stVpssChnMode.enChnMode       = VPSS_CHN_MODE_USER;
  stVpssChnMode.bDouble         = HI_FALSE;
  stVpssChnMode.enPixelFormat   = SAMPLE_PIXEL_FORMAT;
  stVpssChnMode.u32Width        = 720;
  stVpssChnMode.u32Height       = 576;
  stVpssChnMode.enCompressMode  = COMPRESS_MODE_NONE;
  stVpssChnAttr.s32SrcFrameRate = 60;
  stVpssChnAttr.s32DstFrameRate = 25;
  s32Ret = SAMPLE_COMM_VPSS_EnableChn(VpssGrp, VpssChn, &stVpssChnAttr, &stVpssChnMode, HI_NULL);
  if (HI_SUCCESS != s32Ret) {
    LOG_ERROR("Enable vpss chn failed!\n");
    goto END_VENC_1080P_CLASSIC_4;
  }

  // get YUV frame
  //pthread_t pid;
  //pthread_create(&pid, 0, GetYUVThread, NULL);

  /******************************************
   step 5: start stream venc
  ******************************************/
  /*** HD1080P **/
  enRcMode = SAMPLE_RC_CBR;
  //enRcMode = SAMPLE_RC_VBR;
  //enRcMode = SAMPLE_RC_FIXQP;
  enSize[0] = PIC_D1;
  enSize[1] = PIC_CIF;

  VpssGrp = 0;
  VpssChn = 0;
  VencChn = 0;
  s32Ret = SAMPLE_COMM_VENC_Start(VencChn, enPayLoad[0], gs_enNorm, enSize[0], enRcMode,u32Profile);
  if (HI_SUCCESS != s32Ret) {
    LOG_ERROR("Start Venc failed!\n");
    goto END_VENC_1080P_CLASSIC_5;
  }

  s32Ret = SAMPLE_COMM_VENC_BindVpss(VencChn, VpssGrp, VpssChn);
  if (HI_SUCCESS != s32Ret) {
    LOG_ERROR("Start Venc failed!\n");
    goto END_VENC_1080P_CLASSIC_5;
  }

  /*** 1080p **/
  VpssChn = 1;
  VencChn = 1;
  s32Ret = SAMPLE_COMM_VENC_Start(VencChn, enPayLoad[1], gs_enNorm, enSize[1], enRcMode,u32Profile);
  if (HI_SUCCESS != s32Ret) {
    LOG_ERROR("Start Venc failed!\n");
    goto END_VENC_1080P_CLASSIC_5;
  }

  s32Ret = SAMPLE_COMM_VENC_BindVpss(VencChn, VpssGrp, VpssChn);
  if (HI_SUCCESS != s32Ret) {
    LOG_ERROR("Start Venc failed!\n");
    goto END_VENC_1080P_CLASSIC_5;
  }

  /******************************************
  step 6: stream venc process -- get stream, then save it to file.
  ******************************************/
  s32Ret = SAMPLE_COMM_VENC_StartGetStream(s32ChnNum, (void*)&v_shm_video_0);
  if (HI_SUCCESS != s32Ret) {
    LOG_ERROR("Start Venc failed!\n");
    goto END_VENC_1080P_CLASSIC_5;
  }

  printf("please press twice ENTER to exit this sample\n");
  getchar();
  getchar();

  /******************************************
   step 7: exit process
  ******************************************/
  SAMPLE_COMM_VENC_StopGetStream();

END_VENC_1080P_CLASSIC_5:
  VpssGrp = 0;

  VpssChn = 0;
  VencChn = 0;
  SAMPLE_COMM_VENC_UnBindVpss(VencChn, VpssGrp, VpssChn);
  SAMPLE_COMM_VENC_Stop(VencChn);

  VpssChn = 1;
  VencChn = 1;
  SAMPLE_COMM_VENC_UnBindVpss(VencChn, VpssGrp, VpssChn);
  SAMPLE_COMM_VENC_Stop(VencChn);

  SAMPLE_COMM_VI_UnBindVpss(stViConfig.enViMode);
END_VENC_1080P_CLASSIC_4:	//vpss stop
  VpssGrp = 0;
  VpssChn = 0;
  SAMPLE_COMM_VPSS_DisableChn(VpssGrp, VpssChn);
  VpssChn = 1;
  SAMPLE_COMM_VPSS_DisableChn(VpssGrp, VpssChn);

END_VENC_1080P_CLASSIC_3:   //vpss stop
  SAMPLE_COMM_VI_UnBindVpss(stViConfig.enViMode);
END_VENC_1080P_CLASSIC_2:   //vpss stop
  SAMPLE_COMM_VPSS_StopGroup(VpssGrp);
END_VENC_1080P_CLASSIC_1:   //vi stop
  SAMPLE_COMM_VI_StopVi(&stViConfig);
END_VENC_1080P_CLASSIC_0:   //system exit
  SAMPLE_COMM_SYS_Exit();

  return s32Ret;
}

