/************************************************************************/
/* Author      : SoberPeng 2017-06-24
/* Description : dispatcher worker,¼àÌýÏûÏ¢
/************************************************************************/
#ifndef _CLISTENMESSAGE_H
#define _CLISTENMESSAGE_H

#include "vzbase/base/basictypes.h"

#include "dispatcher/sync/dpclient_c.h"

#include "vzlogging/logging/vzwatchdog.h"
#include "vzlogging/logging/vzloggingcpp.h"

class CListenMessage {
 public:
  CListenMessage();
  virtual ~CListenMessage();

  bool  Start(const uint8 *s_dp_ip, uint16 n_dp_port);

  int32 RunLoop();
  
 protected:
  static void msg_handler(const DpMessage *dmp, void* p_usr_arg);
  void OnMessage(const DpMessage *dmp);
  
 private:
  DPPollHandle       p_dp_cli_;
};

#endif  // _CLISTENMESSAGE_H
