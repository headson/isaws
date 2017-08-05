/************************************************************************/
/* Author      : SoberPeng 2017-06-24
/* Description :
/************************************************************************/
#include "uri_handle.h"

#include "vzbase/helper/stdafx.h"

#include "vzbase/base/base64.h"
#include "vzbase/base/vmessage.h"

#include <vector>
#include <iostream>

#include "dispatcher/sync/dpclient_c.h"

#ifdef __cplusplus
namespace web {
extern "C" {
#endif

void uri_hdl_dispatch(struct mg_connection *nc, int ev, void *ev_data) {
  int nstate = RET_JSON_PARSE;

  std::string smsg = "";
  unsigned int nid = 0;
  Json::Value jreq, jresp;

  TAG_WEB_SESSION *pses = get_session((struct http_message*)ev_data);

  if (!parse_request(smsg, nid, jreq, (struct http_message *)ev_data)) {
    LOG(L_ERROR) << "parse body data failed.";
    return;
  }

  std::string sresp = "";
  std::string sjson = jreq.toStyledString();
  int ret = DpClient_SendDpReqToString(smsg.c_str(),
                                       0,
                                       sjson.c_str(),
                                       sjson.size(),
                                       &sresp,
                                       DEF_TIMEOUT_MSEC);
  if (ret == VZNETDP_SUCCEED) {
    Json::Reader jread;
    Json::Value  jroot;
    if (jread.parse(sresp, jroot)) {
      jresp = jroot[MSG_BODY];
      nstate = RET_SUCCESS;
    }
  } else {
    nstate = RET_DP_REPLY_FAILED;
  }

  send_response(nc, smsg, nid, nstate, jresp);
}

#ifdef __cplusplus
}
}  // namespace web
#endif



