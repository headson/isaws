#include <stdio.h>
#include <string.h>

#include <time.h>
#include <winsock2.h>

#include "vzbase/system/vshm.h"

#include "ch264code.h"
#include "camera/icameracaptuer.h"

#include "vzlogging/logging/vzloggingcpp.h"

#pragma comment(lib, "x264/libs/32bit/libx264.lib")

#define VIDEO_WIDTH   640
#define VIDEO_HEIGHT  480

#include "asc8.h"
#include "yuv420.h"
#include "vzbase/base/timeutils.h"

inline int gettimeofday(struct timeval *tp, void *tzp) {
  time_t clock;
  struct tm tm;
  SYSTEMTIME wtm;

  GetLocalTime(&wtm);
  tm.tm_year = wtm.wYear - 1900;
  tm.tm_mon = wtm.wMonth - 1;
  tm.tm_mday = wtm.wDay;
  tm.tm_hour = wtm.wHour;
  tm.tm_min = wtm.wMinute;
  tm.tm_sec = wtm.wSecond;
  tm.tm_isdst = -1;
  clock = mktime(&tm);
  tp->tv_sec = (long)clock;
  tp->tv_usec = wtm.wMilliseconds * 1000;
  return (0);
}

int main() {
  ICameraCaptuer* p_cam = NULL;
  CH264Coder* p_h264_enc = NULL;

  p_cam = CamCaptuerMgr::GetCamCaptuer();
  if (p_cam == NULL) {
    printf("can't get camera.\n");
    return -1;
  }

  // 打开Camera
  if(!p_cam->OpenCamera(0, VIDEO_WIDTH, VIDEO_HEIGHT)) {
    printf("Can not open camera");
    return NULL;
  }

  // 初始化H264 Encode
  p_h264_enc = new CH264Coder(VIDEO_WIDTH, VIDEO_HEIGHT, 1000);
  if (p_h264_enc == NULL) {
    perror("create h264 encoder failed.\n");
    return -1;
  }

  bool ret = false;

  TAG_SHM_ARG shm_vdo = g_shm_avdo[0];
  vzbase::VShm shm_vdo_;
  ret = shm_vdo_.Open(shm_vdo.dev_name, shm_vdo.shm_size);
  if (false == ret) {
    LOG_ERROR("share video failed.");
    return -1;
  }
  TAG_SHM_VDO *shm_vdo_ptr_ = (TAG_SHM_VDO*)shm_vdo_.GetData();

  shm_vdo_ptr_->width = VIDEO_WIDTH;
  shm_vdo_ptr_->height = VIDEO_HEIGHT;

  /*CShareBuffer shm_vdo;
  ret = shm_vdo.Create(SHM_VIDEO_0, SHM_VIDEO_0_SIZE);
  if (ret == false) {
  printf("create share video failed.");
  return -1;
  }

  CShareBuffer shm_img;
  ret = shm_img.Create(SHM_IMAGE_0, SHM_IMAGE_0_SIZE);
  if (false == ret) {
  LOG(L_ERROR) << "share image create failed.";
  return -1;
  }*/

  x264_picture_t pic_in;
  x264_picture_alloc(&pic_in, X264_CSP_I420, VIDEO_WIDTH, VIDEO_HEIGHT);
  pic_in.i_type = X264_TYPE_AUTO;
  pic_in.i_qpplus1 = 0;

  x264_t *x264 = p_h264_enc->x264encoder();
  if (NULL == x264) {
    LOG(L_ERROR) << "get x264 encoder failed.";
    return -1;
  }

  x264_picture_t pic_out;
  x264_picture_init(&pic_out);

  bool bsps = false, bpps = false;

  time_t nTmOld = 0;
  unsigned char nColor = 0;
  char pc_num_osd[64] = {0};
  while (true) {
    Sleep(30);

    struct timeval tv;
    gettimeofday(&tv, NULL);

    // 获取最新视频帧,YUV
    unsigned char *p_img = p_cam->QueryFrame();
    if (p_img) {
      int ret = 0;

      // 可以优化为m_pic中保存一个指针,直接执行szYUVFrame
      /*memcpy(pic_in.img.plane[0], p_img, VIDEO_WIDTH * VIDEO_HEIGHT * 3 / 2);
      shm_img.Write((char*)p_img,
      VIDEO_WIDTH * VIDEO_HEIGHT * 3 / 2,
      tv.tv_sec, tv.tv_usec);*/

#if 1
      time_t nTmNow = time(NULL);
      if (nTmOld != nTmNow) {
        nTmOld = nTmNow;
        nColor = nColor ? 0 : 255;
      }
      yuv_osd(nColor, (unsigned char*)pic_in.img.plane[0],
              VIDEO_WIDTH, VIDEO_HEIGHT,
              (char*)vzbase::SecToString(nTmNow).c_str(),
              1, asc8, 10, 10);

      ret = sprintf(pc_num_osd,
                     "入:%d 出:%d",
                     5, 8);
      if (ret < 64) {
        pc_num_osd[ret] = '\0';
      }
      yuv_osd(nColor, (unsigned char*)pic_in.img.plane[0],
              VIDEO_WIDTH, VIDEO_HEIGHT,
              pc_num_osd,
              1, asc8, 10, VIDEO_HEIGHT-10);
#endif

      int iNal = 0;
      x264_nal_t* pNals = NULL;
      //pic_in.i_type = X264_TYPE_IDR;
      ret = x264_encoder_encode(x264, &pNals, &iNal, &pic_in, &pic_out);

      int n_data = 0;
      for (int j = 0; j < iNal; ++j) {
        x264_nal_t &nal = pNals[j];

        if (nal.i_type == NAL_SPS) {
          if (bsps == false) {
            bsps = true;
            shm_vdo_ptr_->nhead = nal.i_payload;
            memcpy(shm_vdo_ptr_->shead, (char*)nal.p_payload, nal.i_payload);
          }
          // shm_vdo.WriteSps((char*)nal.p_payload, nal.i_payload);
          continue;
        } else if (nal.i_type == NAL_PPS) {
          if (bpps == false) {
            bpps = true;
            shm_vdo_ptr_->nhead += nal.i_payload;
            memcpy(shm_vdo_ptr_->shead+shm_vdo_ptr_->nhead,
                   (char*)nal.p_payload, nal.i_payload);
          }
          // shm_vdo.WritePps((char*)nal.p_payload, nal.i_payload);
          continue;
        } else if (nal.i_type == NAL_SLICE_IDR
                   || nal.i_type == NAL_SLICE) {
          //shm_vdo.Write((char*)nal.p_payload, nal.i_payload,
          //              tv.tv_sec, tv.tv_usec);
          shm_vdo_ptr_->wsec = tv.tv_sec;
          shm_vdo_ptr_->wusec = tv.tv_usec;

          shm_vdo_ptr_->ndata = nal.i_payload;
          memcpy(shm_vdo_ptr_->pdata, (char*)nal.p_payload, nal.i_payload);
          continue;
        }
      }
    }
  }
  return 0;
}
