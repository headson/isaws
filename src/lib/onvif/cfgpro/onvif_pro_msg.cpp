#include <string>
#include "onvif/cfgpro/onvif_pro_msg.h"
#include "onvif/cfgpro/onvif_cfgfile_mng.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
//#include <Msg_Def.h>
#include <sys/ipc.h>
#include "json/json.h"
#include "ipc_comm/SystemServerComm.h"
#include "onvif/base/basedefine.h"

using namespace onvifserver;
#define SYSTEM_SERVER_BUFFER_LENGTH	256
#define ETH_NAME			"eth0"
//#define DEBUG
#ifdef DEBUG
#define DBG(fmt, args...)	printf("Debug " fmt, ##args)
#else
#define DBG(fmt, args...)
#endif

#define ERR(fmt, args...)	printf("Error " fmt, ##args)

ONVIF_STATE_RET OnDealIpChange(unsigned int *pdata, std::string &errormsg) {
  ONVIF_STATE_RET ret = STATE_OK;
  errormsg = OK;
  if (UpdateOnvifState() < 0) {
    ret = STATE_ERR_SERVER_ERR;
    errormsg = Failed;
  } else {
    UpdateNewOnvifIPFlag(pdata);
  }
  return ret;
}

void RegisterDpMessage(DPPollHandle p_hdl) {
  const char *IVS_CMD[] = {
    SYS_SRV_IP_CHANGED,
    SYS_SRV_NETPORT_CHANGED,
  };

  int cmd_size =	sizeof(IVS_CMD)/sizeof(char *);
  DpClient_HdlAddListenMessage(p_hdl, IVS_CMD, cmd_size);
}

void OnDpState(DPPollHandle p_hdl,
               uint32 n_state,
               void *p_user_arg) {
  if (n_state == DP_CLIENT_DISCONNECT) {
    DpClient_HdlReConnect(p_hdl);
    RegisterDpMessage(p_hdl);
  }
}

ONVIF_STATE_RET OnDealPortChange(std::string &errormsg) {
  ONVIF_STATE_RET ret = STATE_OK;
  errormsg = OK;
  if (UpdateOnvifState() < 0) {
    ret = STATE_ERR_SERVER_ERR;
    errormsg = Failed;
  }
  return ret;
}


void OnDpMessage(DPPollHandle p_hdl,
                 const DpMessage *dp_msg,
                 void *p_usr_arg) {
  int need_reply = 0;
  ONVIF_STATE_RET sysret = STATE_OK;
  int havebody = 0;
  std::string errmsg = ONVIF_OK_STR;
  std::string subreply;
  std::string replystr;
  printf("onvif_pro_msg Start!\n");
  printf("dp_msg->method is %s\n",dp_msg->method);

  if (strcmp(dp_msg->method, SYS_SRV_IP_CHANGED) == 0) {
    unsigned int data[3] = {0};
    if (dp_msg->data_size == sizeof(data)) {
      memcpy(data,dp_msg->data,dp_msg->data_size);
      sysret = OnDealIpChange(data,errmsg);
    } else {
      sysret = STATE_ERR_INVALID_VALUE;
      errmsg = ONVIF_FAIL_STR;
    }
  } else if (strcmp(dp_msg->method,SYS_SRV_NETPORT_CHANGED) == 0) {
    sysret = OnDealPortChange(errmsg);
  }	else {
    printf("Unknown message Msg:%s\n",dp_msg->method);
    sysret = STATE_ERR_INVALID_VALUE;
    errmsg = ONVIF_UnsupportedCmd;
    need_reply = 1;
  }

  if (need_reply) {
    Json::Value replyvalue;
    Json::FastWriter fw;
    replyvalue[ONVIF_ERROR_MSG] = errmsg;
    replyvalue[ONVIF_STATE_CODE] = sysret;
    if(havebody && (sysret == STATE_OK)) {
      replyvalue[ONVIF_BODY] = subreply;
    }

    replystr = fw.write(replyvalue);


    DpClient_SendDpReply(dp_msg->method, dp_msg->channel_id, dp_msg->id,
                         (char*)replystr.c_str(), replystr.size());
  }
}

