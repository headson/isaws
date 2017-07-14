#ifndef __ONVIF_SERVER_PRO_MSG_H__
#define __ONVIF_SERVER_PRO_MSG_H__

#include <ipc_comm/OnvifServerComm.h>
#include <ipc_comm/SystemServerComm.h>
#include "dispatcher/sync/dpclient_c.h"

#ifdef __cplusplus
extern "C"
{
#endif

void OnDpMessage(DPPollHandle p_hdl,
                 const DpMessage* dp_msg,
                 void* user_data);

void OnDpState(DPPollHandle p_hdl,
               uint32 n_state,
               void *p_user_arg);

#ifdef __cplusplus
}
#endif


#endif
