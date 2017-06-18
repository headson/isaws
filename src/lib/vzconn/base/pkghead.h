/************************************************************************/
/* Author      : Sober.Peng 17-06-16
/* Description :
/************************************************************************/
#ifndef LIBVZCONN_PKGHEAD_H_
#define LIBVZCONN_PKGHEAD_H_

#include "basictypes.h"

#define NET_MARK_0  'V'
#define NET_MARK_1  'Z'

// 作用：是指把原来对齐方式设置压栈，并设新的对齐方式设置为一个字节对齐
#pragma  pack (push, 1)
#if 1
typedef struct _NetHead {
  uint8   mark_0;     // 'V'
  uint8   mark_1;     // 'Z'
  uint16  type_flag;  // 类型标签; 0=分发消息,1=注册监听,2=取消监听,3=客户端注册
  uint32  data_size;  //
  uint8   data[0];
} NetHead;
#else
typedef struct _NetHead {
  uint8   mark[4];    // 'V','Z'
  uint16  number;     // 包计数
  uint32  data_size;  //
  uint8   data[0];
} NetHead;
#endif
#pragma pack(pop)       // 作用：恢复对齐状态

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

// 网络包头解析
int32 vz_head_parse(const void *p_data,
                    uint32      n_data,
                    uint32     *n_offset);
// 添加网络包头
int32 vz_head_packet(void       *p_data,
                     uint32      n_data,
                     uint32      n_body,
                     const void *p_param);

#ifdef __cplusplus
};
#endif // __cplusplus


#endif  // LIBVZCONN_PKGHEAD_H_
