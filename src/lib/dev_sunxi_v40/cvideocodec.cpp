/************************************************************************
*Author      : Sober.Peng 17-10-17
*Description :
************************************************************************/
#include "cvideocodec.h"

#include "vzbase/helper/stdafx.h"

CVideoCodec::CVideoCodec(int codec,
                         int src_w, int src_h,
                         int dst_w, int dst_h)
  : pVideoEnc(NULL) {
  encode_format = (VENC_CODEC_TYPE)codec;
  src_width = src_w;
  src_height = src_h;
  dst_width = dst_w;
  dst_height = dst_h;
}

CVideoCodec::~CVideoCodec() {
  EncodeExit();
}

CVideoCodec *CVideoCodec::Create(int codec,
                                 int src_w, int src_h,
                                 int dst_w, int dst_h) {
  if (codec != VENC_CODEC_H264 && codec != VENC_CODEC_JPEG) {
    LOG(L_ERROR) << "param is failed.";
    return NULL;
  }
  if (src_w < dst_w || src_h < dst_h) {
    LOG(L_ERROR) << "param is failed.";
    return NULL;
  }

  CVideoCodec *vdo_codec = new CVideoCodec(codec,
      src_w, src_h,
      dst_w, dst_h);
  if (NULL == vdo_codec) {
    LOG(L_ERROR) << "CVideoCodec create failed.";
    return NULL;
  }
  return vdo_codec;
}

int CVideoCodec::GetIFrame() {
  return 0;
}

int CVideoCodec::SetBitRate(int bit_rate) {
  return 0;
}

int CVideoCodec::SetFrameRate(int frame_rate) {
  return 0;
}

int CVideoCodec::SetMaxKeyFrame(int max_key_frame) {
  return 0;
}

int CVideoCodec::EncodeFrame(unsigned int phy_addr,
                             int width, int height,
                             VideoCallback callback, void *usr_arg) {
  return 0;
}

int CVideoCodec::EncodeFrame(const char *vir_addr,
                             int width, int height,
                             VideoCallback callback, void *usr_arg) {
  int res = -1;
  VencInputBuffer inputBuffer;

  res = GetOneAllocInputBuffer(pVideoEnc, &inputBuffer);
  //if (0 != res){
  //  LOG(L_ERROR) << "GetOneAllocInputBuffer failed.";
  //  return -1;
  //}
  //static FILE *file = fopen("test.yuv", "wb+");
  //if (file) {
  //  fwrite(vir_addr, 1, width*height*3/2, file);
  //  fflush(file);
  //}
  memcpy(inputBuffer.pAddrVirY, vir_addr, width*height);
  memcpy(inputBuffer.pAddrVirC, vir_addr+width*height, width*height/2);

  inputBuffer.bEnableCorp       = 0;
  inputBuffer.sCropInfo.nLeft   = 240;
  inputBuffer.sCropInfo.nTop    = 240;
  inputBuffer.sCropInfo.nWidth  = 240;
  inputBuffer.sCropInfo.nHeight = 240;
  res = FlushCacheAllocInputBuffer(pVideoEnc, &inputBuffer);

  res = AddOneInputBuffer(pVideoEnc, &inputBuffer);
  res = VideoEncodeOneFrame(pVideoEnc);

  res = AlreadyUsedInputBuffer(pVideoEnc, &inputBuffer);
  res = ReturnOneAllocInputBuffer(pVideoEnc, &inputBuffer);

  VencOutputBuffer outputBuffer;
  res = GetOneBitstreamFrame(pVideoEnc, &outputBuffer);
  if (res) {
    return -1;
  }

  /*fwrite(outputBuffer.pData0, 1, outputBuffer.nSize0, out_file);
  if (outputBuffer.nSize1)
  {
  fwrite(outputBuffer.pData1, 1, outputBuffer.nSize1, out_file);
  }*/
  if (callback) {
    callback(&outputBuffer, usr_arg);
  }
  res = FreeOneBitStreamFrame(pVideoEnc, &outputBuffer);
  return 0;
}

static void InitJpegExif(EXIFInfo *exifinfo) {
  exifinfo->ThumbWidth = 640;
  exifinfo->ThumbHeight = 480;

  strcpy((char*)exifinfo->CameraMake, "allwinner make test");
  strcpy((char*)exifinfo->CameraModel, "allwinner model test");
  strcpy((char*)exifinfo->DateTime, "2014:02:21 10:54:05");
  strcpy((char*)exifinfo->gpsProcessingMethod, "allwinner gps");

  exifinfo->Orientation = 0;

  exifinfo->ExposureTime.num = 2;
  exifinfo->ExposureTime.den = 1000;

  exifinfo->FNumber.num = 20;
  exifinfo->FNumber.den = 10;
  exifinfo->ISOSpeed = 50;

  exifinfo->ExposureBiasValue.num = -4;
  exifinfo->ExposureBiasValue.den = 1;

  exifinfo->MeteringMode = 1;
  exifinfo->FlashUsed = 0;

  exifinfo->FocalLength.num = 1400;
  exifinfo->FocalLength.den = 100;

  exifinfo->DigitalZoomRatio.num = 4;
  exifinfo->DigitalZoomRatio.den = 1;

  exifinfo->WhiteBalance = 1;
  exifinfo->ExposureMode = 1;

  exifinfo->enableGpsInfo = 1;

  exifinfo->gps_latitude = 23.2368;
  exifinfo->gps_longitude = 24.3244;
  exifinfo->gps_altitude = 1234.5;

  exifinfo->gps_timestamp = (long)time(NULL);

  strcpy((char*)exifinfo->CameraSerialNum, "123456789");
  strcpy((char*)exifinfo->ImageName, "exif-name-test");
  strcpy((char*)exifinfo->ImageDescription, "exif-descriptor-test");
}

int CVideoCodec::EncodeInit() {
  // h264 param
  h264Param.bEntropyCodingCABAC = 1;
  h264Param.nBitrate = 512 * 1024;
  h264Param.nFramerate = 25;
  h264Param.nCodingMode = VENC_FRAME_CODING;
  h264Param.nMaxKeyInterval = 25;
  h264Param.sProfileLevel.nProfile = VENC_H264ProfileHigh;
  h264Param.sProfileLevel.nLevel = VENC_H264Level51;
  h264Param.sQPRange.nMinqp = 10;
  h264Param.sQPRange.nMaxqp = 40;

  InitJpegExif(&exifinfo);

  VencAllocateBufferParam bufferParam;
  memset(&baseConfig, 0, sizeof(VencBaseConfig));
  memset(&bufferParam, 0, sizeof(VencAllocateBufferParam));

  baseConfig.memops = MemAdapterGetOpsS();
  if (baseConfig.memops == NULL) {
    LOG(L_ERROR) << "MemAdapterGetOpsS failed.";
    return -1;
  }
  CdcMemOpen(baseConfig.memops);
  baseConfig.nInputWidth = src_width;
  baseConfig.nInputHeight = src_height;
  baseConfig.nStride = src_width;

  baseConfig.nDstWidth = dst_width;
  baseConfig.nDstHeight = dst_height;
  //the format of yuv file is yuv420p,
  //but the old ic only support the yuv420sp,
  //so use the func yu12_nv12() to config all the format.
  baseConfig.eInputFormat = VENC_PIXEL_YUV420SP;

  bufferParam.nSizeY = baseConfig.nInputWidth*baseConfig.nInputHeight;
  bufferParam.nSizeC = baseConfig.nInputWidth*baseConfig.nInputHeight / 2;
  bufferParam.nBufferNum = 4;

  pVideoEnc = VideoEncCreate(encode_format);
  if (NULL == pVideoEnc) {
    LOG(L_ERROR) << "VideoEncCreate failed.";
    return -1;
  }
  if (encode_format == VENC_CODEC_JPEG) {
    int quality = 90;
    int jpeg_mode = 1;
    VencJpegVideoSignal vs;
    vs.src_colour_primaries = VENC_YCC;
    vs.dst_colour_primaries = VENC_BT601;
    VideoEncSetParameter(pVideoEnc, VENC_IndexParamJpegExifInfo, &exifinfo);
    VideoEncSetParameter(pVideoEnc, VENC_IndexParamJpegQuality, &quality);
    VideoEncSetParameter(pVideoEnc, VENC_IndexParamJpegEncMode, &jpeg_mode);
    VideoEncSetParameter(pVideoEnc, VENC_IndexParamJpegVideoSignal, &vs);

    if (1 == jpeg_mode) {
      int jpeg_biteRate = 12 * 1024 * 1024;
      int jpeg_frameRate = 30;
      VencBitRateRange bitRateRange;
      bitRateRange.bitRateMax = 14 * 1024 * 1024;
      bitRateRange.bitRateMin = 10 * 1024 * 1024;

      VideoEncSetParameter(pVideoEnc, VENC_IndexParamBitrate, &jpeg_biteRate);
      VideoEncSetParameter(pVideoEnc, VENC_IndexParamFramerate, &jpeg_frameRate);
      VideoEncSetParameter(pVideoEnc, VENC_IndexParamSetBitRateRange, &bitRateRange);
    }
  } else if (encode_format == VENC_CODEC_H264) {
    int value;
    VideoEncSetParameter(pVideoEnc, VENC_IndexParamH264Param, &h264Param);

    value = 0;
    VideoEncSetParameter(pVideoEnc, VENC_IndexParamIfilter, &value);

    value = 0; //degree
    VideoEncSetParameter(pVideoEnc, VENC_IndexParamRotation, &value);

    //VideoEncSetParameter(pVideoEnc, VENC_IndexParamH264FixQP, &fixQP);

    //value = 720/4;
    //VideoEncSetParameter(pVideoEnc, VENC_IndexParamSliceHeight, &value);

    value = 0;
    VideoEncSetParameter(pVideoEnc, VENC_IndexParamSetPSkip, &value);

    //value = 1;
    //VideoEncSetParameter(pVideoEnc, VENC_IndexParamH264FastEnc, &value);
  }

  int res = VideoEncInit(pVideoEnc, &baseConfig);
  LOG(L_INFO) << "VideoEncInit result "<<res;

  if (encode_format == VENC_CODEC_H264) {
    VideoEncGetParameter(pVideoEnc, VENC_IndexParamH264SPSPPS, &sps_pps_data);
    LOG_INFO("sps_pps_data.nLength: %d", sps_pps_data.nLength);

    unsigned int head_num = 0;
    for (head_num = 0; head_num < sps_pps_data.nLength; head_num++) {
      LOG_INFO("the sps_pps :%02x", *(sps_pps_data.pBuffer + head_num));
    }
  }

  AllocInputBuffer(pVideoEnc, &bufferParam);
  return 0;
}

int CVideoCodec::EncodeExit() {
  if (pVideoEnc) {
    VideoEncDestroy(pVideoEnc);
    pVideoEnc = NULL;
  }
  return 0;
}
