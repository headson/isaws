/************************************************************************/
/* Author      : Sober.Peng 17-06-20
/* Description : �ַ����������ṹ,������.h�����ô��ļ�
/************************************************************************/
#ifndef LIBDISPATCH_PKGHEAD_H_
#define LIBDISPATCH_PKGHEAD_H_

#include <string.h>
#include "vzconn/base/connhead.h"

#define VZNETDP_SUCCEED 1
#define VZNETDP_FAILURE 0

#define MAX_METHOD_SIZE   32
#define MAX_METHOD_COUNT  64
#define MAX_DATA_SIZE     65536

#define MAX_DPMESSAGE_DATA_SIZE 1048576

const unsigned short FLAG_DISPATCHER_MESSAGE = 0;
const unsigned short FLAG_ADDLISTEN_MESSAGE = 1;
const unsigned short FLAG_REMOVELISTEN_MESSAGE = 2;
const unsigned short FLAG_GET_CLIENT_ID = 3;

enum {
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

#define MAX_CLIS_PER_PROC  6              // һ���������������
#define DEF_TIMEOUT_MSEC   1000           // Ĭ�ϳ�ʱ

typedef struct _TagDpMsg {
  unsigned char type;                     // ��Ϣ����[8bits]
  unsigned char channel_id;               // һ����0[* bits]
  unsigned char reserved;                 // [8bits]
  unsigned char method_size;              // ��Ϣ�����͵ĳ���[8 bits]
  char          method[MAX_METHOD_SIZE];  // ��Ϣ����[32*8bits]
  unsigned int  id;                       // [��Ϣ���]32 bits;8+24
  unsigned int  data_size;                // [���ݳ���]32 bits
  char          data[0];                  //  * bits
} DpMessage;

#endif  // LIBDISPATCH_PKGHEAD_H_
