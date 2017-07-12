/************************************************************************/
/* Author      : SoberPeng 2017-06-24
/* Description :
/************************************************************************/
#include "uri_handle.h"

#ifdef __cplusplus
namespace web {
extern "C" {
#endif

bool parse_request(std::string &s_msg,
                   unsigned int &n_id,
                   Json::Value &j_root,
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

  s_msg = j_root["cmd"].asString();
  n_id  = j_root["id"].asUInt();
  printf("cmd %s \t id %d\n body %s\n",
         s_msg.c_str(),
         n_id,
         j_root.toStyledString().c_str());
  return true;
}

void web::send_response(struct mg_connection  *nc,
                        const std::string     &s_msg,
                        int                    n_id,
                        int                    n_state,
                        const Json::Value     &j_body) {
  static const char K_HTTP_CONTENT_TYPE[] =
    "Content-Type: application/Json; charset=utf-8";

  Json::Value j_root;
  j_root["cmd"]   = s_msg;
  j_root["id"]    = n_id;
  j_root["state"] = n_state;
  j_root["body"]  = j_body;
  std::string s_resp = j_root.toStyledString();

  mg_send_head(nc, 200,
               s_resp.length(),
               K_HTTP_CONTENT_TYPE);
  mg_send(nc, s_resp.c_str(), s_resp.length());
}


#ifdef __cplusplus
}
}  // namespace web
#endif
