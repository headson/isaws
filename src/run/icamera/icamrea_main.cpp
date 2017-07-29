#include <stdio.h>
#include <string.h>

#include <time.h>
#include <winsock2.h>

#include "systemv/shm/vzshm_c.h"
#include "systemv/flvmux/cflvmux.h"

#include "ch264code.h"
#include "camera/icameracaptuer.h"

#include "vzlogging/logging/vzloggingcpp.h"

#pragma comment(lib, "x264/libs/32bit/libx264.lib")

#define VIDEO_WIDTH   SHM_IMAGE_0_W
#define VIDEO_HEIGHT  SHM_IMAGE_0_H

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
  
  CShareBuffer shm_vdo;
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
  }

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

  shm_vdo.SetWidth(VIDEO_WIDTH);
  shm_vdo.SetHeight(VIDEO_HEIGHT);

  while (true) {
    Sleep(30);

    struct timeval tv;
    gettimeofday(&tv, NULL);

    // 获取最新视频帧,YUV
    unsigned char *p_img = p_cam->QueryFrame();
    if (p_img) {
      // 可以优化为m_pic中保存一个指针,直接执行szYUVFrame
      memcpy(pic_in.img.plane[0], p_img, VIDEO_WIDTH * VIDEO_HEIGHT * 3 / 2);
      shm_img.Write((char*)p_img, 
                    VIDEO_WIDTH * VIDEO_HEIGHT * 3 / 2,
                    tv.tv_sec, tv.tv_usec);

      int iNal = 0;
      x264_nal_t* pNals = NULL;
      //pic_in.i_type = X264_TYPE_IDR;
      int ret = x264_encoder_encode(x264, &pNals, &iNal, &pic_in, &pic_out);

      int n_data = 0;
      for (int j = 0; j < iNal; ++j) {
        x264_nal_t &nal = pNals[j];

        if (nal.i_type == NAL_SPS) {
          shm_vdo.WriteSps((char*)nal.p_payload, nal.i_payload);
          continue;
        } else if (nal.i_type == NAL_PPS) {
          shm_vdo.WritePps((char*)nal.p_payload, nal.i_payload);
          continue;
        } else if (nal.i_type == NAL_SLICE_IDR
                   || nal.i_type == NAL_SLICE) {
          shm_vdo.Write((char*)nal.p_payload, nal.i_payload, 
                        tv.tv_sec, tv.tv_usec);
          continue;
        }
      }
    }
  }
  return 0;
}
