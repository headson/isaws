/****************************
��Ȩ����,����Ѷ��˾
�ļ���: define.h
����:   �ܽ���
�汾:   1.0.0
�������:   2011/7/7
����:       �������͵Ķ���
����:

�޸���ʷ��¼�б�:
<�޸���>   <�޸�����>  <�汾>    <����>
  
***************************/
#ifndef _DEFINE_H_
#define _DEFINE_H_

#include <stdlib.h>

#ifdef WIN32
#include <windows.h>

//�������������ض���
typedef   signed char   int8_t;
typedef unsigned char   uint8_t;
typedef   signed short  int16_t;
typedef unsigned short  uint16_t;
typedef   signed int    int32_t;
typedef unsigned int    uint32_t;
#endif // WIN32

#define restrict

#define MIN2(a, b)  ((a) < (b) ? (a) : (b))
#define MAX2(a, b)  ((a) > (b) ? (a) : (b))

#define MAX3(a, b, c) ((a) > (b) ? MAX2(a, c) : MAX2(b, c))
#define MIN3(a, b, c) ((a) < (b) ? MIN2(a, c) : MIN2(b, c))

//�������
typedef struct TAG_RECT
{
    int16_t x0; // ������߽�x����
    int16_t x1; // �����ұ߽�x����
    int16_t y0; // �����ϱ߽�y����
    int16_t y1; // �����±߽�y����
}Rects;

//�����
typedef struct TAG_POINT
{
    int16_t x;  //x����
    int16_t y;  //y����
}Point;

//�����
typedef struct TAG_FPOINT
{
    float x;  //x����
    float y;  //y����
}FPoint;

#ifdef WIN32
#define IVA_SLEEP(n) Sleep(n)
#else
#define IVA_SLEEP(n) usleep(n * 1000)
#endif

#endif
