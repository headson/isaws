/************************************************************************/
/* Author      : Sober.Peng 17-06-20
/* Description : 分发服务器包结构,不可在.h中引用此文件
/************************************************************************/
#ifndef LIBDISPATCH_PKGHEAD_H_
#define LIBDISPATCH_PKGHEAD_H_

#include "vzconn/base/connhead.h"

#define VZNETDP_SUCCEED 1
#define VZNETDP_FAILURE 0

#define MAX_METHOD_SIZE   32
#define MAX_METHOD_COUNT  64
#define MAX_DATA_SIZE     65536

#define MAX_DPMESSAGE_DATA_SIZE 1048576

enum {
  TYPE_MESSAGE = 0,
  TYPE_REQUEST = 1,
  TYPE_REPLY = 2,
  TYPE_ERROR_TIMEOUT = 3,
  TYPE_ERROR_FORMAT = 4
};

const uint16 FLAG_DISPATCHER_MESSAGE    = 0;
const uint16 FLAG_ADDLISTEN_MESSAGE     = 1;
const uint16 FLAG_REMOVELISTEN_MESSAGE  = 2;
const uint16 FLAG_GET_CLIENT_ID         = 3;

// 一个进程最多线程数
#define MAX_CLIS_PER_PROC  6

typedef struct _TagDpMsg {
  unsigned char type;           // 消息类型[8bits]
  unsigned char channel_id;     // 一般是0[* bits]
  unsigned char reserved;       // [8bits]
  unsigned char method_size;    // 消息的类型的长度[8 bits]
  char          method[32];     // 消息类型[32*8bits]
  unsigned int  id;             // [消息序号]32 bits
  unsigned int  data_size;      // [数据长度]32 bits
  char          data[0];        //  * bits
} TagDpMsg;

#ifdef __cplusplus
extern "C" {
#endif

inline int EncDpMsg(TagDpMsg       *p_msg,
                    unsigned char   n_type,
                    unsigned char   n_chn_id,
                    const char     *method,
                    unsigned char   n_msg_id,
                    int             data_size) {
  if (!p_msg || !method) {
    return -1;
  }
  memset(p_msg, 0, sizeof(TagDpMsg));

  p_msg->type         = (unsigned char)n_type;
  p_msg->channel_id   = (unsigned char)n_chn_id;
  p_msg->reserved     = (unsigned char)0;
  p_msg->method_size  = (unsigned char)strlen(method);
  strncpy(p_msg->method, method, 31);
  p_msg->id = (vzconn::VZ_ORDER_BYTE == vzconn::ORDER_NETWORK)
              ? vzconn::HostToNetwork32(n_chn_id) : n_chn_id;
  if (data_size > MAX_DPMESSAGE_DATA_SIZE) {
    return -2;
  }
  p_msg->data_size = (vzconn::VZ_ORDER_BYTE == vzconn::ORDER_NETWORK)
                     ? vzconn::HostToNetwork32(data_size) : data_size;
  return sizeof(TagDpMsg);
}

inline int DecDpMsg(TagDpMsg      *p_msg,
                    const void    *p_data,
                    uint32         n_data) {
  if (!p_msg || !p_data) {
    return -1;
  }

  if (n_data >= sizeof(TagDpMsg)) {
    memcpy(p_msg, p_data, sizeof(TagDpMsg));
    p_msg->id = (vzconn::VZ_ORDER_BYTE == vzconn::ORDER_NETWORK)
                ? vzconn::NetworkToHost32(p_msg->id) : p_msg->id;
    p_msg->data_size = (vzconn::VZ_ORDER_BYTE == vzconn::ORDER_NETWORK)
                       ? vzconn::NetworkToHost32(p_msg->data_size) : p_msg->data_size;
    return sizeof(TagDpMsg);
  }
  return -2;
}

#ifdef __cplusplus
}
#endif


#endif  // LIBDISPATCH_PKGHEAD_H_
