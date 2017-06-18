/************************************************************************/
/* Author      : Sober.Peng 17-06-16
/* Description :
/************************************************************************/
#include "pkghead.h"

#include "stdafx.h"

/************************************************************************/
/* Description : 回调解析网络头部
/* Parameters  : p_data[IN] 数据
                 n_data[IN] 数据长度
                 n_offset[OUT] 数据头部偏移
/* Return      : 0=未找到包头,>0一整包长度(head+body),<0(脏数据)未找到包头
/************************************************************************/
int32 vz_head_parse(const void *p_data, uint32 n_data, uint32 *n_offset) {
  if (n_data < sizeof(NetHead)) {
    return 0;
  }

  // 包长度
  NetHead* p_head = (NetHead*)p_data;
  if (p_head->mark_0 == NET_MARK_0
      && p_head->mark_1 == NET_MARK_1) {
    return sizeof(NetHead) + p_head->data_size;
  }

  // 查找包头偏移

  return -1;
}

/************************************************************************/
/* Description : 回调网络头部打包
/* Parameters  : p_data[OUT] 数据
                 n_data[IN]  可使用数据长度
/* Return      : >0包头占用数据长度, <0不够包头长度
/************************************************************************/
int32 vz_head_packet(void *p_data, uint32 n_data, uint32 n_body, const void *p_param) {
  if (n_data < sizeof(NetHead)) {
    return -1;
  }

  // 组包头
  NetHead* p_head = (NetHead*)p_data;
  p_head->mark_0 = NET_MARK_0;
  p_head->mark_1 = NET_MARK_1;
  p_head->type_flag = *((uint16*)p_param);
  p_head->data_size = n_body;

  return sizeof(NetHead);
}
