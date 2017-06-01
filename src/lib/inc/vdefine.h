/************************************************************************/
/* 作者: SoberPeng 17-05-23
/* 描述: 
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
  RET_SUCCESS             = 0,          // 成功
  RET_UNKNOWN_ERROR       = -1,         // 未知错误
  RET_SERVICE_UNABLE      = -2,         // 服务暂不可用
  RET_UNKNOWN_METHOD      = -3,         // 未知的方法
};

#endif  // _VDEFINE_H