#include "dispatcher/dpserver/dpserver.h"
#include "vzlogging/logging/vzloggingcpp.h"
#include "vzconn/async/cevttcpclient.h"

namespace dp {

DpServer::DpServer(vzconn::EventService &event_service)
  : event_service_(event_service) {
  memset(session_socket_map_, 0,
         MAX_SESSION_SIZE * sizeof(SessionSocketPair));
}

DpServer::~DpServer() {
}

bool DpServer::StartDpServer(
  const char *listen_addr,
  unsigned short listen_port) {
  //vzconn::EVT_LOOP c_loop;
  //c_loop.Start();
  vzconn::CEvtTcpServer *tcp_server_ = vzconn::CEvtTcpServer::Create(&event_service_,
                                       this,
                                       this);

  int32 n_ret = 0;
  vzconn::CInetAddr c_addr(listen_addr, listen_port);
  n_ret = tcp_server_->Open(&c_addr, false, true);

  //while (1) {
  //  n_ret = c_loop.RunLoop();
  //}
  return true;
}

bool DpServer::StopDpServer() {
  return true;
}

bool DpServer::HandleNewConnection(vzconn::VSocket *p_srv,
                                   vzconn::VSocket *new_sock) {
  // 找到可用index
  uint8 session_id = GetNewSessionId();
  if (session_id > MAX_SESSION_SIZE) {
    LOG(L_ERROR) << "Failure to get new session id, reject this connect";
    return false;
  }
  //
  Session *session = new Session(session_id, new_sock, this);
  if (!AddSession(session_id, session, new_sock)) {
    LOG(L_ERROR) << "Failure to add session";
    return false;
  }
  LOG(L_INFO) << "new session " << (uint32)session
              << " connection " << (uint32)new_sock;
  return true;
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
  if (session == NULL) {
    LOG(L_ERROR) << "No session Handle";
    return -1;
  }
  LOG(L_INFO) << "recv packet length " << n_data
              << " type " << dmsg->type
              << " id " << vzconn::NetworkToHost32(dmsg->id)
              << " session connection " << (uint32)session->vz_socket_
              << " connection " << (uint32)p_cli
              << " session id " << session->session_id_;
  //if (dmsg->type == TYPE_MESSAGE
  //    || dmsg->type == TYPE_REQUEST
  //    || dmsg->type == TYPE_REPLY) {
  //  // 分发处理
  //} else {
  //}
  if (dmsg->type == TYPE_GET_SESSION_ID
      || dmsg->type == TYPE_ADD_MESSAGE
      || dmsg->type == TYPE_REMOVE_MESSAGE) {
    // 自处理消息
    session->HandleSessionMessage(dmsg,
                                  p_data + sizeof(DpMessage),
                                  n_data - sizeof(DpMessage),
                                  0);
  } else {
    uint32 handler_size = DispatcherSession(session,
                                            dmsg,
                                            p_data + sizeof(DpMessage),
                                            n_data - sizeof(DpMessage));
    if (dmsg->type == TYPE_REPLY || dmsg->type == TYPE_MESSAGE
        || (dmsg->type == TYPE_REQUEST && handler_size == 0)) {
      if(handler_size) {
        session->ReplyDpMessage(dmsg, TYPE_SUCCEED, dmsg->channel_id);
      } else {
        session->ReplyDpMessage(dmsg, TYPE_FAILURE, dmsg->channel_id);
      }
    }
    //if (!handler_size) {
    //  LOG(L_ERROR) << "Failure to handle this message";
    //}
  }
  return 0;
}

uint32 DpServer::DispatcherSession(Session *session,
                                   const DpMessage *dmsg,
                                   const uint8 *data,
                                   uint32 data_size) {

  LOG(L_INFO) << "send packet length " << data_size
              << " type " << dmsg->type
              << " id " << vzconn::NetworkToHost32(dmsg->id)
              << "  " << (dmsg->id & 0xff);

  uint32 handler_size = 0;
  for (int i = 0; i < MAX_SESSION_SIZE; i++) {
    if (session && session != session_socket_map_[i].session
        && session_socket_map_[i].session != NULL) {  // 去除自身
      // 判断发送
      if(session_socket_map_[i].session->HandleSessionMessage(
            dmsg, data, data_size, 0)) {
        handler_size++;
        if (dmsg->type != TYPE_MESSAGE) {
          return handler_size;
        }
      }
    } else {
      if (session_socket_map_[i].session != NULL) {
        LOG(L_ERROR) << "the same session id "<<i;
      }
    }
  }
  return handler_size;
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
                          const uint8 *data, int size) {
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
uint8 DpServer::GetNewSessionId() {
  for (int i = 0; i < MAX_SESSION_SIZE; i++) {
    if (session_socket_map_[i].session == NULL
        && session_socket_map_[i].socket == NULL) {
      return (uint8)i;
    }
  }
  LOG(L_ERROR) << "Failure to get new session id";
  return MAX_SESSION_SIZE + 1;
}

bool DpServer::AddSession(uint8 n_idx,
                          Session *session,
                          vzconn::VSocket *socket) {
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