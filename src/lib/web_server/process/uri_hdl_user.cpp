/************************************************************************/
/* Author      : SoberPeng 2017-06-24
/* Description :
/************************************************************************/
#include "uri_handle.h"

#include "vzbase/helper/stdafx.h"

#include "vzbase/base/base64.h"
#include "vzbase/core/vmessage.h"

#include <vector>
#include <iostream>

#include "dispatcher/sync/dpclient_c.h"

#ifdef __cplusplus
namespace web {
extern "C" {
#endif

//mg_printf(nc, "<HTML><head>"
//          "<meta http-equiv=\"Refresh\" content=\"0;URL=%s\">"
//          "</head><body>%s</body></HTML>",
//          "http://127.0.0.1:8000/main.html",
//          "{\"resp\":\"all ok\"}");

void uri_hdl_login(struct mg_connection *nc, int ev, void *ev_data) {
  int nstate = RET_JSON_PARSE;

  std::string smsg = "";
  unsigned int nid = 0;
  Json::Value jreq, jresp;

  if (!parse_request(smsg, nid, jreq, (struct http_message *)ev_data)) {
    LOG(L_ERROR) << "parse body data failed.";
    return;
  }

  // 处理部分
  Json::Reader jread;
  Json::Value  juser(Json::arrayValue);

  std::string suser = "";
  int nuser = Kvdb_GetKeyToString(KVDB_KEY_USER, strlen(KVDB_KEY_USER),
                                  &suser);
  if (nuser <= 0 || suser.empty()) {
    // 生成默认用户
    juser[0]["username"] = "admin";
    juser[0]["password"] = vzbase::Base64::Encode("admin");
    suser = juser.toStyledString();

    Kvdb_SetKey(KVDB_KEY_USER, strlen(KVDB_KEY_USER),
                suser.c_str(), suser.size());

    suser = "";
    nuser = Kvdb_GetKeyToString(KVDB_KEY_USER, strlen(KVDB_KEY_USER),
                                &suser);
  }

  std::string username = "";
  std::string password = "";
  if (jread.parse(suser, juser)) {
    if (jreq[MSG_BODY]["username"].isString() &&
        jreq[MSG_BODY]["password"].isString()) {
      username = jreq[MSG_BODY]["username"].asString();
      password = jreq[MSG_BODY]["password"].asString();
    }

    if (!username.empty() && !password.empty()) {
      //从对象数组中找到想要的对象
      for(unsigned int i = 0; i < juser.size(); i++) {
        if (juser[i]["username"].isString() &&
            juser[i]["password"].isString()) {
          if (juser[i]["username"].asString() == username &&
              juser[i]["password"].asString() == password) {
            nstate = RET_SUCCESS;
            break;
          }
        }
      }
    }
  }
  std::string extra_header = "";
  if (nstate == RET_SUCCESS) {
    TAG_WEB_SESSION *s = create_session(
                           username.c_str(),
                           (struct http_message *)ev_data);

    char shead[100];
    snprintf(shead, sizeof(shead),
             "Set-Cookie: %s=%" INT64_X_FMT "; path=/",
             SESSION_COOKIE_NAME, s->id);
    extra_header = shead;
  }

  send_response(nc, smsg, nid, nstate, jresp, extra_header);
}

#ifdef __cplusplus
}
}  // namespace web
#endif



