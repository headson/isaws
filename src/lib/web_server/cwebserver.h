/************************************************************************/
/* Author      : SoberPeng 2017-07-07
/* Description :
/************************************************************************/
#ifndef LIBWEBSRV_CWEBSERVER_H
#define LIBWEBSRV_CWEBSERVER_H

#include "vzbase/base/basictypes.h"

#include "vzbase/thread/thread.h"
#include "web_server/base/mongoose.h"

namespace web { 

class CWebServer : public vzbase::Runnable {
 public:
  CWebServer();
  virtual ~CWebServer();

  bool Start(const char *s_web_path, 
             const char *s_log_path,
             const char *s_http_port);
  void Stop();

  void Broadcast(const void* p_data, unsigned int n_data);

 protected:
  virtual void Run(vzbase::Thread* thread);

  /************************************************************************/
  /* Description : Ä¬ÈÏrequest´¦Àí
  /* Parameters  :
  /* Return      :
  /************************************************************************/
  static void web_ev_handler(struct mg_connection *nc, int ev, void *ev_data);

 private:
  struct mg_mgr              c_web_srv_;
  struct mg_connection      *p_web_conn_;

  bool                       b_runing_;
  vzbase::Thread            *p_web_thread_;
};

}  // namespace web

#endif  // LIBWEBSRV_CWEBSERVER_H
