/************************************************************************/
/* Author      : SoberPeng 2017-06-24
/* Description : dispatcher worker,监听消息
/************************************************************************/
#ifndef _CLISTENMESSAGE_H
#define _CLISTENMESSAGE_H

#include "vzbase/base/basictypes.h"
#include "vzbase/thread/thread.h"

#include "web_server/base/mongoose.h"
#include "dispatcher/sync/dpclient_c.h"

class CListenMessage : public vzbase::Runnable{
 public:
  CListenMessage();
  virtual ~CListenMessage();

  bool  Start(const uint8 *s_dp_ip, uint16 n_dp_port,
              const uint8 *s_http_port,
              const uint8 *s_http_path);

  int32 RunLoop();

  // 广播消息(WebSocket)
  void broadcast(const void* p_data, uint32 n_data);

 protected:
  static void msg_handler(DPPollHandle p_hdl, const DpMessage *dmp, void* p_usr_arg);
  void OnMessage(DPPollHandle p_hdl, const DpMessage *dmp);

  static void state_handler(DPPollHandle p_hdl, uint32 n_state, void* p_usr_arg);
  void OnState(DPPollHandle p_hdl, uint32 n_state);

  //////////////////////////////////////////////////////////////////////////
  virtual void Run(vzbase::Thread* thread);
  /************************************************************************/
  /* Description : 默认request处理
  /* Parameters  :
  /* Return      :
  /************************************************************************/
  static void ev_handler(struct mg_connection *nc, int ev, void *ev_data);
  void OnEvHdl(struct mg_connection *nc, int ev, void *ev_data);

 private:
  DPPollHandle               p_dp_cli_;

  struct mg_mgr              c_web_srv_;
  struct mg_connection      *p_web_conn_;
  vzbase::Thread            *p_web_thread_;
};

#endif  // _CLISTENMESSAGE_H
