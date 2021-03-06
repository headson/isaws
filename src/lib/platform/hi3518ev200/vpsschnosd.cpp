#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <unistd.h>

#include "vpsschnosd.h"
#include "systemv/shm/vzshm_c.h"
#include "vzbase/helper/stdafx.h"

#define FONT_PATH_ASC16 "/mnt/app/exec/asc16"

static const int FONT_WIDTH = 8;
static const int FONT_HEIGHT = 16;
static char *s_pFontSetASC16 = NULL;

// 第二种方法是直接读取时间并进行显示，不在进行位图格式的转换，下载附件字体库放在相应的位置
// 代码的实现如下：
int OSD_LoadFile(char *pName, int filesize, void *pAddr) {
  FILE *pfd = NULL;

  if(pAddr == NULL || pName == NULL) {
    printf("pAddr or pName not exist !!\n");
    goto QUIT_LOAD;
  }

  pfd = fopen(pName, "rb");
  if (pfd == NULL) {
    printf("open file error!!\n");
    goto QUIT_LOAD;
  }

  if(fseek(pfd, 0, SEEK_SET) < 0) {
    printf("file error!!\n");
    goto QUIT_LOAD;
  }

  if(fread(pAddr, filesize, 1, pfd) < 0) {
    printf("file error!!\n");
    goto QUIT_LOAD;
  }

  if (pfd != NULL) {
    fclose(pfd);
  }
  return 0;

QUIT_LOAD:
  if(pfd != NULL) {
    fclose(pfd);
  }

  return -1;
}

long OSD_GetFileSize(char *pName) {
  FILE *pfd = NULL;
  long filesize = 0;

  if( pName == NULL) {
    printf("pName not exist !!\n");
    goto QUIT_GET;
  }

  pfd = fopen(pName, "rb");
  if (pfd == NULL) {
    printf("open file error!!\n");
    goto QUIT_GET;
  }

  if (fseek(pfd, 0, SEEK_END) < 0) {
    printf("file error!!\n");
    goto QUIT_GET;
  }

  filesize = ftell(pfd);

  if (pfd != NULL) {
    fclose(pfd);
  }

  return filesize;

QUIT_GET:
  if(pfd != NULL) {
    fclose(pfd);
  }
  return -1;
}

char *open_font_lib(char* fontPath) {
  int fileSize = 0;
  char *pFont = NULL;

  fileSize = (int)OSD_GetFileSize(fontPath);
  if (fileSize < 0) {
    LOG(L_ERROR) << "osd get file size failed.";
    return NULL;
  }

  pFont = (char*)malloc(fileSize);
  if (!pFont) {
    printf("memory not enough !!\n");
    return NULL;
  }

  if (OSD_LoadFile(fontPath, fileSize, pFont) < 0) {
    free(pFont);
    printf("LoadFile fail !!\n");
    return NULL;
  }
  return pFont;
}

void init_font_libs(void) {
  if (s_pFontSetASC16 == NULL)
    s_pFontSetASC16 = open_font_lib(FONT_PATH_ASC16);
}

static int OSD_Draw_BitMap_ASC16(int len, const char *pdata, unsigned char *pbitmap, int ncolor) {
  if(0 == len || !pdata || !pbitmap) {
    printf("[%s, %d] error, NULL pointer transfered.\n", __FUNCTION__, __LINE__);
    return -1;
  }
  int i, w, h, flag, offset;
  unsigned char ch;
  char *code;
  unsigned short *pDst;

  //printf("[%s %d]: len = %d, data = %s\n", __func__, __LINE__, len, pdata);
  int xx = 0;
  char *asc16 = s_pFontSetASC16;
  /***move 1 Byte to can set color***/
  pbitmap = pbitmap+1;

  /* get the first row, then next*/
  for(h = 0; h < 16; h++) { // height ;
    for(i = 0; i < len; i++) {
      ch = pdata[i];
      offset = ch * 16;
      code = asc16 + offset;
      flag = 0x80;
      for (w = 0; w < 8; w++) {
        pDst = (unsigned short *)(pbitmap + xx);
        if((code[h]) & flag) {
          // *pDst = (0x00<< 10) | (0x00<< 5) | (0x80);//display font
          *pDst = ncolor; // (0xFF<< 10) | (0xFF<< 5) | (0xFF);//display font
        } else {
          *pDst = (0x88<< 10) | (0x88<< 5) | (0x77);//(0xff<< 10) | (0x00<< 5) | (0x00);//background
        }
        flag >>= 1;
        xx += 2;
      }
    }
  }
  //printf("[%s %d]: xx = %d\n", __func__, __LINE__, xx);
  return 0;
}

int OSD_Overlay_RGN_Handle_Init(HI_S32 chn_id, RGN_HANDLE Handle, int x, int y, int w, int h, int bgalpha=10, HI_BOOL bShow=HI_TRUE) {
  HI_S32 s32Ret = HI_FAILURE;
  RGN_ATTR_S stRgnAttr;
  MPP_CHN_S stChn;
  VENC_GRP VencGrp;
  RGN_CHN_ATTR_S stChnAttr;

  stRgnAttr.enType                            = OVERLAY_RGN;
  stRgnAttr.unAttr.stOverlay.enPixelFmt       = PIXEL_FORMAT_RGB_1555;
  stRgnAttr.unAttr.stOverlay.stSize.u32Width  = w;
  stRgnAttr.unAttr.stOverlay.stSize.u32Height = h;
  stRgnAttr.unAttr.stOverlay.u32BgColor       = 0x00007FFF;

  s32Ret = HI_MPI_RGN_Create(Handle, &stRgnAttr);
  if (HI_SUCCESS != s32Ret) {
    printf("HI_MPI_RGN_Create (%d) failed with %#x!\n", Handle, s32Ret);
    return HI_FAILURE;
  }
  printf("create handle:%d success!\n", Handle);

  memset(&stChnAttr, 0, sizeof(stChnAttr));
  stChn.enModId = HI_ID_VENC;
  stChn.s32DevId = 0;
  stChn.s32ChnId = chn_id;

  stChnAttr.bShow = bShow;
  stChnAttr.enType = OVERLAY_RGN;
  stChnAttr.unChnAttr.stOverlayChn.stPoint.s32X = x;
  stChnAttr.unChnAttr.stOverlayChn.stPoint.s32Y = y;
  stChnAttr.unChnAttr.stOverlayChn.u32BgAlpha = bgalpha;
  stChnAttr.unChnAttr.stOverlayChn.u32FgAlpha = 128;
  stChnAttr.unChnAttr.stOverlayChn.u32Layer = Handle;

  stChnAttr.unChnAttr.stOverlayChn.stQpInfo.bAbsQp = HI_FALSE;
  stChnAttr.unChnAttr.stOverlayChn.stQpInfo.s32Qp = 0;
  stChnAttr.unChnAttr.stOverlayChn.stQpInfo.s32Qp = 0;
  stChnAttr.unChnAttr.stOverlayChn.stQpInfo.bQpDisable = HI_FALSE;

  stChnAttr.unChnAttr.stOverlayChn.stInvertColor.stInvColArea.u32Height = 16 * (Handle % 2 + 1);
  stChnAttr.unChnAttr.stOverlayChn.stInvertColor.stInvColArea.u32Width = 16 * (Handle % 2 + 1);
  stChnAttr.unChnAttr.stOverlayChn.stInvertColor.u32LumThresh = 128;
  stChnAttr.unChnAttr.stOverlayChn.stInvertColor.enChgMod = LESSTHAN_LUM_THRESH;
  stChnAttr.unChnAttr.stOverlayChn.stInvertColor.bInvColEn = HI_FALSE;
  s32Ret = HI_MPI_RGN_AttachToChn(Handle, &stChn, &stChnAttr);
  if (HI_SUCCESS != s32Ret) {
    printf("HI_MPI_RGN_AttachToChn (%d to %d) failed with %#x!\n", Handle, VencGrp, s32Ret);
    return HI_FAILURE;
  }

  return HI_SUCCESS;
}

int OSD_Overlay_RGN_Display_English(RGN_HANDLE Handle, const char *pRgnContent, int ncolor) {
  HI_S32 s32Ret = HI_FAILURE;
  BITMAP_S stBitmap;
  int ContentLen = 0;

  if (NULL == pRgnContent) {
    printf("[%s, %d] error, NULL pointer transfered.\n", __FUNCTION__, __LINE__);
    return -1;
  }
  ContentLen = strlen(pRgnContent);

  /* HI_MPI_RGN_SetBitMap */
  unsigned char *BitMap = (unsigned char *)malloc(ContentLen*FONT_WIDTH*FONT_HEIGHT*2); //RGB1555: 2 bytes(R:5 G:5 B:5).
  if (NULL == BitMap) {
    printf("malloc error with\n");
    return HI_FAILURE;
  }
  memset(BitMap, '\0', ContentLen*FONT_WIDTH*FONT_HEIGHT*2);

  OSD_Draw_BitMap_ASC16(ContentLen, pRgnContent, BitMap, ncolor);

  stBitmap.enPixelFormat = PIXEL_FORMAT_RGB_1555;
  stBitmap.u32Width  = FONT_WIDTH * ContentLen;
  stBitmap.u32Height = FONT_HEIGHT;
  stBitmap.pData     = BitMap;

  s32Ret = HI_MPI_RGN_SetBitMap(Handle, &stBitmap);
  if (s32Ret != HI_SUCCESS) {
    printf("HI_MPI_RGN_SetBitMap failed with %x!\n", s32Ret);
    if (BitMap)
      free(BitMap);
    return HI_FAILURE;
  }

  if (BitMap)
    free(BitMap);
  return 0;
}

HI_S32 Hi_LiteOs_OSD_Text_Start(int chn_id, RGN_HANDLE Handle, int x, int y, const char *text) {
  static int firstflag = 1;

  if(firstflag) {
    firstflag = 0;
    init_font_libs();
  }

  OSD_Overlay_RGN_Handle_Init(chn_id, Handle, x, y, FONT_WIDTH*strlen(text), FONT_HEIGHT, 0);

  OSD_Overlay_RGN_Display_English(Handle, text, 0xFFFF);
}

HI_S32 Hi_LiteOs_OSD_Cover_Start(int chn_id, RGN_HANDLE Handle, int x, int y, int w, int h) {
  OSD_Overlay_RGN_Handle_Init(chn_id, Handle, x, y, w, h, 128, HI_FALSE);
}

// 时间显示成功;
HI_S32  Hi_LiteOs_OSD_Update(int channel, const char *text, int ncolor) {
  OSD_Overlay_RGN_Display_English(channel, text, ncolor);
}

int Get_Sys_DayTime(char *pTime) {
  if (NULL == pTime) {
    printf("[%s, %d] error, NULL pointer transfered.\n", __FUNCTION__, __LINE__);
    return -1;
  }
  memset(pTime, 0, 31);

  time_t rawtime;
  char tmp[32] = { 0 };
  struct tm * timeinfo = NULL;

  time(&rawtime);
  timeinfo = localtime(&rawtime);
  sprintf(tmp, "%04d-%02d-%02d %02d:%02d:%02d",
          timeinfo->tm_year + 1900, timeinfo->tm_mon + 1, timeinfo->tm_mday,
          timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);
  tmp[strlen(tmp)] = '\0';
  strcpy(pTime, tmp);
  return 0;
}

// 调用并进行刷新时间
void OSD_Init(TAG_OSD *posd) {
  Get_Sys_DayTime(posd->ch1);
  snprintf(posd->ch2, 31, "IN:0000000000 OUT:0000000000");

  RGN_HANDLE hdl = 0;

  // video0
  hdl = 0;
  Hi_LiteOs_OSD_Text_Start(0, hdl+0, 10, 10, posd->ch1);
  Hi_LiteOs_OSD_Text_Start(0, hdl+1, 10, SHM_VIDEO_0_H - 18, posd->ch2);
  Hi_LiteOs_OSD_Cover_Start(0, hdl+2, 0, SHM_VIDEO_0_H/2+64, SHM_VIDEO_0_W, 80);

  // video1
  hdl = 3;
  Hi_LiteOs_OSD_Text_Start(1, hdl+0, 10, 10, posd->ch1);
  Hi_LiteOs_OSD_Text_Start(1, hdl+1, 10, SHM_VIDEO_1_H - 18, posd->ch2);
  Hi_LiteOs_OSD_Cover_Start(1, hdl+2, 0, SHM_VIDEO_1_H/2+28, SHM_VIDEO_1_W, 32);
}

void OSD_Overlay(TAG_OSD *posd) {
  static int ncolor = 0xFFFF;
  static time_t nnow = time(NULL);
  if ((time(NULL) - nnow) == 0) {
    return;
  }
  nnow = time(NULL);

  if (ncolor == 0xFFFF) {
    ncolor = 0x8000;
  } else {
    ncolor = 0xFFFF;
  }

  RGN_HANDLE hdl = 0;

  Get_Sys_DayTime(posd->ch1);

  hdl = 0;
  Hi_LiteOs_OSD_Update(hdl + 0, posd->ch1, ncolor);
  if (posd->ch2[0] != '\0') {
    Hi_LiteOs_OSD_Update(hdl + 1, posd->ch2, ncolor);
  }

  hdl = 3;
  Hi_LiteOs_OSD_Update(hdl + 0, posd->ch1, ncolor);
  if (posd->ch2[0] != '\0') {
    Hi_LiteOs_OSD_Update(hdl + 1, posd->ch2, ncolor);
  }
}

int OSD_Adjust(HI_S32 chn_id, RGN_HANDLE Handle,
               int x, int y, int bgalpha, HI_BOOL bShow) {
  HI_S32 s32Ret = HI_FAILURE;
  MPP_CHN_S stChn;
  VENC_GRP VencGrp;
  RGN_CHN_ATTR_S stChnAttr;

  memset(&stChnAttr, 0, sizeof(stChnAttr));
  stChn.enModId = HI_ID_VENC;
  stChn.s32DevId = 0;
  stChn.s32ChnId = chn_id;

  stChnAttr.bShow = bShow;
  stChnAttr.enType = OVERLAY_RGN;
  stChnAttr.unChnAttr.stOverlayChn.stPoint.s32X = x/2*2;
  stChnAttr.unChnAttr.stOverlayChn.stPoint.s32Y = y/2*2;
  stChnAttr.unChnAttr.stOverlayChn.u32BgAlpha = bgalpha;
  stChnAttr.unChnAttr.stOverlayChn.u32FgAlpha = 128;
  stChnAttr.unChnAttr.stOverlayChn.u32Layer = Handle;

  stChnAttr.unChnAttr.stOverlayChn.stQpInfo.bAbsQp = HI_FALSE;
  stChnAttr.unChnAttr.stOverlayChn.stQpInfo.s32Qp = 0;
  stChnAttr.unChnAttr.stOverlayChn.stQpInfo.s32Qp = 0;
  stChnAttr.unChnAttr.stOverlayChn.stQpInfo.bQpDisable = HI_FALSE;

  stChnAttr.unChnAttr.stOverlayChn.stInvertColor.stInvColArea.u32Height = 16 * (Handle % 2 + 1);
  stChnAttr.unChnAttr.stOverlayChn.stInvertColor.stInvColArea.u32Width = 16 * (Handle % 2 + 1);
  stChnAttr.unChnAttr.stOverlayChn.stInvertColor.u32LumThresh = 128;
  stChnAttr.unChnAttr.stOverlayChn.stInvertColor.enChgMod = LESSTHAN_LUM_THRESH;
  stChnAttr.unChnAttr.stOverlayChn.stInvertColor.bInvColEn = HI_FALSE;
  s32Ret = HI_MPI_RGN_SetDisplayAttr(Handle, &stChn, &stChnAttr);
  if (HI_SUCCESS != s32Ret) {
    printf("HI_MPI_RGN_AttachToChn (%d to %d) failed with %#x!\n", Handle, VencGrp, s32Ret);
    return HI_FAILURE;
  }

  return HI_SUCCESS;
}
