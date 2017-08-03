#ifndef IHS_GPIO_H
#define IHS_GPIO_H

#define IHS_GPIO 1
#define DEVICE_NAME "ihs_gpio"

typedef enum {
    E_WRITE         = 100,      // д

    E_WriteLED1     = 101,      // �㷨��
    E_WriteLED2,                // �㷨��
    E_WriteLED3,                // �㷨��

    E_WWatchdog     = 106,      // ι��

    E_ZhaoMing      = 110,      // ������
    E_Infrared,                 // ���ⲹ���

    E_WPilotLamp    = 120,      // ָʾ�ƣ�1=�أ�0=��
    E_WriteSwitch1,             // �Ž�1���̵�������1=����0=��(��̬)
    E_WriteSwitch2,             // �Ž�2

    E_READ          = 200,      // 

    E_ReadCover     = 203,      // ���Ӵ�
    E_ReadShake,                // �𶯿���

    E_RDoorbell,                // ���忪��

    E_ReadSwitch1,              // �Ž�1���أ�1=����0=��(��̬)
    E_ReadSwitch2,              // �Ž�2

    
}E_GPIO_NAME;

#endif // IHS_GPIO_H
