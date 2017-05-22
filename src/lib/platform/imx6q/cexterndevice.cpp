/*****************************************************************************
* Copyright (c) 2011,ChengDu ThinkWatch Company
* All rights reserved.
*-----------------------------------------------------------------------------
* �ļ�����: cadoextern.cpp
* ��Ҫ˵��: ����ģ��
* �������:
* ��д����: Sober.Peng
* �������: 2013-11-18
*-----------------------------------------------------------------------------
* �޶�����:
* �޶�ʱ��:
*-----------------------------------------------------------------------------
******************************************************************************/
#include "cexterndevice.h"

CFslExternDevice::CFslExternDevice()
    : CExternDevice()
    , m_fd(0)
{
}

CFslExternDevice::~CFslExternDevice()
{
    Stop();
}

int32_t CFslExternDevice::Start()
{
    char sDev[128] = {0};
    snprintf(sDev, 127, "/dev/%s", DEVICE_NAME);

    if (0 == m_fd)
    {
        m_fd = open(sDev, O_RDWR);
        if (m_fd < 0) {
            loge("Open extern device failed %d.", m_fd);
            return RET_FOPEN_FAILED;
        }
        return RET_SUCCESS;
    }
    return RET_SUCCESS;
}

void CFslExternDevice::Stop()
{
    if (m_fd) { close(m_fd); m_fd = 0; }
}

/*============================================================================
��������: ģ�����
��������: nCmd[IN] ��������
          pArg[IN/OUT] �����������
          nSize[IN] ��������
����˵��: �ɹ�����1��ʧ�ܷ���<0
��д����: Sober
���ʱ��: 2013-11-21 11:27:36
------------------------------------------------------------------------------
�޸�����: Sober 2013-11-21 11:27:36
�޸ı�ע:
=============================================================================*/
int32_t CFslExternDevice::Ioctrl(int32_t eCtrl, TAG_EXT_CTRL* pCtrl)
{
    if (m_fd <= 0) { return RET_INVALID_HDL; }
    ASSERT_RET(pCtrl != NULL, RET_INVALID_ARG);

    int32_t nRet = 0;
    switch(eCtrl)
    {
    case EXT_WRITE:     // д
        {
            nRet = write(m_fd, pCtrl, sizeof(TAG_EXT_CTRL));
        }
        break;

    case EXT_READ:
        {
            nRet = read(m_fd, pCtrl, sizeof(TAG_EXT_CTRL));
        }
        break;

    default:
        break;
    }
    return nRet;
}

