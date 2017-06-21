#include "dispatcher/server/dpserver.h"
#include "vzlogging/logging/vzloggingcpp.h"
#include "vzconn/async/cevttcpclient.h"

namespace dp {


DpServer::DpServer() :
  cur_session_size_(0) {
  memset(session_socket_map_, 0, sizeof(SessionSocketPair));
}

DpServer::~DpServer() {
}

bool DpServer::StartDpServer() {
  vzconn::EVT_LOOP c_loop;
  c_loop.Start();
  vzconn::CEvtTcpServer *tcp_server_ = vzconn::CEvtTcpServer::Create(&c_loop,
                                       this,
                                       this);

  int32 n_ret = 0;
  vzconn::CInetAddr c_addr("0.0.0.0", 3730);
  n_ret = tcp_server_->Open(&c_addr, false, true);

  while (1) {
    n_ret = c_loop.RunLoop();
  }
  return true;
}

bool DpServer::StopDpServer() {
  return true;
}

bool DpServer::HandleNewConnection(vzconn::VSocket *p_srv, 
                                   vzconn::VSocket *new_sock) {
  Session *session = new Session((unsigned char)cur_session_size_,
                                 new_sock,
                                 this);
  if (!AddSession(session, new_sock)) {
    LOG(L_ERROR) << "Failure to add session";
    return false;
  }
  return true;
}

void DpServer::HandleServerClose(vzconn::VSocket *p_srv) {

}

// 有点绕
int32 DpServer::HandleRecvPacket(vzconn::VSocket       *p_cli,
                                 const uint8 *p_data,
                                 uint32      n_data,
                                 uint16      n_flag) {
  const DpMessage *dmsg = (const DpMessage *)(p_data);
  Session *session = FindSessionBySocket((vzconn::VSocket *)p_cli);

  if (dmsg->type == TYPE_MESSAGE
      || dmsg->type == TYPE_REQUEST
      || dmsg->type == TYPE_REPLY) {
    DispatcherSession(session, dmsg,
                      (const char *)p_data + sizeof(DpMessage),
                      n_data - sizeof(DpMessage));
  }
  if (session) {
    session->HandleSessionMessage(dmsg,
                                  (const char *)p_data + sizeof(DpMessage),
                                  n_data - sizeof(DpMessage),
                                  0);
  } else {
    LOG(L_ERROR) << "No Message Handle";
  }
  return 0;
}

bool DpServer::DispatcherSession(Session *session,
                                 const DpMessage *dmsg,
                                 const char *data,
                                 uint32 data_size) {
  for (int i = 0; i < cur_session_size_; i++) {
    if (session && session != session_socket_map_[i].session) {
      if (session_socket_map_[i].session->HandleSessionMessage(
            dmsg, data, data_size, 0) && dmsg->type != TYPE_MESSAGE) {
        return true;
      }
    }
  }
  return true;
}

int32 DpServer::HandleSendPacket(vzconn::VSocket *p_cli) {
  return 0;
}

void  DpServer::HandleClose(vzconn::VSocket *p_cli) {
  RemoveBySocket((vzconn::VSocket *)p_cli);
}

void DpServer::AsyncWrite(Session *session,
                          vzconn::VSocket *vz_socket,
                          const DpMessage *dmp,
                          const char *data, int size) {
  vzconn::CEvtTcpClient * client = static_cast<vzconn::CEvtTcpClient*>(vz_socket);
  if (data) {
    client->AsyncWrite(data, size, 0);
  } else {
    client->AsyncWrite((const void *)dmp, sizeof(DpMessage), 0);
  }
}
void DpServer::OnSessionError(Session *session, vzconn::VSocket *vz_socket) {
  RemoveBySession(session);
}

////////////////////////////////////////////////////////////////////////////////

bool DpServer::AddSession(Session *session, vzconn::VSocket *socket) {
  LOG(L_INFO) << "Add session ";
  if (session == NULL || socket == NULL) {
    return false;
  }
  if (cur_session_size_ < MAX_SESSION_SIZE) {
    session_socket_map_[cur_session_size_].session = session;
    session_socket_map_[cur_session_size_].socket  = socket;
    cur_session_size_ ++;
    return true;
  } else {
    LOG(L_ERROR) << "session size big than MAX_METHOD_COUNT = 128";
  }
  return false;
}

bool DpServer::RemoveBySession(Session *session) {
  LOG(L_INFO) << "Remove session by socket";
  // 一次循环内，首先这个消息找到，然后将后面的消息向前移动一次，达到删除消息的目标
  bool found = false;
  for (int i = 0; i < cur_session_size_; i++) {
    // 先找到这个字段的消息
    if (!found && session_socket_map_[cur_session_size_].session == session) {
      found = true;
    }
    // 然后再将这个字段的使用后面的数据覆盖
    if (found && (i + 1 < cur_session_size_)) {
      memcpy((void *)(session_socket_map_ + i),
             (const void *)(session_socket_map_ + i + 1),
             sizeof(SessionSocketPair));
      cur_session_size_ --;
    }
  }
  if (!found) {
    LOG(L_ERROR) << "the message is not found";
  }
  return found;
}

bool DpServer::RemoveBySocket(vzconn::VSocket *socket) {
  LOG(L_INFO) << "Remove session by socket";
  // 一次循环内，首先这个消息找到，然后将后面的消息向前移动一次，达到删除消息的目标
  bool found = false;
  for (int i = 0; i < cur_session_size_; i++) {
    // 先找到这个字段的消息
    if (!found && session_socket_map_[cur_session_size_].socket == socket) {
      found = true;
    }
    // 然后再将这个字段的使用后面的数据覆盖
    if (found && (i + 1 < cur_session_size_)) {
      memcpy((void *)(session_socket_map_ + i),
             (const void *)(session_socket_map_ + i + 1),
             sizeof(SessionSocketPair));
      cur_session_size_ --;
    }
  }
  if (!found) {
    LOG(L_ERROR) << "the message is not found";
  }
  return found;
}

Session *DpServer::FindSessionBySocket(vzconn::VSocket *socket) {
  for (int i = 0; i < cur_session_size_; i++) {
    if (session_socket_map_[i].socket == socket) {
      return session_socket_map_[i].session;
    }
  }
  return NULL;
}

vzconn::VSocket *DpServer::FindSocketBySession(Session *session) {
  for (int i = 0; i < cur_session_size_; i++) {
    if (session_socket_map_[i].session == session) {
      return session_socket_map_[i].socket;
    }
  }
  return NULL;
}

};