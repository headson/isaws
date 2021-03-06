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
  if (paction->event_type == IVA_EVENT_COUTING) {
    static int last_positive_number = 0;
    static int last_negative_number = 0;
    if (last_positive_number == paction->positive_number &&
        last_negative_number == paction->negative_number) {
      // 没有人数统计,不上传信息
      return;
    }

    jroot[MSG_CMD] = MSG_CATCH_EVENT;
    jroot[MSG_BODY][ALG_EVT_OUT_TIMET] = vzbase::CurrentTimet();
    jroot[MSG_BODY][ALG_POSITIVE_NUMBER] = paction->positive_number;
    //jroot[MSG_BODY][ALG_POSITIVE_ADD_NUM] = paction->positive_number - last_positive_number;
    jroot[MSG_BODY][ALG_NEGATIVE_NUMBER] = paction->negative_number;
    //jroot[MSG_BODY][ALG_NEGATIVE_ADD_NUM] = paction->negative_number - last_negative_number;

    Json::FastWriter jfw;
    std::string sjson = jfw.write(jroot);
    int res = DpClient_SendDpMessage(MSG_CATCH_EVENT, 0,
                                     sjson.c_str(), sjson.size());
    if (VZNETDP_FAILURE == res) {
      // 无论什么原因,send失败,再传一次
      /*DpClient_SendDpMessage(MSG_CATCH_EVENT, 0,
                             sjson.c_str(), sjson.size());*/
    }

    last_positive_number = paction->positive_number;
    last_negative_number = paction->negative_number;
    LOG(L_INFO) << "something is happened."
                << " positive_number " << last_positive_number
                << " negative_number " << last_negative_number;
  } else if (paction->event_type == IVA_EVENT_SET_DAY_MODE
             || paction->event_type == IVA_EVENT_SET_NIGHT_MODE) {
    Json::Value jreq;
    jreq[MSG_CMD] = MSG_IRCUT_CTRLS;
    jreq[MSG_ID]  = 1;
    if (paction->event_type == IVA_EVENT_SET_DAY_MODE) {
      jreq[MSG_BODY]["switch"] = 0;
    } else {
      jreq[MSG_BODY]["switch"] = 1;
    }
    std::string sreq = jreq.toStyledString();
    std::string sresp = "";
    int res = DpClient_SendDpReqToString(MSG_IRCUT_CTRLS, 0,
                                         sreq.c_str(), sreq.size(),
                                         &sresp, DEF_TIMEOUT_MSEC);
    if (res == VZNETDP_SUCCEED) {
      Json::Value jresp;
      Json::Reader jparse;
      if (!jparse.parse(sresp, jresp)) {
        LOG(L_ERROR) << "";
        return;
      }
      if (jresp[MSG_STATE].asInt() == RET_SUCCESS) {
        LOG(L_INFO) << "ircut control success";
      } else {
        LOG(L_INFO) << "ircut control failed";
      }
    }
  }
}
