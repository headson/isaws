/************************************************************************
*Author      : 17-09-12
*Description :
************************************************************************/
#ifndef LIBDPCLIENT_DPLOGGING_H_
#define LIBDPCLIENT_DPLOGGING_H_

#include "vzbase/base/basictypes.h"

#include <string>

////////////////////////////////////////////////////////////////////////////////
// ID Define
#define LOGGING_ID_START    0XA0

#define ID_HTTP_SEND        (LOGGING_ID_START | 0X01)
#define ID_TCP_SERVER       (LOGGING_ID_START | 0X02)
#define ID_EVENT_SERVER     (LOGGING_ID_START | 0X03)
#define ID_BOA              (LOGGING_ID_START | 0X04)
#define ID_SYSTEM_SERVER    (LOGGING_ID_START | 0X05)
#define ID_AV_SERVER        (LOGGING_ID_START | 0X06)
#define ID_BUSINESS_SERVER  (LOGGING_ID_START | 0X07)

//////////////////////////////////////////////////////////////////////////
#define OPTIMIZE_LOGGING          "OPTIMIZE_LOGGING"
#define DP_LOGGING_ENABLE         "DP_LOGGING_ENABLE"
#define LOGGING_SERVIES           "LOGGING_SERVIES"
#define VZLOGGING_SETTINGS        "SETFLAG_LOGGING" 

#define DP_LOGGING_DISABLE_STATUS 0
#define DP_LOGGING_ENABLE_STATUS  1

#define MAX_DPLOGGING_TEXT_SIZE   255

typedef struct _OptimizeLogging {
  uint64          millisec;
  unsigned int    event_id;
  unsigned char   proc_id;
  unsigned short  type;
  unsigned char   msg_size;
  char            msg[0];
} OptimizeLogging;

#ifdef __cplusplus
extern "C" {
#endif

const std::string DumpBinary(const char *data, std::size_t size);

int WriteDpLoggingMessage(unsigned char  proc_id,
                          unsigned short type,
                          unsigned int   msg_size,
                          const char    *msg,
                          unsigned int   event_id);

#ifdef __cplusplus
}
#endif

#endif  // LIBDPCLIENT_DPLOGGING_H_
