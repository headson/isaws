/****************************
版权所有,百威讯公司
文件名: define.h
作者:   周建雄
版本:   1.0.0
完成日期:   2011/7/7
描述:       数据类型的定义
其它:

修改历史记录列表:
<修改者>   <修改日期>  <版本>    <描述>
  
***************************/
#ifndef _DEFINE_H_
#define _DEFINE_H_

#include <stdlib.h>

#ifdef WIN32
#include <windows.h>

//常用数据类型重定义
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

//定义矩形
typedef struct TAG_RECT
{
    int16_t x0; // 矩形左边界x坐标
    int16_t x1; // 矩形右边界x坐标
    int16_t y0; // 矩形上边界y坐标
    int16_t y1; // 矩形下边界y坐标
}Rects;

//定义点
typedef struct TAG_POINT
{
    int16_t x;  //x坐标
    int16_t y;  //y坐标
}Point;

//定义点
typedef struct TAG_FPOINT
{
    float x;  //x坐标
    float y;  //y坐标
}FPoint;

#ifdef WIN32
#define IVA_SLEEP(n) Sleep(n)
#else
#define IVA_SLEEP(n) usleep(n * 1000)
#endif

#endif
