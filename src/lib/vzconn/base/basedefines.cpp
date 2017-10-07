/************************************************************************/
/* Author      : Sober.Peng 17-06-16                                    */
/* Description : 网络包头结构                                            */
/************************************************************************/
#include "vzconn/base/basedefines.h"
#include "vzconn/buffer/byteorder.h"

#include <string.h>

#ifdef __cplusplus
namespace vzconn {
  extern "C" {
#endif

unsigned int VzNetHeadSize() {
  return sizeof(NetHead);
}

unsigned int VzNetHeadPacket(char *pdata, int ndata,
                           unsigned int nbody, unsigned short eflag) {
  if (pdata == NULL || ndata < VzNetHeadSize()) {
    return -1;
  }
  NetHead chead;
  chead.mark[0] = NET_MARK_0;
  chead.mark[1] = NET_MARK_1;
  chead.type_flag = (ORDER_NETWORK == VZ_ORDER_BYTE) ?
    HostToNetwork16(eflag) : eflag;
  chead.data_size = (ORDER_NETWORK == VZ_ORDER_BYTE) ?
    HostToNetwork32(nbody) : nbody;
  memcpy(pdata, &chead, VzNetHeadSize());

  return VzNetHeadSize();
}

int VzNetHeadParse(const char *pdata, int ndata, unsigned short *eflag) {
  if (ndata < VzNetHeadSize()) {
    return 0;
  }

  int32 pkg_size = 0;
  if ((((uint32)pdata) % sizeof(uint32)) == 0) {
    // 对齐解析
    NetHead* p_head = (NetHead*)pdata;
    if (p_head->mark[0] == NET_MARK_0
        && p_head->mark[1] == NET_MARK_1) {
      pkg_size = VzNetHeadSize();
      pkg_size += (ORDER_NETWORK == VZ_ORDER_BYTE) ?
        NetworkToHost32(p_head->data_size) : p_head->data_size;
      *eflag = (ORDER_NETWORK == VZ_ORDER_BYTE) ?
        NetworkToHost16(p_head->type_flag) : p_head->type_flag;
    }
    else {
      return -1;
    }
  }
  else {
    // 未对齐解析
    NetHead c_head;
    memcpy(&c_head, pdata, VzNetHeadSize());
    if (c_head.mark[0] == NET_MARK_0
        && c_head.mark[1] == NET_MARK_1) {
      pkg_size = VzNetHeadSize();
      pkg_size += (ORDER_NETWORK == VZ_ORDER_BYTE) ?
        NetworkToHost32(c_head.data_size) : c_head.data_size;

      *eflag = (ORDER_NETWORK == VZ_ORDER_BYTE) ?
        NetworkToHost16(c_head.type_flag) : c_head.type_flag;;
    }
    else {
      return -1;
    }
  }
  return pkg_size;
}

#ifdef __cplusplus
  };
}  // namespace vzconn
#endif
