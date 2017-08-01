/************************************************************************/
/* Author      : SoberPeng 2017-06-24
/* Description :
/************************************************************************/
#include "uri_handle.h"

#include <string>

#include "vzbase/helper/stdafx.h"
#include "vzbase/base/mysystem.h"

typedef struct _file_writer_data {
  FILE      *fp;
  size_t     bytes_written;
} file_writer_data;

#ifdef __cplusplus
namespace web {
extern "C" {
#endif

#ifdef _WIN32
#define UPLOAD_FILENAME "./upload.tar.gz"
#else  // _LINUX
#define UPLOAD_FILENAME "/tmp/upload.tar.gz"
#endif

void file_remove(const char *filename) {
  char scmd[256] = {0};
  snprintf(scmd, 1023,
           "rm -rf /tmp/bak; rm %s",
           filename);

  LOG(L_INFO) << scmd;
  vzbase::my_system(scmd);
}

int uncompress(const char *filename) {
  int nret = -3;

  // uncompress
  char scmd[256] = { 0 };
  snprintf(scmd, 255,
           "mkdir /tmp/bak;tar -zxf %s -C /tmp/bak",
           filename);
  LOG(L_INFO) << scmd;
  if (vzbase::my_system(scmd)) {
    LOG(L_ERROR) << "uncompress failed.";
    return -1;
  }

  //update.sh
  FILE* file = fopen("/tmp/bak/update.sh", "r");
  if (file) {
    memset(scmd, 0, 255);
    if (fgets(scmd, 255, file) != NULL) {
      std::string shw, suid;
      vzbase::get_hardware(shw, suid);
      int bcmp = strncmp(shw.c_str(), scmd, shw.size());
      LOG(L_INFO) << "shw " << shw
        << " scmd " << scmd << " cmp " << bcmp;
      if (0 == bcmp) {
        char *pcmd = NULL;
        do {
          memset(scmd, 0, 255);
          char *pcmd = fgets(scmd, 255, file);
          if (pcmd != NULL) {
            
            if (!vzbase::my_system(scmd)) {
              LOG(L_INFO) <<"success " << scmd;
              nret = 0;
            } else {
              LOG(L_ERROR) <<"failed " << scmd;
            }
          } else {
            break;
          }
        } while (true);
      } else {
        LOG(L_ERROR) << "check hareware failed.\n";
        nret = -2;
      }
    }

    fclose(file);
  }

  vzbase::my_system("sync");
  return nret;
}

void uri_hdl_upload(struct mg_connection *nc, int ev, void *p) {
  file_writer_data *data = (file_writer_data*)nc->user_data;
  struct mg_http_multipart_part *mp = (struct mg_http_multipart_part *) p;

  switch (ev) {
  case MG_EV_HTTP_PART_BEGIN:
    if (data == NULL) {
      data = (file_writer_data*)calloc(1, sizeof(file_writer_data));
      data->fp = fopen(UPLOAD_FILENAME, "wb+");
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
      int nret = uncompress(UPLOAD_FILENAME);
      file_remove(UPLOAD_FILENAME);

      char *pret = "upload success.";
      if (nret == -1) {
        pret = "uncompress failed.";
      } else if (nret == -2) {
        pret = "hardware compare failed.";
      } else if (nret == -3) {
        pret = "have no command success.";
      }
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
