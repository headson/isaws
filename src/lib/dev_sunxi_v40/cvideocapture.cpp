/************************************************************************
*Author      : Sober.Peng 17-10-17
*Description :
************************************************************************/
#include "cvideocapture.h"

#include "vzbase/helper/stdafx.h"

#define WIDTH   720
#define HEIGHT  576

CVideoCapture::CVideoCapture()
  : thread_(NULL)
  , is_exit_(0)
  , camera_(NULL)
  , vdo_code_(NULL)
  , shm_vdo_ptr_(NULL)
  , shm_img_ptr_(NULL) {
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

int CVideoCapture::Start(int dev_num) {
  TAG_SHM_ARG &shm_img = g_shm_img[dev_num];
  TAG_SHM_ARG &shm_vdo = g_shm_avdo[dev_num];
  LOG_INFO("video %s, w %d, h %d, size %d.\n",
           shm_vdo.dev_name,
           shm_vdo.width, shm_vdo.height,
           shm_vdo.shm_size);
  LOG_INFO("image %s, w %d, h %d, size %d.\n",
           shm_img.dev_name,
           shm_img.width, shm_img.height,
           shm_img.shm_size);

  bool res = false;
  //
  res = shm_img_.Open(shm_img.dev_name, shm_img.shm_size);
  if (false == res) {
    LOG(L_ERROR) << "open share image failed.";
    return -1;
  }
  shm_img_ptr_ = (TAG_SHM_IMG*)shm_img_.GetData();

  res = shm_vdo_.Open(shm_vdo.dev_name, shm_vdo.shm_size);
  if (false == res) {
    LOG(L_ERROR) << "open share video failed.";
    return -1;
  }
  shm_vdo_ptr_ = (TAG_SHM_VDO*)shm_vdo_.GetData();

  //
  vdo_code_ = CVideoCodec::Create(VENC_CODEC_H264,
                                  shm_img.width, shm_img.height,
                                  shm_vdo.width, shm_vdo.height);
  if (NULL == vdo_code_) {
    LOG(L_ERROR) << "CVideoCodec create failed.";
    return -1;
  }

  //
  int ret = vdo_code_->EncodeInit();
  if (0 != ret) {
    LOG(L_ERROR) << "EncodeInit failed.";
    return -1;
  }
  SetSpsPps((char*)vdo_code_->GetHeadData().pBuffer,
            vdo_code_->GetHeadData().nLength);

  char dev_name[32] = {0};
  snprintf(dev_name, 31, "/dev/video%d", dev_num);
  camera_ = CCamera::Create(dev_name,
                            shm_img.width, shm_img.height);
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

  shm_vdo_ptr_ = NULL;
  shm_img_ptr_ = NULL;
}

int CVideoCapture::OnCamera(const void *vir_addr,
                            unsigned int phy_addr,
                            int width, int height) {
  if (vdo_code_) {
    // LOG_INFO("camera 0x%x, %d %d.", vir_addr, width, height);
    if (shm_img_ptr_) {
      struct timeval tv;
      gettimeofday(&tv, NULL);
      shm_img_ptr_->wsec = tv.tv_sec;
      shm_img_ptr_->wusec = tv.tv_usec;

      shm_img_ptr_->ndata = width*height * 3 / 2;
      memcpy(shm_img_ptr_->pdata, vir_addr, shm_img_ptr_->ndata);
    }

    vdo_code_->EncodeFrame((char*)vir_addr, width, height, video_callback, this);
  }
  return 0;
}

int CVideoCapture::OnVideo(VencOutputBuffer *output_buffer) {
  if (NULL == output_buffer) {
    return -1;
  }

  //LOG(L_INFO) << "output_buffer flag " << output_buffer->nFlag;
  //static FILE *file = fopen("test.264", "wb+");
  //if (file) {
  //  if (1 == output_buffer->nFlag) {
  //    fwrite(vdo_code_->GetHeadData().pBuffer, 1,
  //           vdo_code_->GetHeadData().nLength, file);
  //  }
  //  fwrite(output_buffer->pData0, 1, output_buffer->nSize0, file);
  //  if (output_buffer->nSize1) {
  //    fwrite(output_buffer->pData1, 1, output_buffer->nSize1, file);
  //  }
  //  fflush(file);
  //}
  if (shm_vdo_ptr_) {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    shm_vdo_ptr_->wsec = tv.tv_sec;
    shm_vdo_ptr_->wusec = tv.tv_usec;

    memcpy(shm_vdo_ptr_->pdata,
           output_buffer->pData0, output_buffer->nSize0);
    shm_vdo_ptr_->ndata = output_buffer->nSize0;
    if (output_buffer->nSize1) {
      memcpy(shm_vdo_ptr_->pdata+shm_vdo_ptr_->ndata,
             output_buffer->pData1, output_buffer->nSize1);
      shm_vdo_ptr_->ndata += output_buffer->nSize0;
    }
  }
  return 0;
}

int CVideoCapture::SetSpsPps(const char *sps_pps, int sps_pps_size) {
  if (shm_vdo_ptr_) {
    shm_vdo_ptr_->nhead = sps_pps_size;
    memcpy(shm_vdo_ptr_->shead, sps_pps, sps_pps_size);
    return sps_pps_size;
  }
  return 0;
}

void CVideoCapture::Run(vzbase::Thread* thread) {
  is_exit_ = 0;
  if (camera_) {
    camera_->GetFrame(&is_exit_, camera_callback, this);
  }
}

