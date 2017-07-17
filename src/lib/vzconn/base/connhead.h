/************************************************************************/
/* Author      : Sober.Peng 17-06-16                                    */
/* Description : 网络包头结构                                            */
/************************************************************************/
#ifndef LIBVZCONN_PKGHEAD_H_
#define LIBVZCONN_PKGHEAD_H_

#include "vzbase/base/basictypes.h"
//#include "byteorder.h"
#ifdef __cplusplus
namespace vzconn {
#endif

#define NET_MARK_0  'V'
#define NET_MARK_1  'Z'

// 作用：是指把原来对齐方式设置压栈，并设新的对齐方式设置为一个字节对齐
#pragma  pack (push, 1)
typedef struct _NetHead {
  uint8   mark[2];    // [0]='V' [1]='Z'
  uint16  type_flag;  // 类型标签; 0=分发消息,1=注册监听,2=取消监听,3=客户端注册
  uint32  data_size;  //
} NetHead;
#pragma pack(pop)     // 作用：恢复对齐状态

// 默认字符顺序[大端]
const int32 VZ_ORDER_BYTE = 0;  // 0=ORDER_NETWORK;1=ORDER_HOST

// 包默认大小
#define SOCK_DEF_BUFFER_SIZE (4096)
// 最大包尺寸
#define SOCK_MAX_BUFFER_SIZE (2 * 1024 * 1024)
#ifdef __cplusplus
}  // namespace vzconn
#endif

#endif  // LIBVZCONN_PKGHEAD_H_
