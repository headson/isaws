/************************************************************************/
/* Author      : Sober.Peng 17-06-16                                    */
/* Description : �����ͷ�ṹ                                            */
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

// ���ã���ָ��ԭ�����뷽ʽ����ѹջ�������µĶ��뷽ʽ����Ϊһ���ֽڶ���
#pragma  pack (push, 1)
typedef struct _NetHead {
  uint8   mark[2];    // [0]='V' [1]='Z'
  uint16  type_flag;  // ���ͱ�ǩ; 0=�ַ���Ϣ,1=ע�����,2=ȡ������,3=�ͻ���ע��
  uint32  data_size;  //
} NetHead;
#pragma pack(pop)     // ���ã��ָ�����״̬

// Ĭ���ַ�˳��[���]
const int32 VZ_ORDER_BYTE = 0;  // 0=ORDER_NETWORK;1=ORDER_HOST

// ��Ĭ�ϴ�С
#define SOCK_DEF_BUFFER_SIZE (4096)
// �����ߴ�
#define SOCK_MAX_BUFFER_SIZE (2 * 1024 * 1024)
#ifdef __cplusplus
}  // namespace vzconn
#endif

#endif  // LIBVZCONN_PKGHEAD_H_
