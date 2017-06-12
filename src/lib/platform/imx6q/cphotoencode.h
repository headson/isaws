/************************************************************************/
/* ����: SoberPeng 17-05-23
/* ����: 
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
    ��������: ��Ƭ����
    ��������: pDst[OUT] ���������
              nDst[IN]  ��������ݴ�С
              pSrc[IN]  ����ǰͼ������
              nW[IN]    ����ǰͼ���
              nH[IN]    ����ǰͼ���
    ����˵��: �����ͼ�����ݴ�С
    ��д����: Sober
    ���ʱ��: 2014-9-9 14:01:54
    =============================================================================*/
    int32_t Encode(int8_t* pDst, int32_t nDst, int8_t* pSrc, int32_t nW, int32_t nH);

private:
    volatile bool       m_bInited;  // ��ʼ��

    CVpu                m_cVpu;     // 
    CVpu::TAG_VPU_SRC   m_cVpuSrc;  // 
};

#endif  // LIBPLATFORM_CPHOTOENCODE_H
