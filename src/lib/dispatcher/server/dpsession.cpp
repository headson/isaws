#include "dispatcher/server/dpsession.h"

#include "vzlogging/logging/vzloggingcpp.h"
#include <string.h>

namespace dp {

#define GET_CLIENT_ID(id) ((id) >> 24)

Session::Session(unsigned char session_id,
                 vzconn::VSocket *vz_socket,
                 SessionInterface *session_interface)
  : session_interface_(session_interface),
    session_id_(session_id),
    cur_pos_(0),
    vz_socket_(vz_socket) {
}

Session::~Session() {
}

bool Session::StartSession() {
  return true;
}

bool Session::StopSession() {
  return true;
}

bool Session::HandleSessionMessage(const DpMessage *dmp,
                                   const char *data,
                                   int size,
                                   int flag) {
  if (dmp->type == TYPE_GET_SESSION_ID) {
    ProcessGetSessionIdMessage(dmp);
  } else if (dmp->type == TYPE_ADD_MESSAGE) {
    ProcessAddListenMessage(dmp, data, size);
  } else if (dmp->type == TYPE_REMOVE_MESSAGE) {
    ProcessRemoveListenMessage(dmp, data, size);
  } else if (dmp->type == TYPE_MESSAGE) {
    ProcessDpMessage(dmp, data, size);
  } else if (dmp->type == TYPE_REQUEST) {
    ProcessDpMessage(dmp, data, size);
  } else if (dmp->type == TYPE_REPLY) {
    ProcessDpMessage(dmp, data, size);
  } else {
  }
  return true;
}

bool Session::ProcessGetSessionIdMessage(const DpMessage *dmp) {
  memcpy(&dmp_, dmp, sizeof(DpMessage));
  dmp_.channel_id = session_id_;
  if (session_interface_) {
    session_interface_->AsyncWrite(this, vz_socket_, &dmp_, NULL, 0);
  }
  return true;
}

bool Session::ProcessAddListenMessage(const DpMessage *dmp,
                                      const char *data,
                                      int size) {
  for (int i = 0; i < size; i+= MAX_METHOD_SIZE) {
    AddListenMessage(data + i);
  }
  if (session_interface_) {
    memcpy(&dmp_, dmp, sizeof(DpMessage));
    dmp_.type = TYPE_SUCCEED;
    session_interface_->AsyncWrite(this, vz_socket_, &dmp_, NULL, 0);
  }
  return true;
}

bool Session::ProcessRemoveListenMessage(const DpMessage *dmp,
    const char *data,
    int size) {
  for (int i = 0; i < size; i+= MAX_METHOD_SIZE) {
    RemoveListenMessage(data + i);
  }
  if (session_interface_) {
    memcpy(&dmp_, dmp, sizeof(DpMessage));
    dmp_.type = TYPE_SUCCEED;
    session_interface_->AsyncWrite(this, vz_socket_, &dmp_, NULL, 0);
  }
  return true;
}


bool Session::ProcessDpMessage(const DpMessage *dmp,
                               const char *data,
                               int size) {
  if (session_interface_ == NULL) {
    return false;
  }
  // 如果这个client_id与当前session一样，那么就直接回复这个消息
  if (session_id_ == GET_CLIENT_ID(dmp->id)) {
    if (session_interface_) {
      session_interface_->AsyncWrite(this, vz_socket_, dmp, data, size);
    }
  }
  // 小数据量查找，不需要考虑太多的性能因素在里面
  for (int i = 0; i < cur_pos_; i++) {
    if (strncmp(listen_messages_[i], dmp->method, MAX_METHOD_SIZE) == 0) {
      if (session_interface_) {
        session_interface_->AsyncWrite(this, vz_socket_, dmp, data, size);
      }
      return true;
    }
  }
  return false;
}

void Session::AddListenMessage(const char *message) {
  LOG(L_INFO) << "Add listen message >> " << message;
  if (cur_pos_ < MAX_METHOD_COUNT) {
    strncpy(listen_messages_[cur_pos_++], message, MAX_METHOD_SIZE);
  } else {
    LOG(L_ERROR) << "message method size big than MAX_METHOD_COUNT = 64";
  }
}

// [TEST]
void Session::RemoveListenMessage(const char *message) {
  LOG(L_INFO) << "Remove listen message >> " << message;
  // 一次循环内，首先这个消息找到，然后将后面的消息向前移动一次，达到删除消息的目标
  bool found = false;
  for (int i = 0; i < cur_pos_; i++) {
    // 先找到这个字段的消息
    if (!found && strncmp(message, listen_messages_[i], MAX_METHOD_SIZE) == 0) {
      found = true;
    }
    // 然后再将这个字段的使用后面的数据覆盖
    if (found && (i + 1 < cur_pos_)) {
      strncpy(listen_messages_[i], listen_messages_[i + 1], MAX_METHOD_SIZE);
    }
  }
  if (!found) {
    LOG(L_ERROR) << "the message is not found";
  }
}

};