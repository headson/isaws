/************************************************************************/
/* Author      : SoberPeng 2017-06-24
/* Description :
/************************************************************************/
#ifndef _URI_HANDLE_H
#define _URI_HANDLE_H

#include "basictypes.h"
#include "web/base/mongoose.h"

typedef int32 (*URI_HANDLE_METHOD)(struct mg_connection *nc,
                                   const void           *p_data,
                                   uint32                n_data);

#ifdef __cplusplus
extern "C" {
#endif

// ��½��֤
extern void uri_hdl_login(struct mg_connection *nc, int ev, void *ev_data);

#ifdef __cplusplus
}
#endif

#endif  // _URI_HANDLE_H