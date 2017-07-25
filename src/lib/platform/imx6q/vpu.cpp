#include "vpu.h"

#include <stdlib.h>
#include <string.h>

#include "vzbase/helper/stdafx.h"
//#include "vzlogging/logging/vzlogging.h"

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

  sps_size_ = pps_size_ = jpeg_size_ = 0;
  memset(head_, 0, 1024);

  file = NULL;//fopen("./test.h264", "wb+");
}

CVpu::~CVpu() {
  vpu_close();
}

// 编码创建
bool CVpu::vpu_setup() {
  int nRet;
  EncHandle handle   = {0};
  EncOpenParam encop = {0};

  /* get physical contigous bit stream buffer */
  cMemDesc.size = VPU_BUFFER;
  nRet = IOGetPhyMem(&cMemDesc);
  if (nRet) {
    LOG_ERROR("Unable to obtain physical memory.");
    return false;
  }
  cMemDesc.virt_uaddr = IOGetVirtMem(&cMemDesc);
  LOG_INFO("virt uaddr 0x%x.\n", cMemDesc.virt_uaddr);

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
  encop.userGamma             = (unsigned int)(0.75*32768); /* (0*32768 <= gamma <= 1*32768) */
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
    return false;
  }
  enc_fd = handle;

  LOG_INFO("hdl 0x%x, w %d, h %d, enc %d, bit %d, iGop %d.\n",
           (unsigned int)enc_fd, (int)nWidth, (int)nHeight, eEncFormat, nBitrate, nIGopSize);
  return true;
}

// 编码关闭
void CVpu::vpu_close() {
  if (enc_fd) {
    LOG_INFO("hdl 0x%x close.", (unsigned int)enc_fd);

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
int CVpu::vpu_alloc_encbuffs() {
  int i;
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

  LOG_INFO("hdl 0x%x frame number %d.", (unsigned int)enc_fd, nFrmNums);
  // DST frame buffer
  pFrmDesc = (vpu_mem_desc *)calloc(sizeof(vpu_mem_desc), nFrmNums);
  if (pFrmDesc == NULL) {
    LOG_ERROR("hdl 0x%x Can't calloc vpu_mem_desc.", (unsigned int)enc_fd);
    goto err;
  }

  pFrmBufs = (FrameBuffer*)calloc(sizeof(FrameBuffer), nFrmNums);
  if (pFrmBufs == NULL) {
    LOG_ERROR("hdl 0x%x Can't calloc FrameBuffer.", (unsigned int)enc_fd);
    goto err;
  }

  nEncWidth  = (nWidth + 15) & ~15;
  nEncHeight = (nHeight + 15) & ~15;
  for (i = 0; i < initinfo.minFrameBufferCount+nExtFrmBuffCount; i++) {
    memset(&pFrmDesc[i], 0, sizeof(vpu_mem_desc));
    pFrmDesc[i].size = nEncWidth * nEncHeight * 3/2;
    if (IOGetPhyMem(&pFrmDesc[i])) {
      LOG_ERROR("hdl 0x%x Frame buffer allocation failure.", (unsigned int)enc_fd);
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
    LOG_ERROR("hdl 0x%x Register frame buffer failed", (unsigned int)enc_fd);
    goto err;
  }
  return 0;

err:
  vpu_free_encbuffs();
  LOG_ERROR("hdl 0x%x vpu_alloc_encbuffs failed.", (unsigned int)enc_fd);
  return -1;
}

// 释放buffer
void CVpu::vpu_free_encbuffs() {
  if (pFrmDesc) {
    //for (int i = 0; i < (nFrmNums - 1); i++)
    for (unsigned int i = 0; i < nFrmNums; i++) {
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
int CVpu::vpu_alloc_srcbuffs(TAG_VPU_SRC *pSrc, int w, int h) {
  int nFrmSize = w * h;
  int nImgSize = nFrmSize * 3 / 2;

  pSrc->stFrmDesc.size = nImgSize;
  IOGetPhyMem(&pSrc->stFrmDesc);
  if (pSrc->stFrmDesc.phy_addr == 0) {
    LOG_ERROR("IOGetPhyMem %d failed.\n", nImgSize);
    return -1;
  }
  pSrc->stFrmDesc.virt_uaddr = IOGetVirtMem(&pSrc->stFrmDesc);

  pSrc->stFrmBuff.strideY = w;
  pSrc->stFrmBuff.strideC = w >> 1;
  pSrc->stFrmBuff.bufY    = pSrc->stFrmDesc.phy_addr;
  pSrc->stFrmBuff.bufCb   = pSrc->stFrmBuff.bufY + nFrmSize;
  pSrc->stFrmBuff.bufCr   = pSrc->stFrmBuff.bufCb+ (nFrmSize >> 2);
  return 0;
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
int CVpu::enc_process(FrameBuffer *pSrcFrm, char *pDstData, int nDstSize) {
  int nRet = -1;
  unsigned char sSlice[9] = {0};

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
    LOG_ERROR("0x%x Can't enc next frame %d-0x%x.\n",
              (unsigned int)enc_fd, nRet, (unsigned int)pSrcFrm);
    return nRet;
  }

  nRet = RETCODE_FAILURE;
  do {
    while (vpu_IsBusy())
      vpu_WaitForInt(100);

    nRet = vpu_EncGetOutputInfo(enc_fd, &stOutInfo);
  } while (nRet != RETCODE_SUCCESS);
  //nSlice = stOutInfo.sliceInfo.size;

  // sps\pps head
  unsigned int nVdoLen = 0;
  if (eEncFormat == STD_MJPG) { // MJPG
    bGetIFrame = true;
    memcpy(pDstData, head_, jpeg_size_);
    nVdoLen += jpeg_size_;
  }

  // video
  unsigned int nDataAddr = cMemDesc.virt_uaddr + stOutInfo.bitstreamBuffer - cMemDesc.phy_addr;
  memcpy(pDstData+nVdoLen, (unsigned char*)nDataAddr, stOutInfo.bitstreamSize);
  nVdoLen              += stOutInfo.bitstreamSize;

  nDstSize = nVdoLen;
  return nVdoLen;
}

int CVpu::enc_h264(FrameBuffer *pSrcFrm, CShmVdo *shm_vdo) {
  int nRet = -1;
  unsigned char sSlice[9] = {0};

  EncParam stEncParam = {0};
  EncOutputInfo stOutInfo = {0};
  if (enc_fd == NULL) return nRet;

  if (eEncFormat == STD_MJPG) // MJPG
    bGetIFrame = true;

  stEncParam.sourceFrame = pSrcFrm;
  stEncParam.quantParam = 23;
  stEncParam.forceIPicture = bGetIFrame; // I帧设置
  stEncParam.skipPicture = 0;
  stEncParam.enableAutoSkip = 1;
  stEncParam.encLeftOffset = 0;
  stEncParam.encTopOffset = 0;

  stOutInfo.sliceInfo.enable = 1;
  stOutInfo.sliceInfo.addr = sSlice;
  vpu_EncGiveCommand(enc_fd, ENC_SET_REPORT_SLICEINFO, &stOutInfo.sliceInfo);
  nRet = vpu_EncStartOneFrame(enc_fd, &stEncParam);
  if (nRet != RETCODE_SUCCESS) {
    LOG_ERROR("0x%x Can't enc next frame %d-0x%x.\n",
              (unsigned int)enc_fd, nRet, (unsigned int)pSrcFrm);
    return nRet;
  }

  nRet = RETCODE_FAILURE;
  do {
    while (vpu_IsBusy())
      vpu_WaitForInt(100);

    nRet = vpu_EncGetOutputInfo(enc_fd, &stOutInfo);
  } while (nRet != RETCODE_SUCCESS);

  if (stOutInfo.picType == 1) {
    if (file) {
      fwrite(head_, 1, sps_size_ + pps_size_, file);
    }
  }

  // video
  unsigned int nDataAddr = cMemDesc.virt_uaddr +
                           stOutInfo.bitstreamBuffer -
                           cMemDesc.phy_addr;

  struct timeval tv;
  gettimeofday(&tv, NULL);
  shm_vdo->Write((const char*)nDataAddr,
                 stOutInfo.bitstreamSize,
                 tv.tv_sec, tv.tv_usec);

  if (file) {
    fwrite((const char*)nDataAddr, 1, stOutInfo.bitstreamSize, file);
  }
  return stOutInfo.bitstreamSize;
}

//////////////////////////////////////////////////////////////////////////
bool CVpu::vdo_restart() {
  if (!vpu_setup())
    return false;

  if (0 != vpu_alloc_encbuffs()) {
    vpu_close();
    return false;
  }

  // 头
  if (eEncFormat == STD_AVC) {       // H264 - I帧数据头
    unsigned int nDataBuff;
    EncHeaderParam enchdr_param = {0};
    enchdr_param.headerType = SPS_RBSP;
    vpu_EncGiveCommand(enc_fd, ENC_PUT_AVC_HEADER, &enchdr_param);
    nDataBuff = cMemDesc.virt_uaddr + enchdr_param.buf - cMemDesc.phy_addr;
    memcpy(head_, (void*)nDataBuff, enchdr_param.size);
    sps_size_ = enchdr_param.size;

    enchdr_param.headerType = PPS_RBSP;
    vpu_EncGiveCommand(enc_fd, ENC_PUT_AVC_HEADER, &enchdr_param);
    nDataBuff = cMemDesc.virt_uaddr + enchdr_param.buf - cMemDesc.phy_addr;
    memcpy(head_ + sps_size_, (void*)nDataBuff, enchdr_param.size);
    pps_size_ = enchdr_param.size;

    if (file) {
      fwrite(head_, 1, sps_size_+pps_size_, file);
    }
  } else if (eEncFormat == STD_MJPG) { // JPEG头
    jpeg_size_ = 0;
    EncParamSet enchdr_param;
    enchdr_param.size = 1024;
    enchdr_param.pParaSet = (unsigned char*)malloc(1024);
    vpu_EncGiveCommand(enc_fd,ENC_GET_JPEG_HEADER, &enchdr_param);
    memcpy(head_, enchdr_param.pParaSet, enchdr_param.size);
    jpeg_size_ = enchdr_param.size;
    free(enchdr_param.pParaSet);
  }
  return true;
}

void CVpu::vdo_stop() {
  vpu_close();
  vpu_free_encbuffs();
}
