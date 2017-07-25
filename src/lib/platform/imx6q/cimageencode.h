/************************************************************************/
/* ����: SoberPeng 17-05-23
/* ����: 
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
    int Encode(char* pDst, int nDst, char* pSrc, int nw, int nH);

private:
    bool                bInited;  // ��ʼ��

    CVpu                vpu_;     // 
    CVpu::TAG_VPU_SRC   vpu_src_;  // 
};

#endif  // LIBPLATFORM_CPHOTOENCODE_H
