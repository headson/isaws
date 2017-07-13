/************************************************************************/
/* Author      : SoberPeng 2017-06-24
/* Description :
/************************************************************************/
#include "uri_handle.h"

#include "vzbase/helper/stdafx.h"

#include <vector>
#include <iostream>

#include "vzbase/base/base64.h"

#include "web_for_dp/clistenmessage.h"

namespace web {

std::string GetNowTime() {
  struct timeval tv;
  gettimeofday(&tv, NULL);
  time_t tt = tv.tv_sec;
  struct tm *wtm = localtime(&tt);

  char s_time[20] = {0};
  snprintf(s_time, 19,
           "%02d:%02d:%02d",
           wtm->tm_hour, wtm->tm_min, wtm->tm_sec);
  return s_time;
}

// dispatch 测试 handle
// 设置设备地址
void uri_hdl_set_dev_addr(struct mg_connection *nc, int ev, void *ev_data) {
  int n_state = 6;
  std::string s_msg = "";
  unsigned int n_id = 0;
  Json::Value j_req, j_resp;

  if (!parse_request(s_msg, n_id, j_req, (struct http_message *)ev_data)) {
    LOG(L_ERROR) << "parse body data failed.";
    return;
  }

  if (j_req["body"]["ip"].isString()
      && j_req["body"]["port"].isInt()) {

    int n_port = j_req["body"]["port"].asInt();
    std::string s_ip = j_req["body"]["ip"].asString();

    bool b_ret = CListenMessage::Instance()->Restart(s_ip.c_str(), n_port);
    if (true == b_ret) {
      n_state = 0;
    }
  }

  send_response(nc, s_msg, n_id, n_state, j_resp);
}

// 注册消息
void uri_hdl_add_listen_msg(struct mg_connection *nc, int ev, void *ev_data) {
  int n_state = 6;
  std::string s_msg = "";
  unsigned int n_id = 0;
  Json::Value j_req, j_resp;

  if (!parse_request(s_msg, n_id, j_req, (struct http_message *)ev_data)) {
    LOG(L_ERROR) << "parse body data failed.";
    return;
  }

  if (j_req["body"]["method"].isString()) {

    bool b_ret = CListenMessage::Instance()->AddListenMessage(
                   j_req["body"]["method"].asString());
    if (true == b_ret) {
      n_state = 0;
    }
  }

  send_response(nc, s_msg, n_id, n_state, j_resp);
}

// 取消注册消息
void uri_hdl_del_listen_msg(struct mg_connection *nc, int ev, void *ev_data) {
  int n_state = 6;
  std::string s_msg = "";
  unsigned int n_id = 0;
  Json::Value j_req, j_resp;

  if (!parse_request(s_msg, n_id, j_req, (struct http_message *)ev_data)) {
    LOG(L_ERROR) << "parse body data failed.";
    return;
  }

  if (j_req["body"]["method"].isString()) {
    bool b_ret = CListenMessage::Instance()->DelListenMessage(
                   j_req["body"]["method"].asString());
    if (true == b_ret) {
      n_state = 0;
    }
  }

  send_response(nc, s_msg, n_id, n_state, j_resp);
}

// 发送请求
void DpClientCallback(DPPollHandle     p_hdl,
                      const DpMessage *dmp,
                      void            *p_usr_arg) {
  vzbase::Base64::EncodeFromArray(dmp->data,
                                  dmp->data_size,
                                  (std::string *)p_usr_arg);
}

void uri_hdl_send_request(struct mg_connection *nc, int ev, void *ev_data) {
  int n_state = 6;
  std::string s_msg = "";
  unsigned int n_id = 0;
  Json::Value j_req, j_resp;

  if (!parse_request(s_msg, n_id, j_req, (struct http_message *)ev_data)) {
    LOG(L_ERROR) << "parse body data failed.";
    return;
  }

  if (j_req["body"]["method"].isString()
      && j_req["body"]["timeout"].isInt()
      && j_req["body"]["data"].isString()) {
    std::size_t       n_data;
    std::vector<char> s_data;
    vzbase::Base64::Decode(j_req["body"]["data"].asString(),
                           vzbase::Base64::DO_STRICT,
                           &s_data,
                           &n_data);

    std::string s_resp = "";
    int n_ret = DpClient_SendDpRequest(j_req["body"]["method"].asCString(),
                                       0,
                                       &s_data[0], n_data,
                                       DpClientCallback,
                                       &s_resp,
                                       (j_req["body"]["timeout"].asInt()-1)*1000);
    if (n_ret == VZNETDP_SUCCEED) {
      j_resp["method"] = j_req["body"]["method"].asString();
      j_resp["time"]   = GetNowTime();
      j_resp["data"]   = s_resp;

      n_state = 0;
    }
  }

  send_response(nc, s_msg, n_id, n_state, j_resp);
}

// 发送回复
void uri_hdl_send_reply(struct mg_connection *nc, int ev, void *ev_data) {
  int n_state = 6;
  std::string s_msg = "";
  unsigned int n_id = 0;
  Json::Value j_req, j_resp;

  if (!parse_request(s_msg, n_id, j_req, (struct http_message *)ev_data)) {
    LOG(L_ERROR) << "parse body data failed.";
    return;
  }

  if (j_req["body"]["method"].isString()
      && j_req["body"]["data"].isString()) {
    std::size_t       n_data;
    std::vector<char> s_data;
    vzbase::Base64::Decode(j_req["body"]["data"].asString(),
                           vzbase::Base64::DO_STRICT,
                           &s_data,
                           &n_data);

    int n_ret = DpClient_SendDpReply(j_req["body"]["method"].asCString(),
                                     0,
                                     n_id,
                                     &s_data[0],
                                     n_data);
    if (n_ret == VZNETDP_SUCCEED) {
      n_state = 0;
    }
  }

  send_response(nc, s_msg, n_id, n_state, j_resp);
}

// 发送推送
void uri_hdl_send_message(struct mg_connection *nc, int ev, void *ev_data) {
  int n_state = 6;
  std::string s_msg = "";
  unsigned int n_id = 0;
  Json::Value j_req, j_resp;

  if (!parse_request(s_msg, n_id, j_req, (struct http_message *)ev_data)) {
    LOG(L_ERROR) << "parse body data failed.";
    return;
  }

  if (j_req["body"]["method"].isString()
      && j_req["body"]["data"].isString()) {
    std::size_t       n_data;
    std::vector<char> s_data;
    vzbase::Base64::Decode(j_req["body"]["data"].asString(),
                           vzbase::Base64::DO_STRICT,
                           &s_data,
                           &n_data);

    int n_ret = DpClient_SendDpMessage(j_req["body"]["method"].asCString(),
                                       0,
                                       &s_data[0],
                                       n_data);
    if (n_ret == VZNETDP_SUCCEED) {
      n_state = 0;
    }
  }

  send_response(nc, s_msg, n_id, n_state, j_resp);
}

// 发送推送
std::list<Json::Value> k_message;
void uri_hdl_get_recv_msg(struct mg_connection *nc, int ev, void *ev_data) {
  int n_state = 0;
  std::string s_msg = "";
  unsigned int n_id = 0;
  Json::Value j_req, j_resp(Json::arrayValue);

  if (!parse_request(s_msg, n_id, j_req, (struct http_message *)ev_data)) {
    LOG(L_ERROR) << "parse body data failed.";
    return;
  }

  while (k_message.size() > 0) {
    j_resp.append(k_message.front());
    k_message.pop_front();
  }
  send_response(nc, s_msg, n_id, n_state, j_resp);
}

}  // namespace web
