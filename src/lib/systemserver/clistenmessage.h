/************************************************************************/
/* Author      : SoberPeng 2017-06-24
/* Description : dispatcher worker,¼àÌýÏûÏ¢
/************************************************************************/
#ifndef _CLISTENMESSAGE_H
#define _CLISTENMESSAGE_H

#include "vzbase/base/basictypes.h"
#include "dispatcher/sync/dpclient_c.h"

class CListenMessage {
 public:
  CListenMessage();
  virtual ~CListenMessage();

  bool  Start(const uint8 *s_ip, uint16 n_port);

  int32 RunLoop();

 protected:
  static void MsgFunc(const DpMessage *dmp, void* p_usr_arg);
  void OnMessage(const DpMessage *dmp);

 private:
   void   *p_dp_cli_;
};

#endif  // _CLISTENMESSAGE_H
