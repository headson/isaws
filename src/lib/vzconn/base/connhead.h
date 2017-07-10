/************************************************************************/
/* Author      : Sober.Peng 17-06-16                                    */
/* Description : �����ͷ�ṹ                                            */
/************************************************************************/
#ifndef LIBVZCONN_PKGHEAD_H_
#define LIBVZCONN_PKGHEAD_H_

#include "vzbase/base/basictypes.h"
#include "byteorder.h"

namespace vzconn {

#define NET_MARK_0  'V'
#define NET_MARK_1  'Z'

// ���ã���ָ��ԭ�����뷽ʽ����ѹջ�������µĶ��뷽ʽ����Ϊһ���ֽڶ���
#pragma  pack (push, 1)
typedef struct _NetHead {
  unsigned char   mark[2];    // [0]='V' [1]='Z'
  unsigned short  type_flag;  // ���ͱ�ǩ; 0=�ַ���Ϣ,1=ע�����,2=ȡ������,3=�ͻ���ע��
  unsigned int  data_size;  //
} NetHead;
#pragma pack(pop)     // ���ã��ָ�����״̬

//// �����ͱ�ǩ
//const unsigned short FLAG_DISPATCHER_MESSAGE    = 0;
//const unsigned short FLAG_ADDLISTEN_MESSAGE     = 1;
//const unsigned short FLAG_REMOVELISTEN_MESSAGE  = 2;
//const unsigned short FLAG_GET_CLIENT_ID         = 3;

  // Ĭ���ַ�˳��[���]
  const ByteOrder VZ_ORDER_BYTE       = ORDER_NETWORK;

  // ��Ĭ�ϴ�С
  #define DEF_BUFFER_SIZE (4096)
  // �����ߴ�
  #define MAX_BUFFER_SIZE (2 * 1024 * 1024)
}  // namespace vzconn

#endif  // LIBVZCONN_PKGHEAD_H_
