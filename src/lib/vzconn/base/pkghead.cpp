/************************************************************************/
/* Author      : Sober.Peng 17-06-16
/* Description :
/************************************************************************/
#include "pkghead.h"

#include "stdafx.h"

/************************************************************************/
/* Description : �ص���������ͷ��
/* Parameters  : p_data[IN] ����
                 n_data[IN] ���ݳ���
                 n_offset[OUT] ����ͷ��ƫ��
/* Return      : 0=δ�ҵ���ͷ,>0һ��������(head+body),<0(������)δ�ҵ���ͷ
/************************************************************************/
int32 vz_head_parse(const void *p_data, uint32 n_data, uint32 *n_offset) {
  if (n_data < sizeof(NetHead)) {
    return 0;
  }

  // ������
  NetHead* p_head = (NetHead*)p_data;
  if (p_head->mark_0 == NET_MARK_0
      && p_head->mark_1 == NET_MARK_1) {
    return sizeof(NetHead) + p_head->data_size;
  }

  // ���Ұ�ͷƫ��

  return -1;
}

/************************************************************************/
/* Description : �ص�����ͷ�����
/* Parameters  : p_data[OUT] ����
                 n_data[IN]  ��ʹ�����ݳ���
/* Return      : >0��ͷռ�����ݳ���, <0������ͷ����
/************************************************************************/
int32 vz_head_packet(void *p_data, uint32 n_data, uint32 n_body, const void *p_param) {
  if (n_data < sizeof(NetHead)) {
    return -1;
  }

  // ���ͷ
  NetHead* p_head = (NetHead*)p_data;
  p_head->mark_0 = NET_MARK_0;
  p_head->mark_1 = NET_MARK_1;
  p_head->type_flag = *((uint16*)p_param);
  p_head->data_size = n_body;

  return sizeof(NetHead);
}
