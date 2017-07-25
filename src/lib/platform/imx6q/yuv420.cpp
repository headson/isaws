/*****************************************************************************
* Copyright (c) 2011,ChengDu ThinkWatch Company
* All rights reserved.
*-----------------------------------------------------------------------------
* 文件名称: yuv420.h
* 简要说明: OSD叠加字符，YUV缩放
* 相关描述:
* 编写作者: Sober.Peng
* 完成日期: 2014-5-21
*-----------------------------------------------------------------------------
* 修订作者:
* 修订时间:
*-----------------------------------------------------------------------------
******************************************************************************/
#include "yuv420.h"
#include <string.h>

// OSD叠加
// yuv422[IN]YUV数据
// img_w[IN] 图像宽
// img_h[IN] 图像高
// info[IN]  叠加字符串[目前支持数据和字母等字符]
// scale[IN] 字符放大倍数
// zk[IN]    字符数组
// x[IN]     图像左距离
// y[IN]     图像上距离
// 返回：=1 成功，<0 失败
int yuv_osd(int nColor, unsigned char *yuv420, unsigned int img_w, unsigned int img_h,
            char *info, unsigned int scale, unsigned char zk[], unsigned int x, unsigned int y) {
  unsigned int n;
  unsigned int i, j;
  unsigned int x0, y0;
  unsigned int x1, y1;
  unsigned int x2, y2;
  unsigned int nchs;
  unsigned int valid_nchs;
  //unsigned char yy, uv;
  unsigned char *p_zk;
  unsigned char bit;
  const unsigned int CH_W = 8;

  if(yuv420 == NULL) {
    return 0;
  }
  if(info == NULL) {
    return 0;
  }
  if(scale == 0) {
    return 0;
  }
  if(scale > 4) {
    return 0;
  }
  if(zk == NULL) {
    return 0;
  }

  y0 = y;
  if(x % 2 != 0) {
    x0 = x + 1;
  } else {
    x0 = x;
  }
  if(y + scale * CH_W >= img_h) {
    return 0;
  }
  nchs = 0;
  for(n = 0; n < 100; n++) {
    if(info[n] == 0) {
      break;
    }
    nchs++;
  }
  if(x + nchs * scale * CH_W >= img_w) {
    valid_nchs = (img_w - x) / (scale * CH_W);
  } else {
    valid_nchs = nchs;
  }

  for(n = 0; n < valid_nchs; n++) {
    p_zk = zk + info[n] * 8;
    for(i = 0; i < scale * CH_W; i++) {
      for(j = 0; j < scale * CH_W; j++) {
        y1 = y0 + i;
        x1 = x0 + n * scale * CH_W + j;
        x2 = j / scale;
        y2 = i / scale;
        bit = p_zk[y2 + (x2 / 8)] & (0x80 >> (x2 % 8));
        if(bit > 0) {
          /*if (yuv420[y1 * img_w + x1 + 0] > 128)
          yuv420[y1 * img_w + x1 + 0] = 0;
          else
          yuv420[y1 * img_w + x1 + 0] = 255;*/
          yuv420[y1 * img_w + x1 + 0] = nColor;
        }
      }
    }
  }
  return 1;
}

void YUV1024x768toD1(unsigned char *srcYUV, unsigned int mountHeight) {
  int i, j;
  int inWidth, inHeight;
  int xOffset, yOffset;
  unsigned int kx, ky;
  unsigned int x16, y16;
  unsigned char *pSrc = NULL;
  unsigned char *pDst = NULL;
  float zoomScale;

  if(mountHeight < 230) {
    mountHeight = 230;
  }
  if(mountHeight > 250) {
    mountHeight = 250;
  }

  zoomScale = 1.0f + (mountHeight - 230) * (1.33f - 1.0f) / (250 - 230);
  inHeight = (int)(768 / zoomScale);
  inWidth = inHeight * 720 / 576;

  xOffset = (1024 - inWidth) / 2;
  yOffset = (768 - inHeight) / 2;
  kx = (inWidth << 16) / 720;
  ky = (inHeight << 16) / 576;

  y16 = 0;
  pDst = srcYUV;
  for(i = 0; i < 576; i++) {
    x16 = 0;
    pSrc = srcYUV + 1024 * ((y16 >> 16) + yOffset) + xOffset;
    for(j = 0; j < 720; j++) {
      pDst[j] = pSrc[(x16 >> 16)];
      x16 += kx;
    }
    y16 += ky;
    pDst += 720;
  }
}

void ImageResizeNN(unsigned char *srcYUV, int src_w, int src_h, int dst_w, int dst_h) {
  int i, j;
  const unsigned char *p_src = NULL;
  unsigned char *p_dst = NULL;
  unsigned int x16, y16;
  unsigned int kx, ky;

  kx = (src_w << 16) / dst_w;
  ky = (src_h << 16) / dst_h;

  y16 = 0;
  p_dst = srcYUV;
  for(i = 0; i < dst_h; i++) {
    x16 = 0;
    p_src = srcYUV + src_w * (y16 >> 16);
    for(j = 0; j < dst_w; j++) {
      p_dst[j] = p_src[(x16 >> 16)];
      x16 += kx;
    }
    y16 += ky;
    p_dst += dst_w;
  }
}

void get_image_average_intensity(unsigned char *yuv420, int img_w, int img_h, unsigned int *average_intensity, float *light_coef) {
  int i, j;
  int sum;
  int num_pixels;
  float average;
  static float average_intensity_ = 128;

  sum = 0;
  num_pixels = 0;
  for(i = img_h / 2; i < img_h; i+=2) {
    for(j = img_w / 4; j < img_w * 3 / 4; j+=2) {
      sum += yuv420[i * img_w + j];
      num_pixels++;
    }
  }
  average = sum / (float)(num_pixels + 1);
  average_intensity_ = average_intensity_ * 0.99f + average * 0.01f;
  *average_intensity = (int)(average_intensity_);
  if(average_intensity_ > 80) {
    *light_coef = 0.13f;
  } else if(average_intensity_ < 50) {
    *light_coef = 1.0f;
  } else {
    *light_coef = 1.0f - (average_intensity_ - 50) * 0.87f / (80 - 50);
  }
}

//////////////////////////////////////////////////////////////////////////
static void image_transpose_one_channel(unsigned char *in_buf, unsigned char *tmp_buf, int image_width, int image_height) {
  int i, j;
  int offset;
  unsigned char *p_src = NULL;
  unsigned char *p_dst = NULL;
  unsigned int  x16, y16;
  unsigned int  kx, ky;
  int resize_width, resize_height;
  int total_size = image_width * image_height;

  resize_width = image_height;
  resize_height = image_height * resize_width / image_width;

  //x方向缩放系数
  kx = (image_width << 16) / resize_width;

  //y方向缩放系数
  ky = (image_height << 16) / resize_height;

  y16 = 0;
  p_dst = tmp_buf;
  for(i = 0; i < resize_height; i++) {
    x16 = 0;
    p_src = in_buf + image_width * (y16 >> 16);
    for(j = 0; j < resize_width; j++) {
      p_dst[j] = p_src[(x16 >> 16)];
      x16 += kx;
    }
    y16 += ky;
    p_dst += image_width;
  }

  offset = (image_width - resize_height) / 2;
  memset(in_buf, 128, total_size);
  for(i = 0; i < resize_height; i++) {
    for(j = 0; j < resize_width; j++) {
      in_buf[j * image_width + i + offset] = tmp_buf[i * image_width + j];
    }
  }
}

void image_transpose(unsigned char *in_yuv420, unsigned char *tmp_yuv420, int image_width, int image_height) {
  image_transpose_one_channel(in_yuv420, tmp_yuv420, image_width, image_height);
  image_transpose_one_channel(in_yuv420 + image_width * image_height, tmp_yuv420, image_width / 2, image_height / 2);
  image_transpose_one_channel(in_yuv420 + image_width * image_height * 5 / 4, tmp_yuv420, image_width / 2, image_height / 2);
}
