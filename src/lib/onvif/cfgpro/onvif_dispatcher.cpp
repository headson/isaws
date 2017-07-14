#include "onvif/cfgpro/onvif_dispatcher.h"
#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include <stdint.h>
#include <unistd.h>
#include <sysctrl.h>
#include <fstream>
//#include <ipc_comm/OnvifServerComm.h>
#include <ipc_comm/SystemServerComm.h>
#include "onvif/cfgpro/onvif_pro_msg.h"
#include <string>
#include <arpa/inet.h>
#include "dispatcher/sync/dpclient_c.h"
using namespace std;

const unsigned REQ_TIMEOUT = 3; //请求超时时间（秒）
const unsigned REQ_WAIT_GAP = 10;   //请求后等待时的查询间隔（毫秒）

#define ONVIF_DISP_DBG
#ifdef ONVIF_DISP_DBG
#define _DBG(fmt, args...) fprintf(stderr, "[%d]: "fmt, __LINE__,##args)
#else
#define _DBG(fmt, args...)
#endif

#define _ERR(fmt, args...) fprintf(stderr, "[err]: "fmt, ##args)

//启动分发机制
IPC_DISP_RET OnvifDisp_Start() {
  DpClient_Init("127.0.0.1", 5291);
  DpClient_Start(true);
}

//停止分发机制
void OnvifDisp_Stop() {
  DpClient_Stop();
}

struct ReqMessageBuffer {
  void   *buffer;
  uint32 buffer_size;
  uint32 message_size;
};

void DpMessageCallback(DPPollHandle p_hdl,
                       const DpMessage *dmp,
                       void* p_usr_arg) {
  ReqMessageBuffer *buffer = ((ReqMessageBuffer *)p_usr_arg);
  if (buffer == NULL || dmp->data_size > buffer->buffer_size) {
    return;
  }
  buffer->message_size = dmp->data_size;
  memcpy(buffer->buffer, dmp->data, dmp->data_size);
}

int OnvifDisp_SendRequestSync(const char* msg_type,
                              const char *buffer,
                              int len,
                              void *rbuffer,
                              int rbuffer_len) {
  ReqMessageBuffer req_buffer;
  req_buffer.buffer = rbuffer;
  req_buffer.buffer_size = rbuffer_len;
  req_buffer.message_size = 0;
  return DpClient_SendDpRequest(msg_type,
                                0,
                                buffer,
                                len,
                                DpMessageCallback,
                                &req_buffer,
                                5000);
}



