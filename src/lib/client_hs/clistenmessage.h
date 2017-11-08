/************************************************************************/
/* Author      : SoberPeng 2017-06-24
/* Description : dispatcher worker,监听消息
/************************************************************************/
#ifndef _CLISTENMESSAGE_H
#define _CLISTENMESSAGE_H

#include "vzbase/base/basictypes.h"
#include "vzbase/base/noncoypable.h"

#include "vzbase/thread/thread.h"
#include "dispatcher/sync/dpclient_c.h"

#include "client_hs/client/cclientaccess.h"
#include "client_hs/client/cclientdispatch.h"

namespace cli {

class CListenMessage : public vzbase::noncopyable,
  public vzbase::MessageHandler {
 public:
  typedef std::map<std::string, CClientDispatch::Ptr> MAP_DISP;

 protected:
  CListenMessage();
  virtual ~CListenMessage();

 public:
  static CListenMessage *Instance();

  bool  Start();
  void  Stop();

  void  RunLoop();

  vzbase::Thread  *MainThread();
  DPPollHandle     GetDpPollHdl();

 protected:
  //
  static void dpcli_poll_msg_cb(DPPollHandle p_hdl,
                                const DpMessage *dmp, void* p_usr_arg);
  void OnDpMessage(DPPollHandle p_hdl, const DpMessage *dmp);

  //
  static void dpcli_poll_state_cb(DPPollHandle p_hdl,
                                  unsigned int n_state, void* p_usr_arg);
  void OnDpState(DPPollHandle p_hdl, unsigned int n_state);

  // 线程消息Post,处理函数
  void OnMessage(vzbase::Message* msg);

 public:
  bool CreateAccessConnector(vzbase::Thread *thread);
  bool CreateDispatchConnector(int nMain, int nMinor, const std::string &sXml);
  bool CreateStorageConnector(std::string sHost, int nPort);

 private:
  DPPollHandle        dp_cli_;
  vzbase::Thread     *main_thread_;

 private:
  CClientAccess::Ptr  client_access_;
  MAP_DISP            client_dispatch_;
};

}  // namespace cli
#endif  // _CLISTENMESSAGE_H
