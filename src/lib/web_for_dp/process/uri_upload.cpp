/************************************************************************/
/* Author      : SoberPeng 2017-06-24
/* Description :
/************************************************************************/
#include "uri_handle.h"

#include "stdafx.h"

struct file_writer_data {
  FILE 		*fp;
  size_t 	 bytes_written;
};

static void handle_request(struct mg_connection *nc) {
  // This handler gets for all endpoints but /upload
  mg_printf(nc, "%s",
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: text/html\r\n"
            "Connection: close\r\n"
            "\r\n"
            "<html><body>Upload example."
            "<form method=\"POST\" action=\"/upload\" "
            "  enctype=\"multipart/form-data\">"
            "<input type=\"file\" name=\"file\" /> <br/>"
            "<input type=\"submit\" value=\"Upload\" />"
            "</form></body></html>");
  nc->flags |= MG_F_SEND_AND_CLOSE;
}

static void handle_upload(struct mg_connection *nc, int ev, void *p) {
  struct file_writer_data *data = (struct file_writer_data *) nc->user_data;
  struct mg_http_multipart_part *mp = (struct mg_http_multipart_part *) p;

  switch (ev) {
  case MG_EV_HTTP_PART_BEGIN: {
    if (data == NULL) {
      data = calloc(1, sizeof(struct file_writer_data));
      data->fp = tmpfile();
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
  }
  case MG_EV_HTTP_PART_DATA: {
    if (fwrite(mp->data.p, 1, mp->data.len, data->fp) != mp->data.len) {
      mg_printf(nc, "%s",
                "HTTP/1.1 500 Failed to write to a file\r\n"
                "Content-Length: 0\r\n\r\n");
      nc->flags |= MG_F_SEND_AND_CLOSE;
      return;
    }
    data->bytes_written += mp->data.len;
    break;
  }
  case MG_EV_HTTP_PART_END: {
    mg_printf(nc,
              "HTTP/1.1 200 OK\r\n"
              "Content-Type: text/plain\r\n"
              "Connection: close\r\n\r\n"
              "Written %ld of POST data to a temp file\n\n",
              (long) ftell(data->fp));
    nc->flags |= MG_F_SEND_AND_CLOSE;
    fclose(data->fp);
    free(data);
    nc->user_data = NULL;
    break;
  }
  }
}