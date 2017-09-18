#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <string>

#ifdef _WIN32
#include "getopt.h"
#else
#include <getopt.h>
#endif

#define DEF_IMG_POS   4*1024
#define DEF_IMG_CNT   16
typedef struct {
  unsigned int    mark;       // 0xAA55AA55
  char            head[32];   //
  unsigned int    crc32;
  struct {
    char          fname[32];  // 文件名
    unsigned int  file_pos;   // 写偏移
    unsigned int  file_size;  // 文件大小
    unsigned int  part_size;  // 文件大小
    unsigned int  part_vers;  // 文件版本
    unsigned int  last_vers;  // 上个版本
  } fimg[DEF_IMG_CNT];
} TAG_HEADER;

int PacketImage(int argc, char* argv[]) {
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

  int abs_pos = DEF_IMG_POS;
  FILE *ini_file = fopen(ini_path.c_str(), "rt");
  FILE *out_file = fopen(out_path.c_str(), "wb");
  for (int i = 0; i < DEF_IMG_CNT && ini_file && out_file; i++) {
    char sline[256] = {0};
    fgets(sline, 255, ini_file);

    if (strlen(sline) > 0) {
      sscanf(sline, "%s\t%d\t%d\t%d",
             head.fimg[i].fname, &head.fimg[i].file_pos,
             &head.fimg[i].part_size, &head.fimg[i].part_vers);

      if (head.fimg[i].part_size > 0 &&
          strlen(head.fimg[i].fname) > 0) {
        // 移动到写文件地址
        abs_pos += head.fimg[i].file_pos * 1024;
        fseek(out_file,  abs_pos,  SEEK_SET);

        std::string sf = img_path;
        sf  +=  head.fimg[i].fname;
        FILE *img_file = fopen(sf.c_str(), "rb+");
        if (img_file) {
          fwrite(head.fimg[i].fname, 1, 32, out_file);
        }
        head.fimg[i].file_size = 0;
        while (img_file) {
          char sdata[1024] = {0};
          // 读镜像文件
          int ndata = fread(sdata, 1, 1024, img_file);
          if (ndata <= 0) {
            break;
          }

          // 写入到输出文件
          fwrite(sdata, 1, ndata, out_file);
          head.fimg[i].file_size += ndata;
        }
        printf("%s \n\twrite pos %d\n\tfile size %d\n\tpart size %d\n\tpart vers %d.\n",
               sf.c_str(), abs_pos, head.fimg[i].file_size,
               head.fimg[i].part_size, head.fimg[i].part_vers);
        if (img_file) {
          fclose(img_file);
          img_file = NULL;
        }
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

int UnpackImage(int argc, char* argv[]) {
  std::string img_path = "c://tools//image//all_bin.img";
  int opt = 0;
  while ((opt = getopt(argc, argv, "o:i:f:")) != -1) {
    switch (opt) {
    case 'i':
      img_path = optarg;
      break;
    }
  }

  int nimg = 0;
  char *pimg = NULL;
  FILE *img_file = fopen(img_path.c_str(), "rb+");
  if (img_file) {
    fseek(img_file, 0, SEEK_END);
    nimg = ftell(img_file);

    fseek(img_file, 0, SEEK_SET);
    pimg = new char[nimg];
    int nread = fread(pimg, 1, nimg, img_file);
    if (nimg != nread) {
      fclose(img_file);
      printf("read file failed %d %d.\n", nimg, nread);
      return -1;
    }

    fclose(img_file);
    img_file = NULL;
  }
  char sboot[256] = "hello worlds:";
  TAG_HEADER *phead = (TAG_HEADER *)pimg;
  char sver[65] = "100.101.100.101.100....";
  const char *sep = "."; //可按多个字符来分割
  char *p = strtok(sver, sep);
  for (int i = 0; i < DEF_IMG_CNT && p; i++) {
    phead->fimg[i].last_vers = atoi(p);
    p = strtok(NULL, sep);
  }

  memset(sver, 0, 65);
  int abs_pos = DEF_IMG_POS;
  for (int i = 0; i < DEF_IMG_CNT; i++) {
    if (phead->fimg[i].file_size <= 0) {
      break;
    }
    // version
    char sv[4] = {0};
    sprintf(sv, "%03d.", 
            phead->fimg[i].part_vers);
    memcpy(sver + i*4, sv, 4);
    // bootargs
    char spos[24] = {0};
    sprintf(spos, "%dK(%s),",
            phead->fimg[i].part_size, phead->fimg[i].fname);
    memcpy(sboot + strlen(sboot), spos, strlen(spos));

    abs_pos += phead->fimg[i].file_pos * 1024;

    // the same version is not update
    if (phead->fimg[i].part_vers == phead->fimg[i].last_vers) {
      printf("%s is not new version.\n", phead->fimg[i].fname);
      continue;
    }

    // update
    std::string sf = "c://tools//image1//";
    sf += phead->fimg[i].fname;
    FILE *img_file = fopen(sf.c_str(), "wb");
    if (NULL == img_file) {
      printf("open %s failed.\n", sf.c_str());
      continue;
    }
    

    char *img_bng = pimg + abs_pos + 32;
    int   img_len = phead->fimg[i].file_size;
    fwrite(img_bng, 1, img_len, img_file);

    fclose(img_file);
    img_file = NULL;

    printf("%s \n\twrite pos %d\n\tfile size %d\n\tpart size %d\n\tpart vers %d.\n",
           sf.c_str(), abs_pos, phead->fimg[i].file_size,
           phead->fimg[i].part_size, phead->fimg[i].part_vers);
  }
  if (strlen(sver) > 0) {
    sver[strlen(sver)-1] = '\0';
  }
  if (strlen(sboot) > 0) {
    sboot[strlen(sboot)-1] = '\0';
  }
  
  printf("%s\n%s\n", sver, sboot);
  return 0;
}

int main(int argc, char *argv[]) {
  PacketImage(argc, argv);
  printf("---------------------------------------------------\n");
  UnpackImage(argc, argv);
  return 0;
}
