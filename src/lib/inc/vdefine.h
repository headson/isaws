/************************************************************************/
/* ����: SoberPeng 17-05-23
/* ����: 
/************************************************************************/
#ifndef _VDEFINE_H
#define _VDEFINE_H

#include <stdio.h>

#include "verror.h"
#include "vmessage.h"

#define VZ_PRINT(...)                             \
  do {                                            \
    printf("[%s]-%d: ", __FUNCTION__, __LINE__);  \
    printf(__VA_ARGS__);                          \
  } while (0)

typedef enum YH_RESULT {
  RET_SUCCESS             = 0,          // �ɹ�
  RET_UNKNOWN_ERROR       = -1,         // δ֪����
  RET_SERVICE_UNABLE      = -2,         // �����ݲ�����
  RET_UNKNOWN_METHOD      = -3,         // δ֪�ķ���
};

#endif  // _VDEFINE_H