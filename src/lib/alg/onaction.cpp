/************************************************************************/
/* Author      : SoberPeng 2017-07-26
/* Description :
/************************************************************************/
#include "clistenmessage.h"
#include "alg/basedefine.h"

#include "json/json.h"
#include "vzbase/base/vmessage.h"
#include "vzbase/helper/stdafx.h"

void alg::CListenMessage::AlgActionCallback(sdk_iva_output_info *paction) {
  Json::Value jroot;
  jroot[MSG_CMD] = MSG_CATCH_EVENT;

  if (paction->event_type == IVA_EVENT_COUTING) {
    jroot[MSG_BODY][ALG_EVT_OUT_TIME_MS] = vzbase::CurrentTimet();
    jroot[MSG_BODY][ALG_POSITIVE_NUMBER] = paction->positive_number;
    jroot[MSG_BODY][ALG_NEGATIVE_NUMBER] = paction->negative_number;

    Json::FastWriter jfw;
    std::string sjson = jfw.write(jroot);
    DpClient_SendDpMessage(MSG_CATCH_EVENT, 0,
                           sjson.c_str(), sjson.size());
  }
  LOG(L_INFO) << "something is happened.";
}