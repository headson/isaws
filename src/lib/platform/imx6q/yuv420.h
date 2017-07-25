/*****************************************************************************
* Copyright (c) 2011,ChengDu ThinkWatch Company
* All rights reserved.
*-----------------------------------------------------------------------------
* �ļ�����: yuv420.h
* ��Ҫ˵��: OSD�����ַ���YUV����
* �������:
* ��д����: Sober.Peng
* �������: 2014-5-21
*-----------------------------------------------------------------------------
* �޶�����:
* �޶�ʱ��:
*-----------------------------------------------------------------------------
******************************************************************************/
#pragma once

#include <stdio.h>
#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

// OSD����
// yuv420[IN]YUV����
// img_w[IN] ͼ���
// img_h[IN] ͼ���
// info[IN]  �����ַ���[Ŀǰ֧�����ݺ���ĸ���ַ�]
// scale[IN] �ַ��Ŵ���
// zk[IN]    �ַ�����
// x[IN]     ͼ�������
// y[IN]     ͼ���Ͼ���
// ���أ�=1 �ɹ���<0 ʧ��
extern int yuv_osd(int color, unsigned char *pImg, unsigned int img_w, unsigned int img_h,
                   char *info, unsigned int scale, unsigned char zk[], unsigned int x, unsigned int y);

extern void YUV1024x768toD1(unsigned char *pImg, unsigned int nScale);
extern void ImageResizeNN(unsigned char *pImg, int src_w, int src_h, int dst_w, int dst_h);

extern void image_transpose(unsigned char *in_yuv420, unsigned char *tmp_yuv420, int image_width, int image_height);

extern void get_image_average_intensity(unsigned char *yuv420, int img_w, int img_h, unsigned int *average_intensity, float *light_coef);

#ifdef __cplusplus
}
#endif
