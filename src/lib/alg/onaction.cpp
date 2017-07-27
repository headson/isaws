/************************************************************************/
/* Author      : SoberPeng 2017-07-26
/* Description :
/************************************************************************/
#include "clistenmessage.h"

#include "json/json.h"

#include "vzbase/base/vmessage.h"

void alg::CListenMessage::AlgActionCallback(sdk_iva_output_info *paction) {
  Json::Value jroot;
  jroot[MSG_CMD] = MSG_CATCH_EVENT;

  if (paction->event_type == IVA_EVENT_COUTING) {
    jroot[MSG_BODY]["positive_number"] = paction->positive_number;
    jroot[MSG_BODY]["negative_number"] = paction->negative_number;

    Json::FastWriter jfw;
    std::string sjson = jfw.write(jroot);
    DpClient_SendDpMessage(MSG_CATCH_EVENT, 0,
                           sjson.c_str(), sjson.size());
  }
}