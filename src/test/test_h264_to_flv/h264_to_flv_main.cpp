#include <stdio.h>

#include "cflvmux.h"
#include "ch264parse.h"

char *nal_parse(const char *ph264, int nh264, int *frm_type, int *bng_pos);

int main() {
  FILE *file = fopen("D:/workspace/git_work/isaws/src/test/test_h264_to_flv/test.h264", "rb");
  if (file == NULL) {
    perror("can't open file.\n");
    return -1;
  }

  fseek(file, 0, SEEK_END);
  int nfile = ftell(file);

  fseek(file, 0, SEEK_SET);

  char *ph264 = new char[nfile+1];
  if (ph264 == NULL) {
    perror("create h264 buffer failed.\n");
    return -1;
  }

  int nh264 = fread(ph264, 1, nfile, file);
  if (nh264 <= 0) {
    printf("read file failed.%d.\n", nh264);
    return -1;
  }
  fclose(file);
  file = NULL;

  char *p_flv = new char[1024 * 1024];

  CFlvMux flv_mux;

  int   nal_bng = 0;
  int   frm_type = 0;
  char *pnal1 = ph264, *pnal2 = ph264;

  // SPS PPS
  while (true) {
    pnal1 = nal_parse(pnal1, nh264 - (pnal1-ph264), &frm_type, &nal_bng);
    if (pnal1 == NULL) {
      break;
    }

    int frm_type2 = 0;
    pnal2 = nal_parse(pnal1+nal_bng, nh264 - (pnal1-ph264), &frm_type2, &nal_bng);
    if (pnal2) {
      int nlen = pnal2 - pnal1;
      if (frm_type == 7) {
        flv_mux.SetSps(pnal1, nlen);
      } else if (frm_type == 8) {
        flv_mux.SetPps(pnal1, nlen);
        break;
      }
      printf("frame type %d, nal length %d.\n", frm_type, nlen);
    } else {
      break;
    }
    pnal1 = pnal2;
  }
  FILE* fout = fopen("d:\\test1.flv", "wb+");
  char *p_pkg = flv_mux.HeaderAndMetaDataTag(p_flv, 720, 576, 1600, 800, 16, 1);
  p_pkg = flv_mux.Packet(p_pkg, NULL, 0, flv_mux.sps_pps_, flv_mux.sps_size_+flv_mux.pps_size_,
                               0x09, 0);
  if (fout) {
    fwrite(p_flv, 1, p_pkg-p_flv, fout);
    fflush(fout);
  }

  unsigned long ntimet = 0;
  // ÆÕÍ¨
  while (true) {
    pnal1 = nal_parse(pnal1, nh264 - (pnal1 - ph264), &frm_type, &nal_bng);
    if (pnal1 == NULL) {
      break;
    }

    int frm_type2 = 0;
    int nal_bng2  = 0;
    pnal2 = nal_parse(pnal1 + nal_bng, nh264 - (pnal1 - ph264), &frm_type2, &nal_bng2);
    if (pnal2) {
      int nlen = pnal2 - pnal1;

      char *p_pkg = NULL;
      if (frm_type == 5) {
        p_pkg = flv_mux.PacketVideo(p_flv, pnal1+nal_bng, nlen-nal_bng, true, ntimet);
      } else if (frm_type == 1) {
        p_pkg = flv_mux.PacketVideo(p_flv, pnal1+nal_bng, nlen-nal_bng, false, ntimet);
      }
      if (p_pkg) {
        if (fout) {
          fwrite(p_flv, 1, p_pkg - p_flv, fout);
          fflush(fout);
        }
      }
      ntimet += 30;
      printf("frame type %d, nal length %d.\n", frm_type, nlen);
    } else {
      break;
    }
    pnal1 = pnal2;
  }


  if (ph264) {
    delete[] ph264;
    ph264 = NULL;
  }
}

char *nal_parse(const char *ph264, int nh264, int *frm_type, int *nal_bng) {
  int   n_0_cnt = 0;    // 0x00 ¸öÊý
  char *p_nal = const_cast<char*>(ph264);
  while (true) {
    if ((p_nal + 1) > (ph264 + nh264)) {
      return NULL;
    }

    if (*p_nal == 0x01 && n_0_cnt >= 2) {
      p_nal -= n_0_cnt;
      *frm_type = p_nal[n_0_cnt+1] & 0x1f;
      *nal_bng  = n_0_cnt + 1;
      return p_nal;
    }

    if (*p_nal == 0x00) {
      n_0_cnt ++;
    } else {
      n_0_cnt = 0;
    }
    p_nal++;
  }
  return NULL;
}


