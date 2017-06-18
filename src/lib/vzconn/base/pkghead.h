/************************************************************************/
/* Author      : Sober.Peng 17-06-16
/* Description :
/************************************************************************/
#ifndef LIBVZCONN_PKGHEAD_H_
#define LIBVZCONN_PKGHEAD_H_

#include "basictypes.h"

#define NET_MARK_0  'V'
#define NET_MARK_1  'Z'

// ���ã���ָ��ԭ�����뷽ʽ����ѹջ�������µĶ��뷽ʽ����Ϊһ���ֽڶ���
#pragma  pack (push, 1)
#if 1
typedef struct _NetHead {
  uint8   mark_0;     // 'V'
  uint8   mark_1;     // 'Z'
  uint16  type_flag;  // ���ͱ�ǩ; 0=�ַ���Ϣ,1=ע�����,2=ȡ������,3=�ͻ���ע��
  uint32  data_size;  //
  uint8   data[0];
} NetHead;
#else
typedef struct _NetHead {
  uint8   mark[4];    // 'V','Z'
  uint16  number;     // ������
  uint32  data_size;  //
  uint8   data[0];
} NetHead;
#endif
#pragma pack(pop)       // ���ã��ָ�����״̬

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

// �����ͷ����
int32 vz_head_parse(const void *p_data,
                    uint32      n_data,
                    uint32     *n_offset);
// ��������ͷ
int32 vz_head_packet(void       *p_data,
                     uint32      n_data,
                     uint32      n_body,
                     const void *p_param);

#ifdef __cplusplus
};
#endif // __cplusplus


#endif  // LIBVZCONN_PKGHEAD_H_
