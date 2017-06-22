#include "dispatcher/server/dpserver.h"
#include "vzlogging/logging/vzloggingcpp.h"
#include "vzconn/async/cevttcpclient.h"

namespace dp {

DpServer::DpServer() {
  memset(session_socket_map_, 0,
         MAX_SESSION_SIZE * sizeof(SessionSocketPair));
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
  // 找到可用index
  uint32 n_index = 0;
  for (n_index = 0; n_index < MAX_SESSION_SIZE; n_index++) {
    if (session_socket_map_[n_index].session == NULL
        || session_socket_map_[n_index].socket == NULL) {
      break;
    }
  }

  //
  if (n_index < MAX_SESSION_SIZE) {
    Session *session = new Session((unsigned char)n_index,
                                   new_sock,
                                   this);
    if (!AddSession(n_index, session, new_sock)) {
      LOG(L_ERROR) << "Failure to add session";
      return false;
    }
    LOG(L_WARNING) << "new session " << (uint32)session
                   << " connection " << (uint32)new_sock;
    return true;
  }

  LOG(L_ERROR) << "have no session to use.";
  return false;
}

void DpServer::HandleServerClose(vzconn::VSocket *p_srv) {
}

// 有点绕
int32 DpServer::HandleRecvPacket(vzconn::VSocket *p_cli,
                                 const uint8   *p_data,
                                 uint32         n_data,
                                 uint16         n_flag) {
  const DpMessage *dmsg = (const DpMessage *)(p_data);

  Session *session = FindSessionBySocket(p_cli);
  LOG(L_WARNING) << "recv packet length " << n_data
                 << " type " << dmsg->type
                 << " id " << (int32)(dmsg->id & 0xff)
                 << " session connection " << (uint32)session->vz_socket_
                 << " connection " << (uint32)p_cli;
  if (session == NULL) {
    LOG(L_ERROR) << "No session Handle";
  }
  if (dmsg->type == TYPE_MESSAGE
      || dmsg->type == TYPE_REQUEST
      || dmsg->type == TYPE_REPLY) {
    // 分发处理
    DispatcherSession(session,
                      dmsg,
                      (const char *)p_data + sizeof(DpMessage),
                      n_data - sizeof(DpMessage));
  } else {
    // 自处理消息
    session->HandleSessionMessage(dmsg,
                                  (const char *)p_data + sizeof(DpMessage),
                                  n_data - sizeof(DpMessage),
                                  0);
  }
  return 0;
}

bool DpServer::DispatcherSession(Session *session,
                                 const DpMessage *dmsg,
                                 const char *data,
                                 uint32 data_size) {
  bool b_send = false;
  for (int i = 0; i < MAX_SESSION_SIZE; i++) {
    if (session && session != session_socket_map_[i].session) {  // 去除自身
      // 去除不存在session
      if (session_socket_map_[i].session == NULL
          || session_socket_map_[i].socket == NULL) {
        memset(&session_socket_map_[i], 0, sizeof(SessionSocketPair));
        continue;
      }

      // 判断发送
      if(session_socket_map_[i].session->HandleSessionMessage(
            dmsg, data, data_size, 0)) {
        //LOG(L_WARNING) << "send session index " << i
        //               << " method "<< dmsg->method
        //               << " message id " << (uint32)(dmsg->id & 0xff)
        //               << " message type " << (int32)dmsg->type;
        b_send = true;
        if (dmsg->type != TYPE_MESSAGE) {
          break;
        }
      }
    }
  }

  if (dmsg->type == TYPE_REQUEST && b_send == true) {
    return b_send;
  }
  DpMessage dp_msg;
  memcpy(&dp_msg, dmsg, sizeof(DpMessage));
  dp_msg.type = b_send ? TYPE_SUCCEED : TYPE_FAILURE;
  b_send = AsyncWrite(session, session->GetSocket(), &dp_msg, NULL, 0);
  if (b_send == false) {
    LOG(L_ERROR) << "send failed.";
  }
  return b_send;
}

int32 DpServer::HandleSendPacket(vzconn::VSocket *p_cli) {
  return 0;
}

void  DpServer::HandleClose(vzconn::VSocket *p_cli) {
  RemoveBySocket(p_cli);
}

bool DpServer::AsyncWrite(Session *session,
                          vzconn::VSocket *vz_socket,
                          const DpMessage *dmp,
                          const char *data, int size) {
  //vzconn::CEvtTcpClient * client = static_cast<vzconn::CEvtTcpClient*>(vz_socket);
  //if (data) {
  //  client->AsyncWrite(data, size, 0);
  //} else {
  //  client->AsyncWrite((const void *)dmp, sizeof(DpMessage), 0);
  //}
  struct iovec iov[2];
  iov[0].iov_base = (void*)dmp;
  iov[0].iov_len  = sizeof(DpMessage);
  iov[1].iov_base = (void*)data;
  iov[1].iov_len  = size;
  int32 n_ret = vz_socket->AsyncWrite(iov, 2, 0);
  if (n_ret > 0) {
    return true;
  }
  return false;
}

void DpServer::OnSessionError(Session *session, vzconn::VSocket *vz_socket) {
  RemoveBySession(session);
}

////////////////////////////////////////////////////////////////////////////////
bool DpServer::AddSession(uint32 n_idx, Session *session, vzconn::VSocket *socket) {
  LOG(L_INFO) << "Add session ";
  if (session == NULL || socket == NULL) {
    return false;
  }
  if (n_idx < MAX_SESSION_SIZE) {
    session_socket_map_[n_idx].session = session;
    session_socket_map_[n_idx].socket  = socket;
    return true;
  } else {
    LOG(L_ERROR) << "session size big than MAX_METHOD_COUNT = 128";
  }
  return false;
}

bool DpServer::RemoveBySession(Session *session) {
  LOG(L_INFO) << "Remove session by socket";
  // 一次循环内，首先这个消息找到，然后将后面的消息向前移动一次，达到删除消息的目标
  for (int i = 0; i < MAX_SESSION_SIZE; i++) {
    // 先找到这个字段的消息
    if (session_socket_map_[i].session == session) {
      memset(&session_socket_map_[i], 0, sizeof(SessionSocketPair));
      return true;
    }
  }
  return false;
}

bool DpServer::RemoveBySocket(vzconn::VSocket *socket) {
  LOG(L_INFO) << "Remove session by socket";
  // 一次循环内，首先这个消息找到，然后将后面的消息向前移动一次，达到删除消息的目标
  for (int i = 0; i < MAX_SESSION_SIZE; i++) {
    // 先找到这个字段的消息
    if (session_socket_map_[i].socket == socket) {
      memset(&session_socket_map_[i], 0, sizeof(SessionSocketPair));
      return true;
    }
  }
  return false;
}

Session *DpServer::FindSessionBySocket(vzconn::VSocket *socket) {
  for (int i = 0; i < MAX_SESSION_SIZE; i++) {
    if (session_socket_map_[i].socket == socket) {
      return session_socket_map_[i].session;
    }
  }
  return NULL;
}

vzconn::VSocket *DpServer::FindSocketBySession(Session *session) {
  for (int i = 0; i < MAX_SESSION_SIZE; i++) {
    if (session_socket_map_[i].session == session) {
      return session_socket_map_[i].socket;
    }
  }
  return NULL;
}

};