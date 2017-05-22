/************************************************************************/
/* ����: SoberPeng 17-05-23
/* ����: 
/************************************************************************/
#ifndef LIBPLATFORM_CEXTERNDEVICE_H
#define LIBPLATFORM_CEXTERNDEVICE_H
#include "vdefine.h"

#include "../../../driver/imx6q/ihs_gpio/ihs_gpio.h"

class CFslExternDevice : public CExternDevice
{
public:
    CFslExternDevice();
    virtual ~CFslExternDevice();

    virtual int32_t Start();
    virtual void    Stop();

    // ģ�����
    int32_t Ioctrl(int32_t eCtrl, TAG_EXT_CTRL* pCtrl);

private:
    int     m_fd;   // �ļ�
};

#endif  // LIBPLATFORM_CEXTERNDEVICE_H
