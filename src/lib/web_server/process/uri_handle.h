/************************************************************************/
/* Author      : SoberPeng 2017-06-24
/* Description :
/************************************************************************/
#ifndef _URI_HANDLE_H
#define _URI_HANDLE_H

#include "vzbase/base/basictypes.h"

#include <string>

#include "json/json.h"

#include "web_server/base/mongoose.h"
#include "dispatcher/sync/dpclient_c.h"

#ifdef __cplusplus
namespace web {
extern "C" {
#endif

#define SESSION_COOKIE_NAME     "mgs"
#define SESSION_TTL             30.0
#define SESSION_CHECK_INTERVAL  5.0
#define USERNAME_SIZE           32

typedef struct _TAG_WEB_SESSION {
  uint64 id;
  double created;
  double last_used;
  char   username[USERNAME_SIZE];
  int32  lucky_number;
} TAG_WEB_SESSION;

#define SESSION_COUNT 10
extern TAG_WEB_SESSION k_session[SESSION_COUNT];

extern TAG_WEB_SESSION *create_session(const char *usernmae,
                                       const struct http_message *hm);
extern void             destroy_session(TAG_WEB_SESSION *s);

extern TAG_WEB_SESSION *get_session(struct http_message *hm);
extern TAG_WEB_SESSION *get_session_by_sid(const char* ssid);
extern void             check_sessions(void);

//////////////////////////////////////////////////////////////////////////
extern bool parse_request(std::string         &smsg,
                          unsigned int        &nid,
                          Json::Value         &jroot,
                          struct http_message *hm);

extern void send_response(struct mg_connection *nc,
                          const std::string    &smsg,
                          int                   nid,
                          int                   nstate,
                          const Json::Value    &jbody,
                          const std::string     extra_header="");

///用户操作///////////////////////////////////////////////////////////////
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

///转发///////////////////////////////////////////////////////////////////
extern void uri_hdl_dispath(struct mg_connection *nc, int ev, void *ev_data);

///上传///////////////////////////////////////////////////////////////////////
extern void uri_hdl_upload(struct mg_connection *nc, int ev, void *p);

#ifdef __cplusplus
}
}  // namespace web
#endif

#endif  // _URI_HANDLE_H
