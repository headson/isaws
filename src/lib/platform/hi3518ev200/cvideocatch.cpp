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

HI_VOID *VideoVencClassic(HI_VOID *p) {
  PAYLOAD_TYPE_E enPayLoad[3]= {PT_H264, PT_H264, PT_H264};
  PIC_SIZE_E enSize[3] = {PIC_HD1080, PIC_CIF, PIC_QVGA};
  HI_U32 u32Profile = 2;

  VB_CONF_S stVbConf;
  SAMPLE_VI_CONFIG_S stViConfig;

  VPSS_GRP VpssGrp;
  VPSS_CHN VpssChn;
  VPSS_GRP_ATTR_S stVpssGrpAttr;
  VPSS_CHN_ATTR_S stVpssChnAttr;
  VPSS_CHN_MODE_S stVpssChnMode;

  VENC_CHN VencChn;
  SAMPLE_RC_E enRcMode= SAMPLE_RC_CBR;

  HI_S32 s32ChnNum=0;

  HI_S32 s32Ret = HI_SUCCESS;
  HI_U32 u32BlkSize;
  SIZE_S stSize;
  char c;


  /******************************************
   step  1: init sys variable
  ******************************************/
  memset(&stVbConf, 0, sizeof(VB_CONF_S));
  memset(&stViConfig, 0, sizeof(stViConfig));

  s32ChnNum = 2;
  printf("s32ChnNum = %d\n",s32ChnNum);

  stVbConf.u32MaxPoolCnt = 128;

  /*video buffer*/
  if(s32ChnNum >= 1) {
    u32BlkSize = SAMPLE_COMM_SYS_CalcPicVbBlkSize(VIDEO_ENCODING_MODE_NTSC,\
                 enSize[0], SAMPLE_PIXEL_FORMAT, SAMPLE_SYS_ALIGN_WIDTH);
    stVbConf.astCommPool[0].u32BlkSize = u32BlkSize;
    stVbConf.astCommPool[0].u32BlkCnt = 2;
  }
  if(s32ChnNum >= 2) {
    u32BlkSize = SAMPLE_COMM_SYS_CalcPicVbBlkSize(VIDEO_ENCODING_MODE_NTSC,\
                 enSize[1], SAMPLE_PIXEL_FORMAT, SAMPLE_SYS_ALIGN_WIDTH);
    stVbConf.astCommPool[1].u32BlkSize = u32BlkSize;
    stVbConf.astCommPool[1].u32BlkCnt  = 2;
  }

  /******************************************
   step 2: mpp system init.
  ******************************************/
  s32Ret = SAMPLE_COMM_SYS_Init(&stVbConf);
  if (HI_SUCCESS != s32Ret) {
    SAMPLE_PRT("system init failed with %d!\n", s32Ret);
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
    SAMPLE_PRT("start vi failed!\n");
    goto END_VENC_1080P_CLASSIC_1;
  }

  /******************************************
   step 4: start vpss and vi bind vpss
  ******************************************/
  s32Ret = SAMPLE_COMM_SYS_GetPicSize(VIDEO_ENCODING_MODE_NTSC, enSize[0], &stSize);
  if (HI_SUCCESS != s32Ret) {
    SAMPLE_PRT("SAMPLE_COMM_SYS_GetPicSize failed!\n");
    goto END_VENC_1080P_CLASSIC_1;
  }
  if(s32ChnNum >= 1) {
    VpssGrp = 0;
    stVpssGrpAttr.u32MaxW = stSize.u32Width;
    stVpssGrpAttr.u32MaxH = stSize.u32Height;
    stVpssGrpAttr.bIeEn = HI_FALSE;
    stVpssGrpAttr.bNrEn = HI_TRUE;
    stVpssGrpAttr.bHistEn = HI_FALSE;
    stVpssGrpAttr.bDciEn = HI_FALSE;
    stVpssGrpAttr.enDieMode = VPSS_DIE_MODE_NODIE;
    stVpssGrpAttr.enPixFmt = PIXEL_FORMAT_YUV_SEMIPLANAR_420;

    s32Ret = SAMPLE_COMM_VPSS_StartGroup(VpssGrp, &stVpssGrpAttr);
    if (HI_SUCCESS != s32Ret) {
      SAMPLE_PRT("Start Vpss failed!\n");
      goto END_VENC_1080P_CLASSIC_2;
    }

    s32Ret = SAMPLE_COMM_VI_BindVpss(stViConfig.enViMode);
    if (HI_SUCCESS != s32Ret) {
      SAMPLE_PRT("Vi bind Vpss failed!\n");
      goto END_VENC_1080P_CLASSIC_2;
    }

    VpssChn = 0;
    stVpssChnMode.enChnMode      = VPSS_CHN_MODE_USER;
    stVpssChnMode.bDouble        = HI_FALSE;
    stVpssChnMode.enPixelFormat  = PIXEL_FORMAT_YUV_SEMIPLANAR_420;
    stVpssChnMode.u32Width       = stSize.u32Width;
    stVpssChnMode.u32Height      = stSize.u32Height;
    stVpssChnMode.enCompressMode = COMPRESS_MODE_SEG;
    memset(&stVpssChnAttr, 0, sizeof(stVpssChnAttr));
    stVpssChnAttr.s32SrcFrameRate = 30;
    stVpssChnAttr.s32DstFrameRate = 15;
    s32Ret = SAMPLE_COMM_VPSS_EnableChn(VpssGrp, VpssChn, &stVpssChnAttr,&stVpssChnMode, HI_NULL);
    if (HI_SUCCESS != s32Ret) {
      SAMPLE_PRT("Enable vpss chn failed!\n");
      goto END_VENC_1080P_CLASSIC_4;
    }
  }

  if(s32ChnNum >= 2) {
    s32Ret = SAMPLE_COMM_SYS_GetPicSize(VIDEO_ENCODING_MODE_NTSC, enSize[1], &stSize);
    if (HI_SUCCESS != s32Ret) {
      SAMPLE_PRT("SAMPLE_COMM_SYS_GetPicSize failed!\n");
      goto END_VENC_1080P_CLASSIC_4;
    }
    VpssChn = 1;
    stVpssChnMode.enChnMode       = VPSS_CHN_MODE_USER;
    stVpssChnMode.bDouble         = HI_FALSE;
    stVpssChnMode.enPixelFormat   = PIXEL_FORMAT_YUV_SEMIPLANAR_420;
    stVpssChnMode.u32Width        = stSize.u32Width;
    stVpssChnMode.u32Height       = stSize.u32Height;
    stVpssChnMode.enCompressMode  = COMPRESS_MODE_SEG;
    stVpssChnAttr.s32SrcFrameRate = 30;
    stVpssChnAttr.s32DstFrameRate = 15;
    s32Ret = SAMPLE_COMM_VPSS_EnableChn(VpssGrp, VpssChn, &stVpssChnAttr, &stVpssChnMode, HI_NULL);
    if (HI_SUCCESS != s32Ret) {
      SAMPLE_PRT("Enable vpss chn failed!\n");
      goto END_VENC_1080P_CLASSIC_4;
    }
  }

  /******************************************
   step 5: start stream venc
  ******************************************/
  /*** HD1080P **/
  //enRcMode = SAMPLE_RC_CBR;
  enRcMode = SAMPLE_RC_VBR;
  //enRcMode = SAMPLE_RC_FIXQP;

  /*** enSize[0] **/
  if(s32ChnNum >= 1) {
    VpssGrp = 0;
    VpssChn = 0;
    VencChn = 0;
    s32Ret = SAMPLE_COMM_VENC_Start(VencChn, enPayLoad[0],\
                                    VIDEO_ENCODING_MODE_NTSC, enSize[0], enRcMode,u32Profile);
    if (HI_SUCCESS != s32Ret) {
      SAMPLE_PRT("Start Venc failed!\n");
      goto END_VENC_1080P_CLASSIC_5;
    }

    s32Ret = SAMPLE_COMM_VENC_BindVpss(VencChn, VpssGrp, VpssChn);
    if (HI_SUCCESS != s32Ret) {
      SAMPLE_PRT("Start Venc failed!\n");
      goto END_VENC_1080P_CLASSIC_5;
    }
  }

  /*** enSize[1] **/
  if(s32ChnNum >= 2) {
    VpssChn = 1;
    VencChn = 1;
    s32Ret = SAMPLE_COMM_VENC_Start(VencChn, enPayLoad[1], \
                                    VIDEO_ENCODING_MODE_NTSC, enSize[1], enRcMode,u32Profile);
    if (HI_SUCCESS != s32Ret) {
      SAMPLE_PRT("Start Venc failed!\n");
      goto END_VENC_1080P_CLASSIC_5;
    }

    s32Ret = SAMPLE_COMM_VENC_BindVpss(VencChn, VpssGrp, VpssChn);
    if (HI_SUCCESS != s32Ret) {
      SAMPLE_PRT("Start Venc failed!\n");
      goto END_VENC_1080P_CLASSIC_5;
    }
  }

  /******************************************
   step 6: stream venc process -- get stream, then save it to file.
  ******************************************/
  s32Ret = SAMPLE_COMM_VENC_StartGetStream(s32ChnNum, p);
  if (HI_SUCCESS != s32Ret) {
    SAMPLE_PRT("Start Venc failed!\n");
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
  switch(s32ChnNum) {
  case 2:
    VpssChn = 1;
    VencChn = 1;
    SAMPLE_COMM_VENC_UnBindVpss(VencChn, VpssGrp, VpssChn);
    SAMPLE_COMM_VENC_Stop(VencChn);
  case 1:
    VpssChn = 0;
    VencChn = 0;
    SAMPLE_COMM_VENC_UnBindVpss(VencChn, VpssGrp, VpssChn);
    SAMPLE_COMM_VENC_Stop(VencChn);
    break;

  }
  SAMPLE_COMM_VI_UnBindVpss(stViConfig.enViMode);

END_VENC_1080P_CLASSIC_4:	//vpss stop
  VpssGrp = 0;
  switch(s32ChnNum) {
  case 2:
    VpssChn = 1;
    SAMPLE_COMM_VPSS_DisableChn(VpssGrp, VpssChn);
  case 1:
    VpssChn = 0;
    SAMPLE_COMM_VPSS_DisableChn(VpssGrp, VpssChn);
    break;
  }

END_VENC_1080P_CLASSIC_2:    //vpss stop
  SAMPLE_COMM_VPSS_StopGroup(VpssGrp);
END_VENC_1080P_CLASSIC_1:	//vi stop
  SAMPLE_COMM_VI_StopVi(&stViConfig);
END_VENC_1080P_CLASSIC_0:	//system exit
  SAMPLE_COMM_SYS_Exit();
  return NULL;
}

int32 CVideoCatch::Start() {
  p_shm_vdo_ = Shm_Create(SHM_VIDEO_0, SHM_VIDEO_0_SIZE);
  if (p_shm_vdo_ == NULL) {
    LOG(L_ERROR) << "can't open share memory.";
    return -1;
  }

  pthread_create(&p_enc_id_, NULL, VideoVencClassic, this);

  usleep(5*1000*1000);
  pthread_create(&p_yuv_id_, NULL, GetYUVThread, this);
  return 0;
}

HI_S32 CVideoCatch::GetOneFrame(HI_S32 n_chn, VENC_STREAM_S *p_stream) {
  if (n_chn != 1) {
    return 0;
  }

  Shm_W_Begin(p_shm_vdo_);

  uint32 n_vdo_data = 0;
  for (uint32 i = 0; i < p_stream->u32PackCount; i++) {
    Shm_W_Write(p_shm_vdo_, (char*)p_stream->pstPack[i].pu8Addr + p_stream->pstPack[i].u32Offset,
                p_stream->pstPack[i].u32Len - p_stream->pstPack[i].u32Offset, n_vdo_data);
    n_vdo_data += p_stream->pstPack[i].u32Len - p_stream->pstPack[i].u32Offset;
  }

  struct timeval tv;
  gettimeofday(&tv, NULL);
  Shm_W_End(p_shm_vdo_, tv.tv_sec, tv.tv_usec);
  return 0;
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


void* CVideoCatch::GetYUVThread(void* pArg) {
  VPSS_GRP vGrp = 0;
  VPSS_CHN vChn = 1;
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
//#ifdef IVA
//    Update(pUserPageAddr[0], 720 * 576 * 3 / 2);
//#endif
    //printf("Get frame\t%d\t%d\t%d\t0x%x\t0x%x\n",
    //       nSeq++, stFrame.stVFrame.u32Width, stFrame.stVFrame.u32Height,
    //       stFrame.stVFrame.u32PhyAddr[0], stFrame.stVFrame.pVirAddr[0]);

    HI_MPI_SYS_Munmap(pUserPageAddr[0], nSize);
    HI_MPI_VPSS_ReleaseChnFrame(vGrp, vChn, &stFrame);
  }

  MEM_DEV_CLOSE();
  return NULL;
}
#endif