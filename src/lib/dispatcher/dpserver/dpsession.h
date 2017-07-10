#ifndef LIBDISPATCHER_SERVER_SESSION_H_
#define LIBDISPATCHER_SERVER_SESSION_H_

#include "dispatcher/base/pkghead.h"
#include "vzconn/base/vsocket.h"

namespace dp {

class SessionInterface;

class Session {
 public:
  Session(unsigned char session_id,
          vzconn::VSocket *vz_socket,
          SessionInterface *session_interface);
  virtual ~Session();

  bool StartSession();
  bool StopSession();
  bool HandleSessionMessage(const DpMessage *dmp,
                            const char *data,
                            int size,
                            int flag);
  vzconn::VSocket* GetSocket() {
    return vz_socket_;
  }
  bool ReplyDpMessage(const DpMessage *dmsg, unsigned char type, unsigned char channel);
 private:
  bool ProcessGetSessionIdMessage(const DpMessage *dmp);
  bool ProcessAddListenMessage(const DpMessage *dmp,
                               const char *data,
                               int size);
  bool ProcessRemoveListenMessage(const DpMessage *dmp,
                                  const char *data,
                                  int size);

  bool ProcessDpMessage(const DpMessage *dmp,
                        const char *data,
                        int size);

  //////////////////////////////////////////////////////////////////////////////
  void AddListenMessage(const char *message);
  void RemoveListenMessage(const char *message);

 public:
  SessionInterface *session_interface_;
  unsigned char session_id_;
  char listen_messages_[MAX_METHOD_COUNT][MAX_METHOD_SIZE];
  int cur_pos_;
  vzconn::VSocket *vz_socket_;
  DpMessage dmp_;
};


class SessionInterface {
 public:
  virtual bool AsyncWrite(Session *session,
                          vzconn::VSocket *vz_socket,
                          const DpMessage *dmp,
                          const char *data, int size) = 0;

  virtual void OnSessionError(Session *session, vzconn::VSocket *vz_socket) = 0;
  // virtual void OnPushMessage(Session *session,
  //                           const DpMessage *dmp,
  //                           const char *data,
  //                           int size,
  //                           int flag);
};

};

#endif // LIBDISPATCHER_SERVER_SESSION_H_
