#ifndef IHS_GPIO_H
#define IHS_GPIO_H

#define IHS_GPIO 1
#define DEVICE_NAME "ihs_gpio"

typedef enum {
    E_WRITE         = 100,      // 写

    E_WriteLED1     = 101,      // 算法灯
    E_WriteLED2,                // 算法灯
    E_WriteLED3,                // 算法灯

    E_WWatchdog     = 106,      // 喂狗

    E_ZhaoMing      = 110,      // 照明灯
    E_Infrared,                 // 红外补光灯

    E_WPilotLamp    = 120,      // 指示灯；1=关，0=开
    E_WriteSwitch1,             // 门禁1（继电器）。1=开，0=关(常态)
    E_WriteSwitch2,             // 门禁2

    E_READ          = 200,      // 

    E_ReadCover     = 203,      // 盖子打开
    E_ReadShake,                // 震动开关

    E_RDoorbell,                // 门铃开关

    E_ReadSwitch1,              // 门禁1开关；1=开，0=关(常态)
    E_ReadSwitch2,              // 门禁2

    
}E_GPIO_NAME;

#endif // IHS_GPIO_H
