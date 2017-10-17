/************************************************************************
*Author      : Sober.Peng 17-09-12
*Description :
************************************************************************/
#include "dplogging.h"
#include "dpclient_c.h"

#include "vzbase/helper/stdafx.h"

#ifndef min
#define min(a,b)  (((a) < (b)) ? (a) : (b))
#endif

uint64 GetVzTime() {
  struct timeval tv;
  gettimeofday(&tv, NULL);
  return (uint64)tv.tv_sec * 1000000 + (uint64)tv.tv_usec;
}

const std::string DumpBinary(const char *data, std::size_t size) {
  std::string result;
  for (std::size_t i = 0; i < size; i++) {
    char temp[4] = {0};
    sprintf(temp, "%X ", (int)(data[i]));
    result += temp;
  }
  LOG(L_INFO) << result;
  return result;
}

int WriteDpLoggingMessage(unsigned char proc_id, unsigned short type,
                          unsigned int msg_size, const char *msg,
                          unsigned int event_id) {
  char buffer[sizeof(OptimizeLogging) + MAX_DPLOGGING_TEXT_SIZE  + 1] = {0};
  OptimizeLogging *pol = (OptimizeLogging *)(buffer);
  pol->millisec = GetVzTime();
  pol->event_id = event_id;
  pol->proc_id  = proc_id;
  pol->type     = type;
  pol->msg_size = (msg_size < MAX_DPLOGGING_TEXT_SIZE) ? msg_size : MAX_DPLOGGING_TEXT_SIZE;
  if (pol->msg_size && msg) {
    memcpy(buffer + sizeof(OptimizeLogging), msg, pol->msg_size);
    LOG(L_INFO).write(msg, msg_size);
  }
  int ret =  DpClient_SendDpMessage(OPTIMIZE_LOGGING,
                                    0,
                                    buffer,
									sizeof(OptimizeLogging) + pol->msg_size);
  return ret;
}

