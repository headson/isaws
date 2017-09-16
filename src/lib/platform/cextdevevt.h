/************************************************************************
*Author      : Sober.Peng 17-09-16
*Description : 
************************************************************************/
#ifndef LIBPLATFORM_CEXTDEVEVT_H_
#define LIBPLATFORM_CEXTDEVEVT_H_

#include "platform/basedefines.h"
#include "vzbase/thread/thread.h"

class CExtDevEvt : public vzbase::MessageHandler {
 public:
  CExtDevEvt(vzbase::Thread *fast_thread);
  ~CExtDevEvt();

  bool Start();
  void Stop();

  void Write(const TAG_GPIO *gpio);

 protected:
  void OnMessage(vzbase::Message* msg);

 private:
  vzbase::Thread *fast_thread_;
};

#endif  // LIBPLATFORM_CEXTDEVEVT_H_
