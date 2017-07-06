#include "dispatcher/dpserver/dpsession.h"

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
                                   const uint8 *data,
                                   int size,
                                   int flag) {
  bool b_ret = false;
  if (dmp->type == TYPE_GET_SESSION_ID) {
    b_ret = ProcessGetSessionIdMessage(dmp);
  } else if (dmp->type == TYPE_ADD_MESSAGE) {
    b_ret = ProcessAddListenMessage(dmp, data, size);
  } else if (dmp->type == TYPE_REMOVE_MESSAGE) {
    b_ret = ProcessRemoveListenMessage(dmp, data, size);
  } else if (dmp->type == TYPE_MESSAGE) {
    b_ret = ProcessDpMessage(dmp, data, size);
  } else if (dmp->type == TYPE_REQUEST) {
    b_ret = ProcessDpMessage(dmp, data, size);
  } else if (dmp->type == TYPE_REPLY) {
    b_ret = ProcessDpMessage(dmp, data, size);
  }
  return b_ret;
}

bool Session::ProcessGetSessionIdMessage(const DpMessage *dmp) {
  return ReplyDpMessage(dmp, dmp->type, session_id_);
}

bool Session::ProcessAddListenMessage(const DpMessage *dmp,
                                      const uint8 *data,
                                      int size) {
  for (int i = 0; i < size; i+= MAX_METHOD_SIZE) {
    AddListenMessage((const char *)data + i);
  }
  return ReplyDpMessage(dmp, TYPE_SUCCEED, dmp->channel_id);
}

bool Session::ProcessRemoveListenMessage(const DpMessage *dmp,
    const uint8 *data,
    int size) {
  for (int i = 0; i < size; i+= MAX_METHOD_SIZE) {
    RemoveListenMessage((const char *)data + i);
  }
  return ReplyDpMessage(dmp, TYPE_SUCCEED, dmp->channel_id);
}

bool Session::ProcessDpMessage(const DpMessage *dmp,
                               const uint8 *data,
                               int size) {
  if (session_interface_ == NULL) {
    return false;
  }
  // ��ִ���͵���Ϣ,ͨ��session idȥ���Ҷ�Ӧsession
  uint32 id = (vzconn::VZ_ORDER_BYTE == vzconn::ORDER_NETWORK) ?
              vzconn::NetworkToHost32(dmp->id) : dmp->id;

  //LOG(L_WARNING) << "type " << dmp->type
  //               << " session id " << session_id_
  //               << " msg session id " << GET_CLIENT_ID(id);
  // ������client_id�뵱ǰsessionһ������ô��ֱ�ӻظ������Ϣ
  if (session_id_ == GET_CLIENT_ID(id)) {
    if (session_interface_) {
      session_interface_->AsyncWrite(this, vz_socket_, dmp, data, size);
      //LOG(L_INFO) << "send back";
      return true;
    }
  }

  if (dmp->type == TYPE_REPLY) { // TYPE_REPLY��methodû�ı�,Ҳ�ᱻworker����
    //LOG(L_INFO) << "reply";
    return false;
  }

  // С���������ң�����Ҫ����̫�����������������
  for (int i = 0; i < cur_pos_; i++) {
    if (strncmp(listen_messages_[i], dmp->method, MAX_METHOD_SIZE) == 0) {
      if (session_interface_) {
        //LOG(L_INFO) << "dispatch";
        session_interface_->AsyncWrite(this, vz_socket_, dmp, data, size);
      }
      return true;
    }
  }
  return false;
}

bool Session::ReplyDpMessage(const DpMessage *dmsg, uint8 type, uint8 channel) {
  if (session_interface_) {
    memcpy(&dmp_, dmsg, sizeof(DpMessage));
    dmp_.channel_id = session_id_;
    dmp_.reply_type = dmp_.type;
    dmp_.type       = type;
    session_interface_->AsyncWrite(this, vz_socket_, &dmp_, NULL, 0);
    //LOG(L_WARNING) << "send reply message";
    return true;
  }
  return false;
}

//////////////////////////////////////////////////////////////////////////
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
  // һ��ѭ���ڣ����������Ϣ�ҵ���Ȼ�󽫺������Ϣ��ǰ�ƶ�һ�Σ��ﵽɾ����Ϣ��Ŀ��
  bool found = false;
  for (int i = 0; i < cur_pos_; i++) {
    // ���ҵ�����ֶε���Ϣ
    if (!found && strncmp(message, listen_messages_[i], MAX_METHOD_SIZE) == 0) {
      found = true;
    }
    // Ȼ���ٽ�����ֶε�ʹ�ú�������ݸ���
    if (found && (i + 1 < cur_pos_)) {
      strncpy(listen_messages_[i], listen_messages_[i + 1], MAX_METHOD_SIZE);
    }
  }
  if (!found) {
    LOG(L_ERROR) << "the message is not found";
  }
}

};