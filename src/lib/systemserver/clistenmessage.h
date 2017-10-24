/************************************************************************/
/* Author      : SoberPeng 2017-06-24
/* Description : dispatcher worker,������Ϣ
/************************************************************************/
#ifndef _CLISTENMESSAGE_H
#define _CLISTENMESSAGE_H

#include "vzbase/base/basictypes.h"

#include "json/json.h"
#include "vzbase/thread/thread.h"

#include "network/cnetwork.h"
#include "hwclock/chwclock.h"

#include "vzbase/base/noncoypable.h"
#include "dispatcher/sync/dpclient_c.h"

#include "systemserver/base/pkghead.h"

namespace sys {

class CListenMessage : public vzbase::noncopyable {
 protected:
  CListenMessage();
  virtual ~CListenMessage();

 public:
  static CListenMessage *Instance();

  bool  Start();
  void  Stop();

  void  RunLoop();

 protected:
  static void dpcli_poll_msg_cb(DPPollHandle p_hdl,
                                const DpMessage *dmp, void* p_usr_arg);
  void OnDpMessage(DPPollHandle p_hdl, const DpMessage *dmp);

  static void dpcli_poll_state_cb(DPPollHandle p_hdl,
                                  unsigned int n_state, void* p_usr_arg);
  void OnDpState(DPPollHandle p_hdl, unsigned int n_state);

 public:
  void GetHwInfo();                           // ��ȡӲ����Ϣ
  void GetAlgVer();                           // ��ȡ�㷨�汾

  bool GetDevInfo(Json::Value &j_body);       // ��ȡ�豸��Ϣ
  bool SetDevInfo(const Json::Value &j_body); // �����豸��Ϣ

  int SetIp(in_addr_t ip);
  int SetNetmask(in_addr_t ip);
  int SetGateway(in_addr_t ipaddr);
  int SetDNS(in_addr_t ip);

 public:
  TAG_SYS_INFO    sys_info_;      // Ӳ������
  std::string     nickname_;      // eth0\wlan0
  unsigned int    ip_change_;     // 1=change,0=no change

 private:
  DPPollHandle    dp_cli_;
  vzbase::Thread *thread_fast_;   // �����߳�
  vzbase::Thread *thread_slow_;   // ��ʱ�߳�

 private:
  CHwclock       *hw_clock_;      // ʱ������
  CNetwork  *mcast_dev_;     // �������
};

}  // namespace sys

#endif  // _CLISTENMESSAGE_H
