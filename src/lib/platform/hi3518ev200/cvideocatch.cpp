#include "cvideocatch.h"

#include "vpsschnosd.h"

#include "systemv/shm/vzshm_c.h"
#include "vzbase/helper/stdafx.h"

#include "vzbase/base/vmessage.h"
#include "dispatcher/sync/dpclient_c.h"

extern void *vpss_chn_dump(void* arg);
extern void *send_usr_frame(void *arg);

CVideoCatch::CVideoCatch() {
  sys_enc.pid = 0;
  sys_enc.have_start = 0;

  usr_enc.pid = 0;
  usr_enc.chn = -1;

  chn1_yuv.pid = 0;
  chn1_yuv.chn = -1;
}

CVideoCatch::~CVideoCatch() {
}

HI_S32 HisiPutH264DataToBuffer(HI_S32 n_chn, VENC_STREAM_S *p_stream, void* p_usr_arg) {
  if (p_usr_arg) {
    return ((CVideoCatch*)p_usr_arg)->GetOneFrame(n_chn, p_stream);
  }
  return 0;
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
    printf("input count invaild\n");
    return;
  }
  for (i = 0; i < s32ChnTotal; i++) {
    /* decide the stream file name, and open file to save stream */
    VencChn = i;
    s32Ret = HI_MPI_VENC_GetChnAttr(VencChn, &stVencChnAttr);
    if(s32Ret != HI_SUCCESS) {
      printf("HI_MPI_VENC_GetChnAttr chn[%d] failed with %#x!\n", \
             VencChn, s32Ret);
      return;
    }
    enPayLoadType[i] = stVencChnAttr.stVeAttr.enType;

    /* Set Venc Fd. */
    VencFd[i] = HI_MPI_VENC_GetFd(i);
    if (VencFd[i] < 0) {
      printf("HI_MPI_VENC_GetFd failed with %#x!\n",
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

    OSD_Overlay(&((CVideoCatch*)pstPara->p_usr_arg)->enc_osd_);

    FD_ZERO(&read_fds);
    for (i = 0; i < s32ChnTotal; i++) {
      FD_SET(VencFd[i], &read_fds);
    }

    TimeoutVal.tv_sec  = 2;
    TimeoutVal.tv_usec = 0;
    s32Ret = select(maxfd + 1, &read_fds, NULL, NULL, &TimeoutVal);
    if (s32Ret < 0) {
      printf("select failed!\n");
      break;
    } else if (s32Ret == 0) {
      printf("get venc stream time out, exit thread\n");
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
            printf("HI_MPI_VENC_Query chn[%d] failed with %#x!\n", i, s32Ret);
            break;
          }

          /*******************************************************
          step 2.2 :suggest to check both u32CurPacks and u32LeftStreamFrames at the same time,for example:
           if(0 == stStat.u32CurPacks || 0 == stStat.u32LeftStreamFrames)
           {
          	printf("NOTE: Current  frame is NULL!\n");
          	continue;
           }
          *******************************************************/
          if(0 == stStat.u32CurPacks) {
            printf("NOTE: Current  frame is NULL!\n");
            continue;
          }
          /*******************************************************
           step 2.3 : malloc corresponding number of pack nodes.
          *******************************************************/
          stStream.pstPack = (VENC_PACK_S*)malloc(sizeof(VENC_PACK_S) * stStat.u32CurPacks);
          if (NULL == stStream.pstPack) {
            printf("malloc stream pack failed!\n");
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
            printf("HI_MPI_VENC_GetStream failed with %#x!\n", s32Ret);
            break;
          }

          /*******************************************************
           step 2.5 : save frame to file
          *******************************************************/
          // printf("------------------------ channel %d.\n", i);
          s32Ret = HisiPutH264DataToBuffer(i, &stStream, pstPara->p_usr_arg);
          if (HI_SUCCESS != s32Ret) {
            free(stStream.pstPack);
            stStream.pstPack = NULL;
            printf("save stream failed!\n");
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
  PIC_SIZE_E enSize[3] = {PIC_HD720, PIC_CIF, PIC_CIF};
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

  ISP_DEV IspDev = 0;
  ISP_EXPOSURE_ATTR_S ex_attr;

  /******************************************
   step  1: init sys variable
  ******************************************/
  memset(&stVbConf, 0, sizeof(VB_CONF_S));
  memset(&stViConfig, 0, sizeof(stViConfig));

  s32ChnNum = 3;
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
  if (s32ChnNum >= 3) {
    u32BlkSize = SAMPLE_COMM_SYS_CalcPicVbBlkSize(VIDEO_ENCODING_MODE_PAL, \
                 enSize[2], SAMPLE_PIXEL_FORMAT, SAMPLE_SYS_ALIGN_WIDTH);
    stVbConf.astCommPool[2].u32BlkSize = u32BlkSize;
    stVbConf.astCommPool[2].u32BlkCnt = 2;
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
    stVpssChnAttr.s32DstFrameRate = 30;
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
    stVpssChnAttr.s32DstFrameRate = 30;
    s32Ret = SAMPLE_COMM_VPSS_EnableChn(VpssGrp, VpssChn, &stVpssChnAttr, &stVpssChnMode, HI_NULL);
    if (HI_SUCCESS != s32Ret) {
      printf("Enable vpss chn failed!\n");
      goto END_VENC_1080P_CLASSIC_4;
    }
  }

  if (s32ChnNum >= 3) {
    s32Ret = SAMPLE_COMM_SYS_GetPicSize(VIDEO_ENCODING_MODE_PAL, enSize[2], &stSize);
    if (HI_SUCCESS != s32Ret) {
      printf("SAMPLE_COMM_SYS_GetPicSize failed!\n");
      goto END_VENC_1080P_CLASSIC_4;
    }
    VpssChn = 2;
    stVpssChnMode.enChnMode = VPSS_CHN_MODE_USER;
    stVpssChnMode.bDouble = HI_FALSE;
    stVpssChnMode.enPixelFormat = PIXEL_FORMAT_YUV_SEMIPLANAR_420;
    stVpssChnMode.u32Width = stSize.u32Width;
    stVpssChnMode.u32Height = stSize.u32Height;
    stVpssChnMode.enCompressMode = COMPRESS_MODE_NONE;

    stVpssChnAttr.s32SrcFrameRate = -1;
    stVpssChnAttr.s32DstFrameRate = -1;

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
    s32Ret = SAMPLE_COMM_VENC_Start(VencChn, enPayLoad[0], \
                                    VIDEO_ENCODING_MODE_PAL, enSize[0], enRcMode, u32Profile);
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
                                    VIDEO_ENCODING_MODE_PAL, enSize[1], enRcMode, u32Profile);
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

  /*** enSize[2] **/
  if (s32ChnNum >= 3) {
    VpssChn = 2;
    VencChn = 2;
    s32Ret = SAMPLE_COMM_VENC_Start(VencChn, enPayLoad[2], \
                                    VIDEO_ENCODING_MODE_PAL, enSize[2], enRcMode, u32Profile);
    if (HI_SUCCESS != s32Ret) {
      printf("Start Venc failed!\n");
      goto END_VENC_1080P_CLASSIC_5;
    }

    //s32Ret = SAMPLE_COMM_VENC_BindVpss(VencChn, VpssGrp, VpssChn);
    //if (HI_SUCCESS != s32Ret) {
    //  printf("Start Venc failed!\n");
    //  goto END_VENC_1080P_CLASSIC_5;
    //}

    VENC_COLOR2GREY_S vcs;
    vcs.bColor2Grey = HI_TRUE;
    HI_MPI_VENC_SetColor2Grey(VencChn, &vcs);
  }

  HI_MPI_ISP_GetExposureAttr(IspDev, &ex_attr);
  ex_attr.bByPass = HI_FALSE;
  ex_attr.stAuto.enAEMode = AE_MODE_FIX_FRAME_RATE;
  HI_MPI_ISP_SetExposureAttr(IspDev, &ex_attr);

  // OSD
  OSD_Init(&((CVideoCatch*)p)->enc_osd_);
  {
    Json::Value josd;
    Json::Reader jread;
    std::string sosd = "";
    Kvdb_GetKeyToString(KVDB_ENC_OSD,
                        strlen(KVDB_ENC_OSD),
                        &sosd);
    if (sosd.empty()) {
      sosd = "[{\"handle\": 0, \"enable\" : 1, \"x\" : 3,  \"y\" : 3,  \"size\" : 16 },\
               {\"handle\": 1, \"enable\" : 1, \"x\" : 3,  \"y\" : 3,  \"size\" : 16}, \
               {\"handle\": 2, \"enable\" : 1, \"x\" : 0,  \"y\" : 10, \"size\" : 40}]";
      if (jread.parse(sosd, josd)) {
        sosd = josd.toStyledString();
        Kvdb_SetKey(KVDB_ENC_OSD,
                    strlen(KVDB_ENC_OSD),
                    sosd.c_str(), sosd.size());
      }
    }
    if (jread.parse(sosd, josd)) {
      ((CVideoCatch*)p)->OSDAdjust(josd);
    }
  }

  /******************************************
   step 6: stream venc process -- get stream, then save it to file.
  ******************************************/
  SAMPLE_VENC_GETSTREAM_PARA_S gs_stPara;
  gs_stPara.bThreadStart  = HI_TRUE;
  gs_stPara.s32Cnt        = s32ChnNum;
  gs_stPara.p_usr_arg     = p;

  ((CVideoCatch*)p)->sys_enc.have_start = 1;
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
  case 3:
    VpssChn = 2;
    VencChn = 2;
    // SAMPLE_COMM_VENC_UnBindVpss(VencChn, VpssGrp, VpssChn);
    SAMPLE_COMM_VENC_Stop(VencChn);
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
  case 3:
    VpssChn = 2;
// SAMPLE_COMM_VPSS_DisableChn(VpssGrp, VpssChn);
  case 2:
    VpssChn = 1;
    SAMPLE_COMM_VPSS_DisableChn(VpssGrp, VpssChn);
  case 1:
    VpssChn = 0;
    SAMPLE_COMM_VPSS_DisableChn(VpssGrp, VpssChn);
    break;
  }

END_VENC_1080P_CLASSIC_3:     // vpss stop
  SAMPLE_COMM_VI_UnBindVpss(stViConfig.enViMode);
END_VENC_1080P_CLASSIC_2:     // vpss stop
  SAMPLE_COMM_VPSS_StopGroup(VpssGrp);
END_VENC_1080P_CLASSIC_1:     // vi stop
  SAMPLE_COMM_VI_StopVi(&stViConfig);
END_VENC_1080P_CLASSIC_0:     // system exit
  SAMPLE_COMM_SYS_Exit();
  return NULL;
}

bool CVideoCatch::Start() {
  bool bres = false;

  // video_0
  bres = sys_enc.shm[0].Create(SHM_VIDEO_0, SHM_VIDEO_0_SIZE);
  if (bres == false) {
    LOG(L_ERROR) << "can't open share memory.";
    return -1;
  }
  sys_enc.shm[0].SetWidth(SHM_VIDEO_0_W);
  sys_enc.shm[0].SetHeight(SHM_VIDEO_0_H);

  // video_1
  bres = sys_enc.shm[1].Create(SHM_VIDEO_1, SHM_VIDEO_1_SIZE);
  if (bres == false) {
    LOG(L_ERROR) << "can't open share memory.";
    return -1;
  }
  sys_enc.shm[1].SetWidth(SHM_VIDEO_1_W);
  sys_enc.shm[1].SetHeight(SHM_VIDEO_1_H);

  // video_2
  bres = sys_enc.shm[2].Create(SHM_VIDEO_2, SHM_VIDEO_2_SIZE);
  if (bres == false) {
    LOG(L_ERROR) << "can't open share memory.";
    return -1;
  }
  sys_enc.shm[2].SetWidth(SHM_VIDEO_2_W);
  sys_enc.shm[2].SetHeight(SHM_VIDEO_2_H);

  // image 0 to alg
  bres = chn1_yuv.shm.Create(SHM_IMAGE_0, SHM_IMAGE_0_SIZE);
  if (bres == false) {
    LOG(L_ERROR) << "can't open share memory.";
    return -1;
  }
  chn1_yuv.shm.SetWidth(SHM_IMAGE_0_W);
  chn1_yuv.shm.SetHeight(SHM_IMAGE_0_H);

  //
  pthread_create(&sys_enc.pid, NULL, VideoVencClassic, this);

  for (int i = 0; i < 20; i++) {
    if (1 == sys_enc.have_start) {
      break;
    }
  }
  usleep(4*1000*1000);
  chn1_yuv.chn = 1;
  pthread_create(&chn1_yuv.pid, NULL, vpss_chn_dump, &chn1_yuv);

  usr_enc.chn = 2;
  pthread_create(&usr_enc.pid, NULL, send_usr_frame, &usr_enc);
  return true;
}

void CVideoCatch::Stop() {

}

void CVideoCatch::SetOsdChn2(const char *osd) {
  memset(enc_osd_.ch2, 0, MAX_OSD_SIZE);
  strncpy(enc_osd_.ch2, osd, MAX_OSD_SIZE);
  LOG(L_INFO) << "osd ch2: " << enc_osd_.ch2;
}

/*
[
{
"handle": 0,
"enable": 1,
"x": 10,
"y": 10,
"size": 16
},
{
"handle": 1,
"enable": 1,
"x": 10,
"y": 10,
"size": 16
},
{
"handle": 2,
"enable": 1,
"x": 0,
"y": 10,
"size": 40
}
]
*/
bool CVideoCatch::OSDAdjust(const Json::Value &jchn) {
  for (int i = 0; i < MAX_OSD_HDL; i++) {
    if (!jchn[i]["handle"].isNull() &&
        !jchn[i]["enable"].isNull() &&
        !jchn[i]["x"].isNull() &&
        !jchn[i]["y"].isNull() &&
        !jchn[i]["size"].isNull()) {
      OSD_Adjust(0,
                 jchn[i]["handle"].asInt(),
                 SHM_IMAGE_0_W * jchn[i]["x"].asInt() / 100,
                 SHM_IMAGE_0_H * jchn[i]["y"].asInt() / 100,
                 (i == 2) ? 128 : 10,
                 ((jchn[i]["enable"].asInt() > 0) ? HI_TRUE : HI_FALSE));

      OSD_Adjust(1,
                 MAX_OSD_HDL + jchn[i]["handle"].asInt(),
                 SHM_IMAGE_1_W * jchn[i]["x"].asInt() / 100,
                 SHM_IMAGE_1_H * jchn[i]["y"].asInt() / 100,
                 (i == 2) ? 128 : 10,
                 ((jchn[i]["enable"].asInt() > 0) ? HI_TRUE : HI_FALSE));

    }
  }

  return true;
}

HI_S32 CVideoCatch::GetOneFrame(HI_S32 chn, VENC_STREAM_S *pStream) {
  if (chn < 0 || chn > 3) {
    return 0;
  }

  // LOG(L_INFO) << "channel "<<chn;
  for (uint32 i = 0; i < pStream->u32PackCount; i++) {
    int n_frm_type = 0;
    char *p_nal = (char*)pStream->pstPack[i].pu8Addr + pStream->pstPack[i].u32Offset;
    if (p_nal[0] == 0x00 && p_nal[1] == 0x00 && p_nal[1] == 0x01) {
      n_frm_type = p_nal[3] & 0x1f;
    } else if (p_nal[0] == 0x00 && p_nal[1] == 0x00 &&
               p_nal[2] == 0x00 && p_nal[3] == 0x01) {
      n_frm_type = p_nal[4] & 0x1f;
    }

    if (n_frm_type == 7) {
      sys_enc.shm[chn].WriteSps((char*)pStream->pstPack[i].pu8Addr + pStream->pstPack[i].u32Offset,
                                pStream->pstPack[i].u32Len - pStream->pstPack[i].u32Offset);
    } else if (n_frm_type == 8) {
      sys_enc.shm[chn].WritePps((char*)pStream->pstPack[i].pu8Addr + pStream->pstPack[i].u32Offset,
                                pStream->pstPack[i].u32Len - pStream->pstPack[i].u32Offset);
    } else if (n_frm_type == 5 || n_frm_type == 1) {
      struct timeval tv;
      gettimeofday(&tv, NULL);
      sys_enc.shm[chn].Write((char*)pStream->pstPack[i].pu8Addr + pStream->pstPack[i].u32Offset,
                             pStream->pstPack[i].u32Len - pStream->pstPack[i].u32Offset,
                             tv.tv_sec, tv.tv_usec);
    }
  }
  return 0;
}
