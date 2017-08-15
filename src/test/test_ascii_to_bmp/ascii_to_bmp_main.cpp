#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include "asc8.h"

#pragma pack(push, 1)
typedef struct tagBITMAPFILEHEADER {
  short bfType;
  int bfSize;
  short bfReserved1;
  short bfReserved2;
  int bfOffBits;
} BITMAPFILEHEADER;
typedef struct tagBITMAPINFOHEADER {
  int biSize;
  int biWidth;
  int biHeight;
  short biPlanes;
  short biBitCount;
  int biCompression;
  int biSizeImage;
  int biXPelsPerMeter;
  int biYPelsPerMeter;
  int biClrUsed;
  int biClrImportant;
} BITMAPINFOHEADER;
typedef struct tagRGBQUAD {
  char rgbBlue;
  char rgbGreen;
  char rgbRed;
  char rgbReserved;
} RGBQUAD;
typedef struct tagBITMAPINFO {
  BITMAPINFOHEADER bmiHeader;
  RGBQUAD bmiColors[1];
} BITMAPINFO;
typedef struct tagBITMAP {
  BITMAPFILEHEADER bfHeader;
  BITMAPINFO biInfo;
} BITMAPFILE;
#pragma pack(pop)

int GenBmpData(char *pData, char bitCountPerPix, int width, int height, const char *filename) {
  FILE *fp = fopen(filename, "wb");
  if (!fp) {
    printf("fopen failed : %s, %d\n", __FILE__, __LINE__);
    return 0;
  }
  int bmppitch = ((width*bitCountPerPix + 31) >> 5) << 2;
  int filesize = bmppitch*height;
  BITMAPFILE bmpfile;
  bmpfile.bfHeader.bfType = 0x4D42;
  bmpfile.bfHeader.bfSize = filesize + sizeof(BITMAPFILEHEADER)+sizeof(BITMAPINFOHEADER);
  bmpfile.bfHeader.bfReserved1 = 0;
  bmpfile.bfHeader.bfReserved2 = 0;
  bmpfile.bfHeader.bfOffBits = sizeof(BITMAPFILEHEADER)+sizeof(BITMAPINFOHEADER);
  bmpfile.biInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
  bmpfile.biInfo.bmiHeader.biWidth = width;
  bmpfile.biInfo.bmiHeader.biHeight = height;
  bmpfile.biInfo.bmiHeader.biPlanes = 1;
  bmpfile.biInfo.bmiHeader.biBitCount = bitCountPerPix;
  bmpfile.biInfo.bmiHeader.biCompression = 0;
  bmpfile.biInfo.bmiHeader.biSizeImage = 0;
  bmpfile.biInfo.bmiHeader.biXPelsPerMeter = 0;
  bmpfile.biInfo.bmiHeader.biYPelsPerMeter = 0;
  bmpfile.biInfo.bmiHeader.biClrUsed = 0;
  bmpfile.biInfo.bmiHeader.biClrImportant = 0;
  fwrite(&(bmpfile.bfHeader), sizeof(BITMAPFILEHEADER), 1, fp);
  fwrite(&(bmpfile.biInfo.bmiHeader), sizeof(BITMAPINFOHEADER), 1, fp);
  char *pEachLinBuf = (char*)malloc(bmppitch);
  memset(pEachLinBuf, 0, bmppitch);
  char BytePerPix = bitCountPerPix >> 3;
  int pitch = width * BytePerPix;
  if (pEachLinBuf) {
    int h, w;
    for (h = height - 1; h >= 0; h--) {
      for (w = 0; w < width; w++) {
        pEachLinBuf[w*BytePerPix + 0] = pData[h*pitch + w*BytePerPix + 0];
        pEachLinBuf[w*BytePerPix + 1] = pData[h*pitch + w*BytePerPix + 1];
        pEachLinBuf[w*BytePerPix + 2] = pData[h*pitch + w*BytePerPix + 2];
      }
      fwrite(pEachLinBuf, bmppitch, 1, fp);
    }
    free(pEachLinBuf);
  }
  fclose(fp);
  return 1;
}

typedef struct _LI_RGB {
  char b;
  char g;
  char r;
} LI_RGB;
int PutBMP(FILE *fp16, char *str) {
  const unsigned char bit[8] = {128, 64, 32, 16, 8, 4, 2, 1};
  int x, y, wid;
  const int WIDTH = 16 * 8, HEIGHT = 16;
  long len;
  char buf[32];
  LI_RGB pRGB[HEIGHT][WIDTH];         // 定义位图数据
  memset(pRGB, 0XFF, sizeof(pRGB));   // 设置背景为白色
  for (wid = 0; wid < strlen(str); wid++) {
    len = str[wid] * 16;
    fseek(fp16, len, SEEK_SET);
    fread(buf, 1, 16, fp16);
    for (y = 0; y < 16; y++) {
      for (x = 0; x < 8; x++) {
        if (buf[y + x / 8] & bit[x % 8]) {
          pRGB[y][wid * 8 + x].b = 0x00;
          pRGB[y][wid * 8 + x].g = 0x00;
          pRGB[y][wid * 8 + x].r = 0x00;
        }
      }
    }
  }

  GenBmpData((char*)pRGB, 24, WIDTH, HEIGHT, "out.bmp");//生成BMP文件
  return 1;
}


int main(char argc, char *argv[]) {
  char buf[17] = {"2014/10/15 17:06"};
  FILE *fd = fopen("E:/workspace/git_work/isaws/src/test/test_ascii_to_bmp/asc16", "rb");
  if (!fd) {
    printf("fopen failed : %s, %d\n", __FILE__, __LINE__);
    return 0;
  }
  PutBMP(fd, buf);
  fclose(fd);
  return 1;
}