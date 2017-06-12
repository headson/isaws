#include "vpu.h"

#include <stdlib.h>
#include <string.h>

#include "inc/vdefine.h"

#ifdef __cplusplus
extern "C" {
#endif
#include "sdk/inc/vpu_jpegtable.h"
#ifdef __cplusplus
}
#endif

///JPEG///////////////////////////////////////////////////////////////////////
static void jpgGetHuffTable(EncMjpgParam *param) {
  /* Rearrange and insert pre-defined Huffman table to deticated variable. */
  memcpy(param->huffBits[DC_TABLE_INDEX0], lumaDcBits,    16); /* Luma DC BitLength */
  memcpy(param->huffVal[DC_TABLE_INDEX0],  lumaDcValue,   16); /* Luma DC HuffValue */

  memcpy(param->huffBits[AC_TABLE_INDEX0], lumaAcBits,    16); /* Luma DC BitLength */
  memcpy(param->huffVal[AC_TABLE_INDEX0],  lumaAcValue,   162);/* Luma DC HuffValue */

  memcpy(param->huffBits[DC_TABLE_INDEX1], chromaDcBits,  16); /* Chroma DC BitLength */
  memcpy(param->huffVal[DC_TABLE_INDEX1],  chromaDcValue, 16); /* Chroma DC HuffValue */

  memcpy(param->huffBits[AC_TABLE_INDEX1], chromaAcBits,  16); /* Chroma AC BitLength */
  memcpy(param->huffVal[AC_TABLE_INDEX1],  chromaAcValue, 162);/* Chorma AC HuffValue */
}

static void jpgGetQMatrix(EncMjpgParam *param) {
  /* Rearrange and insert pre-defined Q-matrix to deticated variable. */
  memcpy(param->qMatTab[DC_TABLE_INDEX0], lumaQ2,     64);
  memcpy(param->qMatTab[AC_TABLE_INDEX0], chromaBQ2,  64);

  memcpy(param->qMatTab[DC_TABLE_INDEX1], param->qMatTab[DC_TABLE_INDEX0], 64);
  memcpy(param->qMatTab[AC_TABLE_INDEX1], param->qMatTab[AC_TABLE_INDEX0], 64);
}

static void jpgGetCInfoTable(EncMjpgParam *param) {
  int format = param->mjpg_sourceFormat;
  memcpy(param->cInfoTab, cInfoTable[format], 6 * 4);
}
///END JPEG////////////////////////////////////////////////////////////////////

CVpu::CVpu() {
  enc_fd = 0;

  nWidth = 720;
  nHeight= 576;

  nBitrate = 0;
  nIGopSize= 10;
  eEncFormat = STD_AVC;

  nFrmNums = 0;
  pFrmBufs = NULL;
  pFrmDesc = NULL;

  nHeadData = 0;
  memset(sHeadData, 0, 2*1024);
}

CVpu::~CVpu() {
  vpu_close();
}

// 编码创建
int32_t CVpu::vpu_setup() {
  int32_t nRet;
  EncHandle handle   = {0};
  EncOpenParam encop = {0};

  /* get physical contigous bit stream buffer */
  cMemDesc.size = VPU_BUFFER;
  nRet = IOGetPhyMem(&cMemDesc);
  if (nRet) {
    LOG_ERROR("Unable to obtain physical memory.");
    return RET_MALLOC_ERROR;
  }
  cMemDesc.virt_uaddr = IOGetVirtMem(&cMemDesc);
  printf("%s[%d] 0x%x.\n", __FUNCTION__, __LINE__,   cMemDesc.virt_uaddr);

  /* Fill up parameters for encoding */
  encop.bitstreamBuffer       = cMemDesc.phy_addr;
  encop.bitstreamBufferSize   = cMemDesc.size;
  encop.bitstreamFormat       = eEncFormat;
  encop.mapType               = 0;
  encop.linear2TiledEnable    = 0;
  if (eEncFormat == STD_MJPG)
    encop.linear2TiledEnable= 1;

  /* enc_width and enc_height in command line means encoder output size */
  encop.picWidth              = nWidth;
  encop.picHeight             = nHeight;

  /*Note: Frame rate cannot be less than 15fps per H.263 spec */
  encop.frameRateInfo         = 30;
  encop.bitRate               = nBitrate;
  encop.gopSize               = nIGopSize;
  encop.slicemode.sliceMode   = 0;   /* 0: 1 slice per picture; 1: Multiple slices per picture */
  encop.slicemode.sliceSizeMode = 0; /* 0: silceSize defined by bits; 1: sliceSize defined by MB number*/
  encop.slicemode.sliceSize   = 4000;/* Size of a slice in bits or MB numbers */
  encop.initialDelay          = 0;
  encop.vbvBufferSize         = 0;   /* 0 = ignore 8 */
  encop.intraRefresh          = 0;
  encop.sliceReport           = 0;
  encop.mbReport              = 0;
  encop.mbQpReport            = 0;
  encop.rcIntraQp             = -1;
  encop.userQpMax             = 35;
  encop.userQpMin             = 0;
  encop.userQpMinEnable       = 0;
  encop.userQpMaxEnable       = 1;
  encop.IntraCostWeight       = 0;
  encop.MEUseZeroPmv          = 0;
  encop.MESearchRange         = 3;                    /* (3: 16x16, 2:32x16, 1:64x32, 0:128x64, H.263(Short Header : always 3) */
  encop.userGamma             = (Uint32)(0.75*32768); /* (0*32768 <= gamma <= 1*32768) */
  encop.RcIntervalMode        = 3;                    /* 0:normal, 1:frame_level, 2:slice_level, 3: user defined Mb_level */
  encop.MbInterval            = 1;
  encop.avcIntra16x16OnlyModeEnable = 0;
  encop.ringBufferEnable      = 0;
  encop.dynamicAllocEnable    = 0;
  encop.chromaInterleave      = 0;
  if (eEncFormat == STD_AVC) {
    encop.EncStdParam.avcParam.avc_constrainedIntraPredFlag = 0;
    encop.EncStdParam.avcParam.avc_disableDeblk             = 0;
    encop.EncStdParam.avcParam.avc_deblkFilterOffsetAlpha   = 6;
    encop.EncStdParam.avcParam.avc_deblkFilterOffsetBeta    = 0;
    encop.EncStdParam.avcParam.avc_chromaQpOffset           = 10;

    encop.EncStdParam.avcParam.avc_audEnable                = 0;
    encop.EncStdParam.avcParam.interview_en                 = 0;
    encop.EncStdParam.avcParam.paraset_refresh_en           = 0;
    encop.EncStdParam.avcParam.prefix_nal_en                = 0;
    encop.EncStdParam.avcParam.mvc_extension                = 0;

    encop.EncStdParam.avcParam.avc_frameCroppingFlag        = 0;
    encop.EncStdParam.avcParam.avc_frameCropLeft            = 0;
    encop.EncStdParam.avcParam.avc_frameCropRight           = 0;
    encop.EncStdParam.avcParam.avc_frameCropTop             = 0;
    encop.EncStdParam.avcParam.avc_frameCropBottom          = 0;
  } else if (eEncFormat == STD_MJPG) {
    encop.EncStdParam.mjpgParam.mjpg_sourceFormat           = 0;
    encop.EncStdParam.mjpgParam.mjpg_restartInterval        = 60;
    encop.EncStdParam.mjpgParam.mjpg_thumbNailEnable        = 0;
    encop.EncStdParam.mjpgParam.mjpg_thumbNailWidth         = 0;
    encop.EncStdParam.mjpgParam.mjpg_thumbNailHeight        = 0;
    jpgGetHuffTable(&encop.EncStdParam.mjpgParam);
    jpgGetQMatrix(&encop.EncStdParam.mjpgParam);
    jpgGetCInfoTable(&encop.EncStdParam.mjpgParam);
  }
  nRet = vpu_EncOpen(&handle, &encop);
  if (nRet != RETCODE_SUCCESS) {
    LOG_ERROR("Encoder open failed %d %d-%d.\n", nRet, nWidth, nHeight);
    return RET_INVLIAD_HANDLE;
  }
  enc_fd = handle;

  LOG_INFO("hdl 0x%x, w %d, h %d, enc %d, bit %d, iGop %d.\n",
       (uint32_t)enc_fd, (int)nWidth, (int)nHeight, eEncFormat, nBitrate, nIGopSize);
  return RET_SUCCESS;
}

// 编码关闭
void CVpu::vpu_close() {
  if (enc_fd) {
    LOG_INFO("hdl 0x%x close.", (uint32_t)enc_fd);

    RetCode ret;
    ret = vpu_EncClose(enc_fd);
    if (ret == RETCODE_FRAME_NOT_COMPLETE) {
      vpu_SWReset(enc_fd, 0);
      vpu_EncClose(enc_fd);
    }
    enc_fd = NULL;

    /* free the physical memory */
    IOFreeVirtMem(&cMemDesc);
    IOFreePhyMem(&cMemDesc);
  }
}

// 分配buffer
int32_t CVpu::vpu_alloc_encbuffs() {
  int32_t i;
  RetCode nRet;
  int nEncStride, nSrcStride;
  int nEncWidth, nEncHeight;

  int nExtFrmBuffCount = 2;
  if (eEncFormat == STD_MJPG)
    nExtFrmBuffCount = 0;

  // minFrameBufferCount
  EncInitialInfo initinfo;
  nRet = vpu_EncGetInitialInfo(enc_fd, &initinfo);

  // Total Frame Buffer
  nFrmNums = initinfo.minFrameBufferCount+nExtFrmBuffCount;

  LOG_INFO("hdl 0x%x frame number %d.", (uint32_t)enc_fd, nFrmNums);
  // DST frame buffer
  pFrmDesc = (vpu_mem_desc *)calloc(sizeof(vpu_mem_desc), nFrmNums);
  if (pFrmDesc == NULL) {
    LOG_ERROR("hdl 0x%x Can't calloc vpu_mem_desc.", (uint32_t)enc_fd);
    goto err;
  }

  pFrmBufs = (FrameBuffer*)calloc(sizeof(FrameBuffer), nFrmNums);
  if (pFrmBufs == NULL) {
    LOG_ERROR("hdl 0x%x Can't calloc FrameBuffer.", (uint32_t)enc_fd);
    goto err;
  }

  nEncWidth  = (nWidth + 15) & ~15;
  nEncHeight = (nHeight + 15) & ~15;
  for (i = 0; i < initinfo.minFrameBufferCount+nExtFrmBuffCount; i++) {
    memset(&pFrmDesc[i], 0, sizeof(vpu_mem_desc));
    pFrmDesc[i].size = nEncWidth * nEncHeight * 3/2;
    if (IOGetPhyMem(&pFrmDesc[i])) {
      LOG_ERROR("hdl 0x%x Frame buffer allocation failure.", (uint32_t)enc_fd);
      goto err;
    }
    pFrmBufs[i].bufY  = pFrmDesc[i].phy_addr;
    pFrmBufs[i].bufCb = pFrmBufs[i].bufY + nEncWidth * nEncHeight;
    pFrmBufs[i].bufCr = pFrmBufs[i].bufCb + nEncWidth * nEncHeight / 4;

    pFrmBufs[i].strideY = nEncWidth;
    pFrmBufs[i].strideC = nEncWidth / 2;
    pFrmDesc[i].virt_uaddr = IOGetVirtMem (&pFrmDesc[i]);
    if (pFrmDesc[i].virt_uaddr <= 0) {
      IOFreePhyMem(&pFrmDesc[i]);
      goto err;
    }
  }

  nEncStride = (nWidth + 15) & ~15;
  nSrcStride = (nWidth + 15) & ~15;
  nRet = vpu_EncRegisterFrameBuffer(enc_fd, pFrmBufs, initinfo.minFrameBufferCount, nEncStride, nSrcStride, 0, 0, NULL);
  if (nRet != RETCODE_SUCCESS) {
    LOG_ERROR("hdl 0x%x Register frame buffer failed", (uint32_t)enc_fd);
    goto err;
  }
  return RET_SUCCESS;

err:
  vpu_free_encbuffs();
  LOG_ERROR("hdl 0x%x vpu_alloc_encbuffs failed.", (uint32_t)enc_fd);
  return RET_MALLOC_ERROR;
}

// 释放buffer
void CVpu::vpu_free_encbuffs() {
  if (pFrmDesc) {
    //for (int i = 0; i < (nFrmNums - 1); i++)
    for (uint32_t i = 0; i < nFrmNums; i++) {
      if (pFrmDesc[i].virt_uaddr)
        IOFreeVirtMem(&pFrmDesc[i]);

      if (pFrmDesc[i].phy_addr)
        IOFreePhyMem(&pFrmDesc[i]);

      memset(&(pFrmDesc[i]), 0, sizeof(vpu_mem_desc));
    }
    free(pFrmDesc);
    pFrmDesc = NULL;
  }

  if (pFrmBufs) {
    free(pFrmBufs);
    pFrmBufs = NULL;
  }
}

//////////////////////////////////////////////////////////////////////////
// 编码源分配
int32_t CVpu::vpu_alloc_srcbuffs(TAG_VPU_SRC *pSrc, int w, int h) {
  int nFrmSize = w * h;
  int nImgSize = nFrmSize * 3 / 2;

  pSrc->stFrmDesc.size = nImgSize;
  IOGetPhyMem(&pSrc->stFrmDesc);
  if (pSrc->stFrmDesc.phy_addr == 0) {
    LOG_ERROR("IOGetPhyMem %d failed.\n", nImgSize);
    return RET_MALLOC_ERROR;
  }
  pSrc->stFrmDesc.virt_uaddr = IOGetVirtMem(&pSrc->stFrmDesc);

  pSrc->stFrmBuff.strideY = w;
  pSrc->stFrmBuff.strideC = w >> 1;
  pSrc->stFrmBuff.bufY    = pSrc->stFrmDesc.phy_addr;
  pSrc->stFrmBuff.bufCb   = pSrc->stFrmBuff.bufY + nFrmSize;
  pSrc->stFrmBuff.bufCr   = pSrc->stFrmBuff.bufCb+ (nFrmSize >> 2);
  return RET_SUCCESS;
}

// 编码源释放
void CVpu::vpu_free_srcbuffs(TAG_VPU_SRC *pSrc) {
  if (pSrc->stFrmDesc.phy_addr) {
    IOFreeVirtMem(&pSrc->stFrmDesc);
    IOFreePhyMem(&pSrc->stFrmDesc);

    memset(&pSrc->stFrmBuff, 0, sizeof(pSrc->stFrmBuff));
  }
}

// 视频\照片编码
int32_t CVpu::enc_process(FrameBuffer *pSrcFrm, int8_t *pDstData, int32_t nDstSize, volatile bool& bRuning) {
  int32_t nRet = -1;
  uint8_t sSlice[9] = {0};

  EncParam stEncParam = {0};
  EncOutputInfo stOutInfo = {0};
  if (enc_fd == NULL) return nRet;

  if (eEncFormat == STD_MJPG) // MJPG
    bGetIFrame = true;

  stEncParam.sourceFrame      = pSrcFrm;
  stEncParam.quantParam       = 23;
  stEncParam.forceIPicture    = bGetIFrame; // I帧设置
  stEncParam.skipPicture      = 0;
  stEncParam.enableAutoSkip   = 1;
  stEncParam.encLeftOffset    = 0;
  stEncParam.encTopOffset     = 0;

  stOutInfo.sliceInfo.enable  = 1;
  stOutInfo.sliceInfo.addr    = sSlice;
  vpu_EncGiveCommand(enc_fd, ENC_SET_REPORT_SLICEINFO, &stOutInfo.sliceInfo);
  nRet = vpu_EncStartOneFrame(enc_fd, &stEncParam);
  if (nRet != RETCODE_SUCCESS) {
    printf("0x%x Can't enc next frame %d-0x%x.\n",
           (uint32_t)enc_fd, nRet, (uint32_t)pSrcFrm);
    return nRet;
  }

  nRet = RETCODE_FAILURE;
  do {
    while (vpu_IsBusy() && bRuning)
      vpu_WaitForInt(100);

    if(!bRuning) return 0;

    nRet = vpu_EncGetOutputInfo(enc_fd, &stOutInfo);
  } while (nRet != RETCODE_SUCCESS);
  //nSlice = stOutInfo.sliceInfo.size;

  // sps\pps head
  uint32_t nVdoLens = 0;
  if ((stOutInfo.picType == 0)     // I帧
      || (eEncFormat == STD_MJPG)) { // MJPG
    bGetIFrame = true;
    memcpy(pDstData, sHeadData, nHeadData);
    nVdoLens += nHeadData;
  }

  // video
  uint32_t nDataAddr = cMemDesc.virt_uaddr + stOutInfo.bitstreamBuffer - cMemDesc.phy_addr;
  memcpy(pDstData+nVdoLens, (uint8_t*)nDataAddr, stOutInfo.bitstreamSize);
  nVdoLens              += stOutInfo.bitstreamSize;

  nDstSize = nVdoLens;
  return nVdoLens;
}

//////////////////////////////////////////////////////////////////////////
int32_t CVpu::vdo_restart() {
  if (vpu_setup() != RET_SUCCESS)
    return RET_INVLIAD_HANDLE;

  if (vpu_alloc_encbuffs() != RET_SUCCESS) {
    vpu_close();
    return RET_MALLOC_ERROR;
  }

  // 头
  if (eEncFormat == STD_AVC) {       // H264 - I帧数据头
    nHeadData = 0;
    uint32_t nDataBuff;
    EncHeaderParam enchdr_param = {0};
    enchdr_param.headerType = SPS_RBSP;
    vpu_EncGiveCommand(enc_fd, ENC_PUT_AVC_HEADER, &enchdr_param);
    nDataBuff = cMemDesc.virt_uaddr + enchdr_param.buf - cMemDesc.phy_addr;
    memcpy(sHeadData + nHeadData, (void*)nDataBuff, enchdr_param.size);
    nHeadData = enchdr_param.size;

    enchdr_param.headerType = PPS_RBSP;
    vpu_EncGiveCommand(enc_fd, ENC_PUT_AVC_HEADER, &enchdr_param);
    nDataBuff = cMemDesc.virt_uaddr + enchdr_param.buf - cMemDesc.phy_addr;
    memcpy(sHeadData + nHeadData, (void*)nDataBuff, enchdr_param.size);
    nHeadData += enchdr_param.size;
  } else if (eEncFormat == STD_MJPG) { // JPEG头
    EncParamSet enchdr_param;
    enchdr_param.size = 1024;
    enchdr_param.pParaSet = (Uint8*)malloc(1024);
    vpu_EncGiveCommand(enc_fd,ENC_GET_JPEG_HEADER, &enchdr_param);
    memcpy(sHeadData, enchdr_param.pParaSet, enchdr_param.size);
    nHeadData = enchdr_param.size;
    free(enchdr_param.pParaSet);
  }
  return RET_SUCCESS;
}

void CVpu::vdo_stop() {
  vpu_close();
  vpu_free_encbuffs();
}
