/************************************************************************
*Author      : Sober.Peng 17-06-27
*Description : dispatch\kvdb包结构
************************************************************************/
#ifndef LIBDISPATCH_PKGHEAD_H_
#define LIBDISPATCH_PKGHEAD_H_

#include <string.h>
#include "vzconn/base/connhead.h"

#define VZNETDP_SUCCEED 1
#define VZNETDP_FAILURE 0

#define MAX_METHOD_SIZE   64
#define MAX_METHOD_COUNT  64
#define MAX_DATA_SIZE     65536

#define MAX_DPMESSAGE_DATA_SIZE 1048576

const unsigned short FLAG_DISPATCHER_MESSAGE = 0;
const unsigned short FLAG_ADDLISTEN_MESSAGE = 1;
const unsigned short FLAG_REMOVELISTEN_MESSAGE = 2;
const unsigned short FLAG_GET_CLIENT_ID = 3;

enum {
  TYPE_INVALID = -1,            // 非法TYPE
  TYPE_MESSAGE = 0,
  TYPE_REQUEST = 1,
  TYPE_REPLY = 2,
  TYPE_ERROR_TIMEOUT = 3,
  TYPE_ERROR_FORMAT = 4,
  TYPE_GET_SESSION_ID = 5,
  TYPE_SUCCEED = 6,
  TYPE_FAILURE = 7,
  TYPE_ADD_MESSAGE = 8,
  TYPE_REMOVE_MESSAGE = 9,
};

#define MAX_CLIS_PER_PROC  6              // 一个进程最多链接数
#define DEF_TIMEOUT_MSEC   1000           // 默认超时 1S

typedef struct _TagDpMsg {
  unsigned char type;                     // 消息类型[8bits]
  unsigned char channel_id;               // 一般是0[* bits]
  unsigned char reply_type;               // 返回时存type[8bits]
  unsigned char method_size;              // 消息的类型的长度[8 bits]
  char          method[MAX_METHOD_SIZE];  // 消息类型[32*8bits]
  unsigned int  id;                       // [消息序号]32 bits;8+24
  unsigned int  data_size;                // [数据长度]32 bits
  unsigned int  reserved02;               // [保留] 对齐48bits
  char          data[0];                  //  * bits
} DpMessage;

////////////////////////////////////////////////////////////////////////////////

enum {
  KVDB_INVALID = -1,            // 非法KVDB
  KVDB_REPLACE = 0,
  KVDB_DELETE = 1,
  KVDB_SELECT = 2,

  KVDB_BACKUP = 3,
  KVDB_RESTORE = 4,

  KVDB_SUCCEED = 5,
  KVDB_FAILURE = 6,
};

#define MAX_KVDB_KEY_SIZE 32

typedef struct _KvdbMsg {
  unsigned int type;            // 请求类型;回执错误代码
  unsigned int id;              // 
  unsigned int key_size;        // key 长度
  char key[MAX_KVDB_KEY_SIZE];
  char value[0];
} KvdbMessage;

#endif  // LIBDISPATCH_PKGHEAD_H_
