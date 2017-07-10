/************************************************************************/
/* Author      : SoberPeng 2017-06-24
/* Description :
/************************************************************************/
#include "uri_handle.h"

#include "vzbase/helper/stdafx.h"

#include <vector>
#include <iostream>

void uri_hdl_login(struct mg_connection *nc, int ev, void *ev_data) {
  struct http_message *hm = (struct http_message *)ev_data;

  char ss[1024] = {0};
  mg_get_http_var(&hm->body, "json", ss, 1024);

  mg_printf(nc, "<HTML><head>"
    "<meta http-equiv=\"Refresh\" content=\"0;URL=%s\">"
    "</head><body>%s</body></HTML>",
    "http://127.0.0.1:8000/main.html",
    "{\"resp\":\"all ok\"}");
  nc->flags |= MG_F_SEND_AND_CLOSE;
}



