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
#pragma once

#include <stdio.h>
#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

// OSD叠加
// yuv420[IN]YUV数据
// img_w[IN] 图像宽
// img_h[IN] 图像高
// info[IN]  叠加字符串[目前支持数据和字母等字符]
// scale[IN] 字符放大倍数
// zk[IN]    字符数组
// x[IN]     图像左距离
// y[IN]     图像上距离
// 返回：=1 成功，<0 失败
extern int yuv_osd(int color, unsigned char *pImg, unsigned int img_w, unsigned int img_h,
                   char *info, unsigned int scale, unsigned char zk[], unsigned int x, unsigned int y);

extern void YUV1024x768toD1(unsigned char *pImg, unsigned int nScale);
extern void ImageResizeNN(unsigned char *pImg, int src_w, int src_h, int dst_w, int dst_h);

extern void image_transpose(unsigned char *in_yuv420, unsigned char *tmp_yuv420, int image_width, int image_height);

extern void get_image_average_intensity(unsigned char *yuv420, int img_w, int img_h, unsigned int *average_intensity, float *light_coef);

#ifdef __cplusplus
}
#endif
