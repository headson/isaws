/************************************************************************/
/* Author      : SoberPeng 2017-06-24
/* Description :
/************************************************************************/
#include "uri_handle.h"

#include <string>

#include "vzbase/helper/stdafx.h"
#include "vzbase/base/mysystem.h"
#include "vzbase/base/vmessage.h"

typedef struct _file_writer_data {
  FILE      *fp;
  size_t     bytes_written;
} file_writer_data;

#ifdef __cplusplus
namespace web {
extern "C" {
#endif

void uri_hdl_upload(struct mg_connection *nc, int ev, void *p) {
  file_writer_data *data = (file_writer_data*)nc->user_data;
  struct mg_http_multipart_part *mp = (struct mg_http_multipart_part *) p;

  switch (ev) {
  case MG_EV_HTTP_PART_BEGIN:
    if (data == NULL) {
      data = (file_writer_data*)calloc(1, sizeof(file_writer_data));
      data->fp = fopen(DEF_UPLOAD_FILENAME, "wb+");
      data->bytes_written = 0;

      if (data->fp == NULL) {
        mg_printf(nc, "%s",
                  "HTTP/1.1 500 Failed to open a file\r\n"
                  "Content-Length: 0\r\n\r\n");
        nc->flags |= MG_F_SEND_AND_CLOSE;
        return;
      }
      nc->user_data = (void *) data;
    }
    break;
  case MG_EV_HTTP_PART_DATA:
    if (data) {
      if (fwrite(mp->data.p, 1, mp->data.len, data->fp) != mp->data.len) {
        mg_printf(nc, "%s",
                  "HTTP/1.1 500 Failed to write to a file\r\n"
                  "Content-Length: 0\r\n\r\n");
        nc->flags |= MG_F_SEND_AND_CLOSE;
        return;
      }
      data->bytes_written += mp->data.len;
    }
    break;
  case MG_EV_HTTP_PART_END:
    if (data) {
      fclose(data->fp);

      // 处理接收完成
      int nret = vzbase::update_file_uncompress(DEF_UPLOAD_FILENAME);
      vzbase::file_remove(DEF_UPLOAD_FILENAME);

      const char *pret = "upload success.";
      if (nret == 0) {
        DpClient_SendDpMessage(MSG_REBOOT_DEVICE, 0, NULL, 0);
      } else if (nret == -1) {
        pret = "uncompress failed.";
      } else if (nret == -2) {
        pret = "hardware compare failed.";
      } else if (nret == -3) {
        pret = "have no command success.";
      }
      LOG(L_ERROR) << pret;

      mg_printf(nc,
                "HTTP/1.1 200 OK\r\n"
                "Content-Type: text/plain\r\n"
                "Connection: close\r\n\r\n"
                "%s\n\n", pret);
      nc->flags |= MG_F_SEND_AND_CLOSE;
      nc->user_data = NULL;
      free(data);
    }
    break;
  }
}

#ifdef __cplusplus
}
}  // namespace web
#endif
