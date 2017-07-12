/************************************************************************/
/* Author      : SoberPeng 2017-06-24
/* Description :
/************************************************************************/
#ifndef _URI_HANDLE_H
#define _URI_HANDLE_H

#include "vzbase/base/basictypes.h"

#include <string>
#include <json/json.h>

#include "web_server/base/mongoose.h"

#ifdef __cplusplus
namespace web {
extern "C" {
#endif

extern bool parse_request(std::string         &s_msg,
                          unsigned int        &n_id,
                          Json::Value         &j_root,
                          struct http_message *hm);

extern void send_response(struct mg_connection *nc,
                          const std::string    &s_msg,
                          int                   n_id,
                          int                   n_state,
                          const Json::Value    &j_body);

///用户操作////////////////////////////////////////////////////////////////
// 登陆验证
extern void uri_hdl_login(struct mg_connection *nc, int ev, void *ev_data);

// 用户添加
extern void uri_hdl_user_add(struct mg_connection *nc, int ev, void *ev_data);

// 用户删除
extern void uri_hdl_user_del(struct mg_connection *nc, int ev, void *ev_data);

// 用户修改
extern void uri_hdl_user_cfg(struct mg_connection *nc, int ev, void *ev_data);

///时间配置////////////////////////////////////////////////////////////////
// 手动矫正时间
extern void uri_hdl_time_cfg(struct mg_connection *nc, int ev, void *ev_data);

///时间配置////////////////////////////////////////////////////////////////
// 手动矫正时间
extern void uri_hdl_time_cfg(struct mg_connection *nc, int ev, void *ev_data);

#ifdef __cplusplus
}
}  // namespace web
#endif

#endif  // _URI_HANDLE_H
