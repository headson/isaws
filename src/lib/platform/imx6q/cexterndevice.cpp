/*****************************************************************************
* Copyright (c) 2011,ChengDu ThinkWatch Company
* All rights reserved.
*-----------------------------------------------------------------------------
* 文件名称: cadoextern.cpp
* 简要说明: 外设模块
* 相关描述:
* 编写作者: Sober.Peng
* 完成日期: 2013-11-18
*-----------------------------------------------------------------------------
* 修订作者:
* 修订时间:
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
函数功能: 模块控制
参数描述: nCmd[IN] 控制命令
          pArg[IN/OUT] 输入输出参数
          nSize[IN] 参数长度
返回说明: 成功返回1，失败返回<0
编写作者: Sober
完成时间: 2013-11-21 11:27:36
------------------------------------------------------------------------------
修改作者: Sober 2013-11-21 11:27:36
修改备注:
=============================================================================*/
int32_t CFslExternDevice::Ioctrl(int32_t eCtrl, TAG_EXT_CTRL* pCtrl)
{
    if (m_fd <= 0) { return RET_INVALID_HDL; }
    ASSERT_RET(pCtrl != NULL, RET_INVALID_ARG);

    int32_t nRet = 0;
    switch(eCtrl)
    {
    case EXT_WRITE:     // 写
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

