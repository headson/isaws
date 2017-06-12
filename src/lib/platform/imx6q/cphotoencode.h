/************************************************************************/
/* 作者: SoberPeng 17-05-23
/* 描述: 
/************************************************************************/
#ifndef LIBPLATFORM_CPHOTOENCODE_H
#define LIBPLATFORM_CPHOTOENCODE_H
#include "inc/vtypes.h"

#include "vpu.h"

class CPhotoEncode
{
public:
    CPhotoEncode(int32_t eCodec = CODE_ID_JPEG);
    virtual ~CPhotoEncode();

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
    int32_t Encode(int8_t* pDst, int32_t nDst, int8_t* pSrc, int32_t nW, int32_t nH);

private:
    volatile bool       m_bInited;  // 初始化

    CVpu                m_cVpu;     // 
    CVpu::TAG_VPU_SRC   m_cVpuSrc;  // 
};

#endif  // LIBPLATFORM_CPHOTOENCODE_H
