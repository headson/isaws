/************************************************************************/
/* 作者: SoberPeng 17-05-23
/* 描述: 
/************************************************************************/
#ifndef LIBPLATFORM_CPHOTOENCODE_H
#define LIBPLATFORM_CPHOTOENCODE_H

#include "vpu.h"

class CImageEncode
{
public:
    CImageEncode(int eCodec = STD_MJPG);
    virtual ~CImageEncode();

    /*============================================================================
    函数功能: 照片编码
    参数描述: pDst[OUT] 编码后数据
              nDst[IN]  编码后数据大小
              pSrc[IN]  编码前图像数据
              nW[IN]    编码前图像宽
              nH[IN]    编码前图像高
    返回说明: 编码后图像数据大小
    编写作者: Sober
    完成时间: 2014-9-9 14:01:54
    =============================================================================*/
    int Encode(char* pDst, int nDst, char* pSrc, int nw, int nH);

private:
    bool                bInited;  // 初始化

    CVpu                vpu_;     // 
    CVpu::TAG_VPU_SRC   vpu_src_;  // 
};

#endif  // LIBPLATFORM_CPHOTOENCODE_H
