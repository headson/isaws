#include "cvideocatch.h"

#include "systemv/shm/vzshm_c.h"
#include "systemv/flvmux/cflvmux.h"

#include "vzbase/helper/stdafx.h"

HI_S32 HisiPutH264DataToBuffer(HI_S32 n_chn, VENC_STREAM_S *p_stream, void* p_usr_arg) {
  if (p_usr_arg) {
    return ((CVideoCatch*)p_usr_arg)->GetOneFrame(n_chn, p_stream);
  }
  return 0;
}

CVideoCatch::CVideoCatch()
  : shm_video_() {
}

CVideoCatch::~CVideoCatch() {
}

/******************************************************************************
* funciton : get stream from each channels and save them
******************************************************************************/
HI_VOID SAMPLE_COMM_VENC_GetVencStreamProc(HI_VOID *p) {
  HI_S32 i;
  HI_S32 s32ChnTotal;
  VENC_CHN_ATTR_S stVencChnAttr;
  SAMPLE_VENC_GETSTREAM_PARA_S *pstPara;
  HI_S32 maxfd = 0;
  struct timeval TimeoutVal;
  fd_set read_fds;
  HI_S32 VencFd[VENC_MAX_CHN_NUM];

  VENC_CHN_STAT_S stStat;
  VENC_STREAM_S stStream;
  HI_S32 s32Ret;
  VENC_CHN VencChn;
  PAYLOAD_TYPE_E enPayLoadType[VENC_MAX_CHN_NUM];

  pstPara = (SAMPLE_VENC_GETSTREAM_PARA_S*)p;
  s32ChnTotal = pstPara->s32Cnt;

  /******************************************
   step 1:  check & prepare save-file & venc-fd
  ******************************************/
  if (s32ChnTotal >= VENC_MAX_CHN_NUM) {
    SAMPLE_PRT("input count invaild\n");
    return;
  }
  for (i = 0; i < s32ChnTotal; i++) {
    /* decide the stream file name, and open file to save stream */
    VencChn = i;
    s32Ret = HI_MPI_VENC_GetChnAttr(VencChn, &stVencChnAttr);
    if(s32Ret != HI_SUCCESS) {
      SAMPLE_PRT("HI_MPI_VENC_GetChnAttr chn[%d] failed with %#x!\n", \
                 VencChn, s32Ret);
      return;
    }
    enPayLoadType[i] = stVencChnAttr.stVeAttr.enType;

    /* Set Venc Fd. */
    VencFd[i] = HI_MPI_VENC_GetFd(i);
    if (VencFd[i] < 0) {
      SAMPLE_PRT("HI_MPI_VENC_GetFd failed with %#x!\n",
                 VencFd[i]);
      return;
    }
    if (maxfd <= VencFd[i]) {
      maxfd = VencFd[i];
    }
  }

  /******************************************
   step 2:  Start to get streams of each channel.
  ******************************************/
  while (HI_TRUE == pstPara->bThreadStart) {
    FD_ZERO(&read_fds);
    for (i = 0; i < s32ChnTotal; i++) {
      FD_SET(VencFd[i], &read_fds);
    }

    TimeoutVal.tv_sec  = 2;
    TimeoutVal.tv_usec = 0;
    s32Ret = select(maxfd + 1, &read_fds, NULL, NULL, &TimeoutVal);
    if (s32Ret < 0) {
      SAMPLE_PRT("select failed!\n");
      break;
    } else if (s32Ret == 0) {
      SAMPLE_PRT("get venc stream time out, exit thread\n");
      continue;
    } else {
      for (i = 0; i < s32ChnTotal; i++) {
        if (FD_ISSET(VencFd[i], &read_fds)) {
          /*******************************************************
           step 2.1 : query how many packs in one-frame stream.
          *******************************************************/
          memset(&stStream, 0, sizeof(stStream));
          s32Ret = HI_MPI_VENC_Query(i, &stStat);
          if (HI_SUCCESS != s32Ret) {
            SAMPLE_PRT("HI_MPI_VENC_Query chn[%d] failed with %#x!\n", i, s32Ret);
            break;
          }

          /*******************************************************
          step 2.2 :suggest to check both u32CurPacks and u32LeftStreamFrames at the same time,for example:
           if(0 == stStat.u32CurPacks || 0 == stStat.u32LeftStreamFrames)
           {
          	SAMPLE_PRT("NOTE: Current  frame is NULL!\n");
          	continue;
           }
          *******************************************************/
          if(0 == stStat.u32CurPacks) {
            SAMPLE_PRT("NOTE: Current  frame is NULL!\n");
            continue;
          }
          /*******************************************************
           step 2.3 : malloc corresponding number of pack nodes.
          *******************************************************/
          stStream.pstPack = (VENC_PACK_S*)malloc(sizeof(VENC_PACK_S) * stStat.u32CurPacks);
          if (NULL == stStream.pstPack) {
            SAMPLE_PRT("malloc stream pack failed!\n");
            break;
          }

          /*******************************************************
           step 2.4 : call mpi to get one-frame stream
          *******************************************************/
          stStream.u32PackCount = stStat.u32CurPacks;
          s32Ret = HI_MPI_VENC_GetStream(i, &stStream, HI_TRUE);
          if (HI_SUCCESS != s32Ret) {
            free(stStream.pstPack);
            stStream.pstPack = NULL;
            SAMPLE_PRT("HI_MPI_VENC_GetStream failed with %#x!\n", s32Ret);
            break;
          }

          /*******************************************************
           step 2.5 : save frame to file
          *******************************************************/
          s32Ret = HisiPutH264DataToBuffer(i, &stStream, pstPara->p_usr_arg);
          if (HI_SUCCESS != s32Ret) {
            free(stStream.pstPack);
            stStream.pstPack = NULL;
            SAMPLE_PRT("save stream failed!\n");
            break;
          }

          /*******************************************************
           step 2.6 : release stream
          *******************************************************/
          s32Ret = HI_MPI_VENC_ReleaseStream(i, &stStream);
          if (HI_SUCCESS != s32Ret) {
            free(stStream.pstPack);
            stStream.pstPack = NULL;
            break;
          }
          /*******************************************************
           step 2.7 : free pack nodes
          *******************************************************/
          free(stStream.pstPack);
          stStream.pstPack = NULL;
        }
      }
    }
  }
  return;
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
    u32BlkSize = SAMPLE_COMM_SYS_CalcPicVbBlkSize(VIDEO_ENCODING_MODE_PAL,\
                 enSize[0], SAMPLE_PIXEL_FORMAT, SAMPLE_SYS_ALIGN_WIDTH);
    stVbConf.astCommPool[0].u32BlkSize = u32BlkSize;
    stVbConf.astCommPool[0].u32BlkCnt = 2;
  }
  if(s32ChnNum >= 2) {
    u32BlkSize = SAMPLE_COMM_SYS_CalcPicVbBlkSize(VIDEO_ENCODING_MODE_PAL,\
                 enSize[1], SAMPLE_PIXEL_FORMAT, SAMPLE_SYS_ALIGN_WIDTH);
    stVbConf.astCommPool[1].u32BlkSize = u32BlkSize;
    stVbConf.astCommPool[1].u32BlkCnt  = 2;
  }

  /******************************************
   step 2: mpp system init.
  ******************************************/
  s32Ret = SAMPLE_COMM_SYS_Init(&stVbConf);
  if (HI_SUCCESS != s32Ret) {
    printf("system init failed with %d!\n", s32Ret);
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
    printf("start vi failed!\n");
    goto END_VENC_1080P_CLASSIC_1;
  }

  /******************************************
   step 4: start vpss and vi bind vpss
  ******************************************/
  s32Ret = SAMPLE_COMM_SYS_GetPicSize(VIDEO_ENCODING_MODE_PAL, enSize[0], &stSize);
  if (HI_SUCCESS != s32Ret) {
    printf("SAMPLE_COMM_SYS_GetPicSize failed!\n");
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
      printf("Start Vpss failed!\n");
      goto END_VENC_1080P_CLASSIC_2;
    }

    s32Ret = SAMPLE_COMM_VI_BindVpss(stViConfig.enViMode);
    if (HI_SUCCESS != s32Ret) {
      printf("Vi bind Vpss failed!\n");
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
      printf("Enable vpss chn failed!\n");
      goto END_VENC_1080P_CLASSIC_4;
    }
  }

  if(s32ChnNum >= 2) {
    s32Ret = SAMPLE_COMM_SYS_GetPicSize(VIDEO_ENCODING_MODE_PAL, enSize[1], &stSize);
    if (HI_SUCCESS != s32Ret) {
      printf("SAMPLE_COMM_SYS_GetPicSize failed!\n");
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
      printf("Enable vpss chn failed!\n");
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
                                    VIDEO_ENCODING_MODE_PAL, enSize[0], enRcMode,u32Profile);
    if (HI_SUCCESS != s32Ret) {
      printf("Start Venc failed!\n");
      goto END_VENC_1080P_CLASSIC_5;
    }

    s32Ret = SAMPLE_COMM_VENC_BindVpss(VencChn, VpssGrp, VpssChn);
    if (HI_SUCCESS != s32Ret) {
      printf("Start Venc failed!\n");
      goto END_VENC_1080P_CLASSIC_5;
    }
  }

  /*** enSize[1] **/
  if(s32ChnNum >= 2) {
    VpssChn = 1;
    VencChn = 1;
    s32Ret = SAMPLE_COMM_VENC_Start(VencChn, enPayLoad[1], \
                                    VIDEO_ENCODING_MODE_PAL, enSize[1], enRcMode,u32Profile);
    if (HI_SUCCESS != s32Ret) {
      printf("Start Venc failed!\n");
      goto END_VENC_1080P_CLASSIC_5;
    }

    s32Ret = SAMPLE_COMM_VENC_BindVpss(VencChn, VpssGrp, VpssChn);
    if (HI_SUCCESS != s32Ret) {
      printf("Start Venc failed!\n");
      goto END_VENC_1080P_CLASSIC_5;
    }
  }

  /******************************************
   step 6: stream venc process -- get stream, then save it to file.
  ******************************************/
  SAMPLE_VENC_GETSTREAM_PARA_S gs_stPara;
  gs_stPara.bThreadStart  = HI_TRUE;
  gs_stPara.s32Cnt        = s32ChnNum;
  gs_stPara.p_usr_arg     = p;
  SAMPLE_COMM_VENC_GetVencStreamProc((HI_VOID*)&gs_stPara);

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
  bool b_ret = false;

  b_ret = shm_video_.Create(SHM_VIDEO_0, SHM_VIDEO_0_SIZE);
  if (b_ret == false) {
    LOG(L_ERROR) << "can't open share memory.";
    return -1;
  }
  shm_video_.SetWidth(352);
  shm_video_.SetHeight(288);

  b_ret = shm_image_.Create(SHM_IMAGE_0, SHM_IMAGE_0_SIZE);
  if (b_ret == false) {
    LOG(L_ERROR) << "can't open share memory.";
    return -1;
  }
  shm_image_.SetWidth(352);
  shm_image_.SetHeight(288);

  pthread_create(&enc_pid_, NULL, VideoVencClassic, this);

  usleep(5*1000*1000);
  pthread_create(&yuv_pid_, NULL, GetYUVThread, this);
  return 0;
}

HI_S32 CVideoCatch::GetOneFrame(HI_S32 n_chn, VENC_STREAM_S *p_stream) {
  if (n_chn != 1) {
    return 0;
  }

  bool b_i_frame = false;
  if (H264E_NALU_SPS == p_stream->pstPack[0].DataType.enH264EType) {
    b_i_frame = true;
  }


  for (uint32 i = 0; i < p_stream->u32PackCount; i++) {
    int n_frm_type = 0;
    char *p_nal = (char*)p_stream->pstPack[i].pu8Addr + p_stream->pstPack[i].u32Offset;
    if (p_nal[0] == 0x00 && p_nal[1] == 0x00 && p_nal[1] == 0x01) {
      n_frm_type = p_nal[3] & 0x1f;
    } else if (p_nal[0] == 0x00 && p_nal[1] == 0x00 &&
               p_nal[2] == 0x00 && p_nal[3] == 0x01) {
      n_frm_type = p_nal[4] & 0x1f;
    }
    //printf("------------------------ frame %d. length %d.\n",
    //       n_frm_type,
    //       p_stream->pstPack[i].u32Len - p_stream->pstPack[i].u32Offset);

    if (n_frm_type == 7) {
      shm_video_.WriteSps((char*)p_stream->pstPack[i].pu8Addr + p_stream->pstPack[i].u32Offset,
                          p_stream->pstPack[i].u32Len - p_stream->pstPack[i].u32Offset);
    } else if (n_frm_type == 8) {
      shm_video_.WritePps((char*)p_stream->pstPack[i].pu8Addr + p_stream->pstPack[i].u32Offset,
                          p_stream->pstPack[i].u32Len - p_stream->pstPack[i].u32Offset);
    } else if (n_frm_type == 5 || n_frm_type == 1) {
      struct timeval tv;
      gettimeofday(&tv, NULL);
      shm_video_.Write((char*)p_stream->pstPack[i].pu8Addr + p_stream->pstPack[i].u32Offset,
                       p_stream->pstPack[i].u32Len - p_stream->pstPack[i].u32Offset,
                       tv.tv_sec, tv.tv_usec);
    }

    /*memcpy(s_vdo_data+n_vdo_data, (char*)p_stream->pstPack[i].pu8Addr + p_stream->pstPack[i].u32Offset,
           p_stream->pstPack[i].u32Len - p_stream->pstPack[i].u32Offset);
    n_vdo_data += p_stream->pstPack[i].u32Len - p_stream->pstPack[i].u32Offset;*/
  }
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

    struct timeval tv;
    gettimeofday(&tv, NULL);
    ((CVideoCatch*)pArg)->shm_image_.Write((const char*)pUserPageAddr[0],
                                           stFrame.stVFrame.u32Width * stFrame.stVFrame.u32Height * 3 / 2,
                                           tv.tv_sec, tv.tv_usec);

    HI_MPI_SYS_Munmap(pUserPageAddr[0], nSize);
    HI_MPI_VPSS_ReleaseChnFrame(vGrp, vChn, &stFrame);
  }

  MEM_DEV_CLOSE();
  return NULL;
}

HI_S32 CVideoCatch::RGN_CreateCover() {
  HI_S32 s32Ret = HI_FAILURE;
  RGN_ATTR_S stRgnAttr;

  /****************************************
  step 1: create overlay regions
  ****************************************/
  stRgnAttr.enType = OVERLAY_RGN;
  stRgnAttr.unAttr.stOverlay.enPixelFmt = PIXEL_FORMAT_RGB_1555;//PIXEL_FORMAT_RGB_565
  stRgnAttr.unAttr.stOverlay.stSize.u32Width  = 288;
  stRgnAttr.unAttr.stOverlay.stSize.u32Height = 128;
  stRgnAttr.unAttr.stOverlay.u32BgColor = 0xffffff;

  rgn_hdl_ = 0;
  s32Ret = HI_MPI_RGN_Create(rgn_hdl_, &stRgnAttr);
  if(HI_SUCCESS != s32Ret) {
    printf("HI_MPI_RGN_Create (%d) failed with %#x!\n",
           rgn_hdl_, s32Ret);
    return HI_FAILURE;
  }
  printf("the handle:%d,creat success!\n", rgn_hdl_);

  MPP_CHN_S stChn;
  stChn.enModId = HI_ID_GROUP;
  stChn.s32DevId = 0;
  stChn.s32ChnId = 0;

  printf("%s:[%d] stChn.s32ChnId is %d ,\n", __FUNCTION__, __LINE__, stChn.s32ChnId);

  RGN_CHN_ATTR_S stChnAttr;
  memset(&stChnAttr, 0, sizeof(stChnAttr));
  stChnAttr.bShow  = HI_TRUE;
  stChnAttr.enType = OVERLAY_RGN;

  stChnAttr.unChnAttr.stOverlayChn.stPoint.s32X    = 16;
  stChnAttr.unChnAttr.stOverlayChn.stPoint.s32Y    = 32;
  stChnAttr.unChnAttr.stOverlayChn.u32BgAlpha      = 128;
  stChnAttr.unChnAttr.stOverlayChn.u32FgAlpha      = 0;
  stChnAttr.unChnAttr.stOverlayChn.u32Layer        = 0;

  stChnAttr.unChnAttr.stOverlayChn.stQpInfo.bAbsQp = HI_FALSE;
  stChnAttr.unChnAttr.stOverlayChn.stQpInfo.s32Qp  = 0;

  stChnAttr.unChnAttr.stOverlayChn.stInvertColor.stInvColArea.u32Height = 32;
  stChnAttr.unChnAttr.stOverlayChn.stInvertColor.stInvColArea.u32Width  = 16;
  stChnAttr.unChnAttr.stOverlayChn.stInvertColor.u32LumThresh           = 64;
  stChnAttr.unChnAttr.stOverlayChn.stInvertColor.bInvColEn              = HI_TRUE;
  stChnAttr.unChnAttr.stOverlayChn.stInvertColor.enChgMod               = LESSTHAN_LUM_THRESH;

  s32Ret = HI_MPI_RGN_AttachToChn(rgn_hdl_, &stChn, &stChnAttr);
  if (HI_SUCCESS != s32Ret) {
    printf("HI_MPI_RGN_AttachToChn (%d) failed with %#x!\n", rgn_hdl_, s32Ret);
    return HI_FAILURE;
  }
  printf("display region to s32DevId %d chn success!\n", stChn.s32DevId);
}

#endif