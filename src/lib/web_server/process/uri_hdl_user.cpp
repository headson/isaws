/************************************************************************/
/* Author      : SoberPeng 2017-06-24
/* Description :
/************************************************************************/
#include "uri_handle.h"

#include "vzbase/helper/stdafx.h"

#include <vector>
#include <iostream>

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
  int n_state = 6;
  std::string s_msg = "";
  unsigned int n_id = 0;
  Json::Value j_req, j_resp;

  if (!parse_request(s_msg, n_id, j_req, (struct http_message *)ev_data)) {
    LOG(L_ERROR) << "parse body data failed.";
    return;
  }

  // 处理部分

  send_response(nc, s_msg, n_id, n_state, j_resp);
}

#ifdef __cplusplus
}
}  // namespace web
#endif



