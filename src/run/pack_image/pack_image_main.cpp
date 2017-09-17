#include <stdio.h>
#include <string.h>

#include <string>

#ifdef _WIN32
#include "getopt.h"
#else
#include <getopt.h>
#endif

typedef struct {
  unsigned int    mark;       // 0xAA55AA55
  char            head[32];   //
  struct {
    char          fname[32];  // 文件名
    unsigned int  fpos;       // 写偏移
    unsigned int  fsize;      // 文件大小
    unsigned int  version;    // 文件版本
  } fimg[16];
} TAG_HEADER;

int main(int argc, char* argv[]) {
  std::string img_path = "c://tools//image//";
  std::string ini_path = "c://tools//image//offset.ini";
  std::string out_path = "c://tools//image//all_bin.img";

  int opt = 0;
  while ((opt = getopt(argc, argv, "o:i:f:")) != -1) {
    switch (opt) {
    case 'o':
      out_path = optarg;
      break;

    case 'i':
      ini_path = optarg;
      break;

    case 'f':
      img_path = optarg;
      break;
    }
  }

  TAG_HEADER head;
  memset(&head, 0, sizeof(head));

  int abs_pos = 4*1024;
  FILE *ini_file = fopen(ini_path.c_str(), "rt");
  FILE *out_file = fopen(out_path.c_str(), "wb");
  for (int i = 0; i < 16 &&
       ini_file && out_file; i++) {
    char spos[256] = {0};
    fgets(spos, 255, ini_file);

    if (strlen(spos) > 0) {
      sscanf(spos, "%s\t%d\t%d",
             head.fimg[i].fname,
             &head.fimg[i].fpos,
             &head.fimg[i].version);

      if (head.fimg[i].fpos > 0 &&
          strlen(head.fimg[i].fname) > 0) {
        // 移动到写文件地址
        fseek(out_file, abs_pos, SEEK_SET);

        std::string sf = img_path;
        sf  +=  head.fimg[i].fname;
        printf("%s file write pos %d\t", sf.c_str(), abs_pos);

        FILE *img_file = fopen(sf.c_str(), "rb+");
        if (img_file) {
          fwrite(head.fimg[i].fname, 1, 32, out_file);
        }
        head.fimg[i].fsize = 0;
        while (img_file) {
          char sdata[1024] = {0};
          // 读镜像文件
          int ndata = fread(sdata, 1, 1024, img_file);
          if (ndata <= 0) {
            break;
          }

          // 写入到输出文件
          fwrite(sdata, 1, ndata, out_file);
          head.fimg[i].fsize += ndata;
        }
        printf("file size %d-%d.\n", head.fimg[i].fsize, head.fimg[i].fpos);
        if (img_file) {
          fclose(img_file);
          img_file = NULL;
        }

        // 下一个文件地址
        abs_pos += head.fimg[i].fpos * 1024;
      }

    } else {
      break;
    }
  }
  if (ini_file) {
    fclose(ini_file);
    ini_file = NULL;
  }
  if (out_file) {
    // 结束写头
    fseek(out_file, 0, SEEK_SET);
    head.mark = 0xAA55AA55;
    strcpy(head.head,
           "auto update image");
    fwrite(&head, 1, sizeof(head), out_file);

    fclose(out_file);
    out_file = NULL;
  }
  return 0;
}