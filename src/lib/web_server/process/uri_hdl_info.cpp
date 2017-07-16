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

void uri_hdl_get_info(struct mg_connection *nc, int ev, void *ev_data) {
  int nstate = RET_JSON_PARSE;

  std::string smsg = "";
  unsigned int nid = 0;
  Json::Value jreq, jresp;

  if (!parse_request(smsg, nid, jreq, (struct http_message *)ev_data)) {
    LOG(L_ERROR) << "parse body data failed.";
    return;
  }

  std::string sresp = "";
  std::string sjson = jreq[MSG_BODY].toStyledString();
  int ret = DpClient_SendDpRequest(MSG_SYSC_GET_INFO, 0,
                                   sjson.c_str(), sjson.size(),
                                   dpc_msg_callback, &sresp,
                                   DEF_TIMEOUT_MSEC);
  if (ret == VZNETDP_SUCCEED) {
    Json::Reader jread;
    if (jread.parse(sresp, jresp)) {
      jresp = jresp[MSG_BODY];
    }
  }

  send_response(nc, smsg, nid, nstate, jresp);
}

#ifdef __cplusplus
}
}  // namespace web
#endif



