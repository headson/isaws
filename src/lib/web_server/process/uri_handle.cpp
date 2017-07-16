/************************************************************************/
/* Author      : SoberPeng 2017-06-24
/* Description :
/************************************************************************/
#include "uri_handle.h"

#include <stdio.h>
#include <stdlib.h>

#include "vzbase/helper/stdafx.h"
#include "vzbase/core/vmessage.h"

#ifdef __cplusplus
namespace web {
extern "C" {
#endif

TAG_WEB_SESSION k_session[SESSION_COUNT];

TAG_WEB_SESSION * get_session(struct http_message *hm) {
  struct mg_str *cookie_header = mg_get_http_header(hm, "cookie");
  if (cookie_header == NULL)
    return NULL;
  char ssid[21];
  if (!mg_http_parse_header(cookie_header, SESSION_COOKIE_NAME,
                            ssid, sizeof(ssid))) {
    return NULL;
  }
  uint64_t sid = strtoull(ssid, NULL, 16);
  for (int i = 0; i < SESSION_COUNT; i++) {
    if (k_session[i].id == sid) {
      k_session[i].last_used = mg_time();
      return &k_session[i];
    }
  }
  return NULL;
}


void destroy_session(TAG_WEB_SESSION *s) {
  memset(s, 0, sizeof(*s));
}


TAG_WEB_SESSION * create_session(const char *usernmae, const struct http_message *hm) {
  /* Find first available slot or use the oldest one. */
  TAG_WEB_SESSION *s = NULL;
  TAG_WEB_SESSION *oldest_s = k_session;
  for (int i = 0; i < SESSION_COUNT; i++) {
    if (k_session[i].id == 0) {
      s = &k_session[i];
      break;
    }
    if (k_session[i].last_used < oldest_s->last_used) {
      oldest_s = &k_session[i];
    }
  }
  if (s == NULL) {
    destroy_session(oldest_s);
    printf("Evicted %" INT64_X_FMT "/%s\n",
           oldest_s->id, oldest_s->username);
    s = oldest_s;
  }

  /* Initialize new session. */
  s->created      = s->last_used = mg_time();
  strncpy(s->username, usernmae, USERNAME_SIZE);
  s->lucky_number = rand();
  /* Create an ID by putting various volatiles into a pot and stirring. */
  cs_sha1_ctx ctx;
  cs_sha1_init(&ctx);
  cs_sha1_update(&ctx, (const unsigned char *)hm->message.p, hm->message.len);
  cs_sha1_update(&ctx, (const unsigned char *)s, sizeof(*s));
  unsigned char digest[20];
  cs_sha1_final(digest, &ctx);
  s->id = *((uint64_t *)digest);
  return s;
}

void check_sessions(void) {
  double threshold = mg_time() - SESSION_TTL;
  for (int i = 0; i < SESSION_COUNT; i++) {
    TAG_WEB_SESSION *s = &k_session[i];
    if (s->id != 0 && s->last_used < threshold) {
      fprintf(stderr,
              "Session %" INT64_X_FMT " (%s) closed due to idleness.\n",
              s->id, s->username);
      destroy_session(s);
    }
  }
}

//////////////////////////////////////////////////////////////////////////
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

void send_response(struct mg_connection *nc,
                   const std::string    &s_msg,
                   int                   n_id,
                   int                   n_state,
                   const Json::Value    &j_body,
                   const std::string     extra_header) {
  static const std::string K_HTTP_CONTENT_TYPE =
    "Content-Type: application/Json; charset=utf-8";

  Json::Value j_root;
  j_root["cmd"]   = s_msg;
  j_root["id"]    = n_id;
  j_root["state"] = n_state;
  j_root["body"]  = j_body;
  std::string s_resp = j_root.toStyledString();

  mg_send_head(nc, 200,
               s_resp.length(),
               (K_HTTP_CONTENT_TYPE + extra_header).c_str());
  mg_send(nc, s_resp.c_str(), s_resp.length());
}

#ifdef __cplusplus
}
}  // namespace web
#endif
