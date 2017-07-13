/************************************************************************/
/* Author      : SoberPeng 2017-06-24
/* Description :
/************************************************************************/
#ifndef _URI_HANDLE_H
#define _URI_HANDLE_H

#include "vzbase/base/basictypes.h"

#include <list>

#include "json/json.h"
#include "web_for_dp/base/mongoose.h"

#ifdef __cplusplus
namespace web {
extern "C" {
#endif

extern std::string GetNowTime();

static bool parse_request(std::string    &s_msg,
                          unsigned int   &n_id,
                          Json::Value    &j_root,
                          struct http_message *hm) {
  // mg_url_decode(hm->body.p, hm->body.len, p_data, n_data, 1);
  Json::Reader reader;
  bool b_ret = reader.parse(hm->body.p,
                            hm->body.p + hm->body.len,
                            j_root);
  if (false == b_ret) {
    return false;
  }

  if (!j_root["cmd"].isString()
      || !j_root["id"].isUInt()) {
    return false;
  }

  s_msg  = j_root["cmd"].asString();
  n_id   = j_root["id"].asUInt();
  //printf("cmd %s \t id %d\n body %s\n",
  //       s_msg.c_str(),
  //       n_id,
  //       j_root.toStyledString().c_str());
  return true;
}

static void send_response(struct mg_connection *nc,
                          const std::string    &s_msg,
                          int                   n_id,
                          int                   n_state,
                          const Json::Value    &j_body) {
  static const char kHttpContentType[] =
    "Content-Type: application/Json; charset=utf-8";

  Json::Value j_root;
  j_root["cmd"]   = s_msg;
  j_root["id"]    = n_id;
  j_root["state"] = n_state;
  j_root["body"]  = j_body;
  std::string response = j_root.toStyledString();

  mg_send_head(nc, 200,
               response.length(),
               kHttpContentType);
  mg_send(nc, response.c_str(), response.length());
}

// dispatch 测试 handle
// 设置设备地址
extern void uri_hdl_set_dev_addr(struct mg_connection *nc, int ev, void *ev_data);

// 注册消息
extern void uri_hdl_add_listen_msg(struct mg_connection *nc, int ev, void *ev_data);

// 取消注册消息
extern void uri_hdl_del_listen_msg(struct mg_connection *nc, int ev, void *ev_data);

// 发送请求
extern void uri_hdl_send_request(struct mg_connection *nc, int ev, void *ev_data);

// 发送回复
extern void uri_hdl_send_reply(struct mg_connection *nc, int ev, void *ev_data);

// 发送推送
extern void uri_hdl_send_message(struct mg_connection *nc, int ev, void *ev_data);

// 发送推送
extern std::list<Json::Value> k_message;
extern void uri_hdl_get_recv_msg(struct mg_connection *nc, int ev, void *ev_data);

#ifdef __cplusplus
}

}  // namespace web
#endif

#endif  // _URI_HANDLE_H
