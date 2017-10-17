/************************************************************************
*Author      : Sober.Peng 17-10-17
*Description :
************************************************************************/
#include "cvideocapture.h"

#include "vzbase/helper/stdafx.h"

CVideoCapture::CVideoCapture()
  : is_exit_(0)
  , camera_(NULL)
  , vdo_code_(NULL) {
}

CVideoCapture::~CVideoCapture() {
  if (camera_) {
    delete camera_;
    camera_ = NULL;
  }
  if (vdo_code_) {
    delete vdo_code_;
    vdo_code_ = NULL;
  }
}

int CVideoCapture::Start() {
  vdo_code_ = CVideoCodec::Create(VENC_CODEC_H264,
                                  640, 480, 640, 480);
  if (NULL == vdo_code_) {
    LOG(L_ERROR) << "CVideoCodec create failed.";
    return -1;
  }
  int res = vdo_code_->EncodeInit();
  if (0 != res) {
    LOG(L_ERROR) << "EncodeInit failed.";
    return -1;
  }

  camera_ = CCamera::Create("/dev/video0", 640, 480);
  if (NULL == camera_) {
    LOG(L_ERROR) << "CCamera create failed.";
    return -1;
  }

  is_exit_ = 0;
  thread_ = new vzbase::Thread();
  if (thread_) {
    thread_->Start(this);
  }
  return 0;
}

void CVideoCapture::Stop() {
  is_exit_ = 1;
  if (thread_) {
    thread_->Stop();
    thread_ = NULL;
  }

  if (camera_) {
    delete camera_;
    camera_ = NULL;
  }

  if (vdo_code_) {
    delete vdo_code_;
    vdo_code_ = NULL;
  }
}

int CVideoCapture::OnCamera(const void *vir_addr,
                            unsigned int phy_addr,
                            int width, int height) {
  if (vdo_code_) {
    // LOG_INFO("camera 0x%x, %d %d.", vir_addr, width, height);
    vdo_code_->EncodeFrame((char*)vir_addr, width, height, video_callback, this);
  }
  return 0;
}

int CVideoCapture::OnVideo(VencOutputBuffer *output_buffer) {
  if (NULL == output_buffer) {
    return -1;
  }

  //LOG(L_INFO) << "output_buffer flag " << output_buffer->nFlag;
  static FILE *file = fopen("test.264", "wb+");
  if (file) {
    if (1 == output_buffer->nFlag) {
      fwrite(vdo_code_->GetHeadData().pBuffer, 1,
             vdo_code_->GetHeadData().nLength, file);
    }
    fwrite(output_buffer->pData0, 1, output_buffer->nSize0, file);
    if (output_buffer->nSize1) {
      fwrite(output_buffer->pData1, 1, output_buffer->nSize1, file);
    }
    fflush(file);
  }

  //fwrite(outputBuffer.pData0, 1, outputBuffer.nSize0, out_file);
  //if (outputBuffer.nSize1)
  //{
  //  fwrite(outputBuffer.pData1, 1, outputBuffer.nSize1, out_file);
  //}
  //printf("outputBuffer.nSize0 %d.\n", output_buffer->nSize0);
  //if (output_buffer->nSize1) {
  //  printf("outputBuffer.nSize1 %d.\n", output_buffer->nSize1);
  //}
  return 0;
}

void CVideoCapture::Run(vzbase::Thread* thread) {
  is_exit_ = 0;
  if (camera_) {
    camera_->GetFrame(&is_exit_, camera_callback, this);
  }
}

