
// 第二种方法是直接读取时间并进行显示，不在进行位图格式的转换，下载附件字体库放在相应的位置
// 代码的实现如下：
int OSD_LoadFile(char *pName, int filesize, void *pAddr) {
  FILE *pfd = NULL;

  if( pAddr == NULL || pName == NULL ) {
    printf("pAddr or pName not exist !!\n");
    goto QUIT_LOAD;
  }


  pfd = fopen(pName, "rb");
  if ( pfd == NULL ) {
    printf("open file error!!\n");
    goto QUIT_LOAD;
  }


  if( fseek( pfd, 0, SEEK_SET ) < 0) {
    printf("file error!!\n");
    goto QUIT_LOAD;
  }

  if( fread(pAddr, filesize, 1, pfd) < 0) {
    printf("file error!!\n");
    goto QUIT_LOAD;
  }


  if ( pfd != NULL ) {
    fclose(pfd);
  }


  return 0;

QUIT_LOAD:
  if( pfd != NULL ) {
    fclose(pfd);
  }

  return -1;
}

long OSD_GetFileSize(char *pName) {
  FILE *pfd = NULL;
  long filesize = 0;

  if(  pName == NULL ) {
    printf("pName not exist !!\n");
    goto QUIT_GET;
  }

  pfd = fopen(pName, "rb");
  if ( pfd == NULL ) {
    printf("open file error!!\n");
    goto QUIT_GET;
  }

  if (fseek( pfd, 0, SEEK_END ) < 0) {
    printf("file error!!\n");
    goto QUIT_GET;
  }

  filesize = ftell(pfd);

  if ( pfd != NULL ) {
    fclose(pfd);
  }

  return filesize;

QUIT_GET:
  if( pfd != NULL ) {
    fclose(pfd);
  }
  return -1;
}

char *open_font_lib(char* fontPath) {
  int fileSize = 0;
  char *pFont = NULL;

  fileSize = (int)OSD_GetFileSize(fontPath);
  pFont  = malloc(fileSize);
  if ( !pFont ) {
    printf("memory not enough !!\n");
    return NULL;
  }

  if ( OSD_LoadFile(fontPath, fileSize, pFont) < 0 ) {
    free ( pFont );
    printf("LoadFile fail !!\n");
    return NULL;
  }
  return pFont;
}

void init_font_libs(void) {
  if (s_pFontSetASC32 == NULL)
    s_pFontSetASC32 = open_font_lib(FONT_PATH_ASC32);
  if (s_pFontSetASC16 == NULL)
    s_pFontSetASC16 = open_font_lib(FONT_PATH_ASC16);
}

int OSD_Draw_BitMap_ASC32(int len, const unsigned char *pdata, unsigned char *pbitmap) {
  int temp;
  int index=0;

  if( 0 == len || !pdata || !pbitmap ) {
    printf("[%s, %d] error, NULL pointer transfered.\n", __FUNCTION__, __LINE__);
    return -1;
  }

  int i, w, h, flag, offset;
  unsigned char ch;
  unsigned char *code, *pDst;

  int xx = 0;
  char *asc32 =s_pFontSetASC32;
  /***move 1 Byte to can set color***/
  pbitmap = pbitmap+1;

  /* get the first row, then next*/
  for( h = 0; h < 32; h++ ) { // height ;  32
    for( i = 0; i < len; i++ ) {
      ch = pdata[i];
      offset  = ch * 64;
      code = asc32 + offset;
      flag = 0x8000;

      for ( w = 0; w < 16; w++ ) { //width : 16
        pDst = (unsigned short *)( pbitmap + xx );
        if(flag > 0x80) {
          if( (code[h*2]) & (flag >> 8)) {
            *pDst = (0xff<< 10) | (0xff<< 5) | (0xff);//display font
          } else {
            *pDst = (0x88<< 10) | (0x88<< 5) | (0x77);//(0xff<< 10) | (0x00<< 5) | (0x00);//background
          }
        } else {
          if( (code[h*2+1]) & flag ) {
            *pDst = (0xFF<< 10) | (0xFF<< 5) | (0xff);//display font
          } else {
            *pDst = (0x88<< 10) | (0x88<< 5) | (0x77);//(0xff<< 10) | (0x00<< 5) | (0x00);//background
          }
        }

        flag >>= 1;
        xx += 2;
      }
    }
  }


}

static int OSD_Draw_BitMap_ASC16(int len, const unsigned char *pdata, unsigned char *pbitmap) {
  if( 0 == len || !pdata || !pbitmap ) {
    printf("[%s, %d] error, NULL pointer transfered.\n", __FUNCTION__, __LINE__);
    return -1;
  }
  int i, w, h, flag, offset;
  unsigned char ch;
  unsigned char *code, *pDst;

  //printf("[%s %d]: len = %d, data = %s\n", __func__, __LINE__, len, pdata);
  int xx = 0;
  char *asc16 =s_pFontSetASC16;
  /***move 1 Byte to can set color***/
  pbitmap = pbitmap+1;

  /* get the first row, then next*/
  for( h = 0; h < 16; h++ ) { // height ;
    for( i = 0; i < len; i++ ) {
      ch = pdata[i];
      offset = ch * 16;
      code = asc16 + offset;
      flag = 0x80;
      for ( w = 0; w < 8; w++ ) {
        pDst = (unsigned short *)( pbitmap + xx );
        if( (code[h]) & flag) {
          // *pDst = (0x00<< 10) | (0x00<< 5) | (0x80);//display font
          *pDst = (0xFF<< 10) | (0xFF<< 5) | (0xFF);//display font
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

int OSD_Overlay_RGN_Handle_Init(RGN_HANDLE Handle,  unsigned int ContentLen) {
  HI_S32 s32Ret = HI_FAILURE;
  RGN_ATTR_S stRgnAttr;
  MPP_CHN_S stChn;
  VENC_GRP VencGrp;
  RGN_CHN_ATTR_S stChnAttr;

  int font_w = 8;
  int font_h = 16;
  if(Handle == 0) {
    font_w = 16;
    font_h = 32;
  } else {
    font_w = 8;
    font_h = 16;
  }

  stRgnAttr.enType                            = OVERLAY_RGN;
  stRgnAttr.unAttr.stOverlay.enPixelFmt       = PIXEL_FORMAT_RGB_1555;
  stRgnAttr.unAttr.stOverlay.stSize.u32Width  = font_w * ContentLen;
  stRgnAttr.unAttr.stOverlay.stSize.u32Height = font_h;
  stRgnAttr.unAttr.stOverlay.u32BgColor       = OSD_PALETTE_COLOR_WHITE;

  s32Ret = HI_MPI_RGN_Create(Handle, &stRgnAttr);
  if (HI_SUCCESS != s32Ret) {
    printf("HI_MPI_RGN_Create (%d) failed with %#x!\n", Handle, s32Ret);
    return HI_FAILURE;
  }
  printf("create handle:%d success!\n", Handle);

  memset(&stChnAttr, 0, sizeof(stChnAttr));
  if(Handle == 0) {
    stChn.enModId = HI_ID_VENC;
    stChn.s32DevId = 0;
    stChn.s32ChnId = 0;
  } else {
    stChn.enModId = HI_ID_VENC;
    stChn.s32DevId = 0;
    stChn.s32ChnId = 1;
  }
  stChnAttr.bShow = HI_TRUE;
  stChnAttr.enType = OVERLAY_RGN;
  stChnAttr.unChnAttr.stOverlayChn.stPoint.s32X     = OSD_POSITION_X;
  stChnAttr.unChnAttr.stOverlayChn.stPoint.s32Y     = OSD_POSITION_Y;
  stChnAttr.unChnAttr.stOverlayChn.u32BgAlpha       = 10;
  stChnAttr.unChnAttr.stOverlayChn.u32FgAlpha       = 128;
  stChnAttr.unChnAttr.stOverlayChn.u32Layer         = 0;

  stChnAttr.unChnAttr.stOverlayChn.stQpInfo.bAbsQp  = HI_FALSE;
  stChnAttr.unChnAttr.stOverlayChn.stQpInfo.s32Qp   = 0;
  s32Ret = HI_MPI_RGN_AttachToChn(Handle, &stChn, &stChnAttr);
  if (HI_SUCCESS != s32Ret) {
    printf("HI_MPI_RGN_AttachToChn (%d to %d) failed with %#x!\n", Handle, VencGrp, s32Ret);
    return HI_FAILURE;
  }
  return HI_SUCCESS;
}

int OSD_Overlay_RGN_Display_English(RGN_HANDLE Handle, const unsigned char *pRgnContent) {
  HI_S32 s32Ret = HI_FAILURE;
  BITMAP_S stBitmap;
  int ContentLen = 0;

  if (NULL == pRgnContent) {
    printf("[%s, %d] error, NULL pointer transfered.\n", __FUNCTION__, __LINE__);
    return -1;
  }
  ContentLen = strlen(pRgnContent);

  int font_w = 8;
  int font_h = 16;
  if(Handle == 0) {  // english  :  16 x 32
    font_w = 16;
    font_h = 32;
  } else {
    font_w = 8;
    font_h = 16;
  }

  /* HI_MPI_RGN_SetBitMap */
  unsigned char *BitMap = (unsigned char *)malloc(ContentLen*font_w*font_h*2); //RGB1555: 2 bytes(R:5 G:5 B:5).
  if (NULL == BitMap) {
    printf("malloc error with\n");
    return HI_FAILURE;
  }
  memset(BitMap, '\0', ContentLen*font_w*font_h*2);

  if(font_w == 16)
    OSD_Draw_BitMap_ASC32(ContentLen, pRgnContent, BitMap);
  else
    OSD_Draw_BitMap_ASC16(ContentLen, pRgnContent, BitMap);

  stBitmap.enPixelFormat = PIXEL_FORMAT_RGB_1555;
  stBitmap.u32Width  = font_w*ContentLen;
  stBitmap.u32Height = font_h;
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

HI_S32 Hi_LiteOs_OSD_Start(int channel, const char * text) {
  static int firstflag = 1;

  if(firstflag) {
    firstflag = 0;
    init_font_libs();
  }

  RGN_HANDLE Handle = channel;
  VENC_GRP RgnVencChn = channel;
  OSD_Overlay_RGN_Handle_Init(Handle, RgnVencChn, strlen(text));

  OSD_Overlay_RGN_Display_English(Handle, text);
}

// 时间显示成功;
HI_S32  Hi_LiteOs_OSD_Update(int channel, const char *text ) {
  OSD_Overlay_RGN_Display_English(channel, text);
}

// 调用并进行刷新时间
void *API_OSD_DisplayProcess(void * arg) {
  char timebuf[32];
  Get_Sys_DayTime(timebuf);

  Hi_LiteOs_OSD_Start(0, timebuf);
  Hi_LiteOs_OSD_Start(1, timebuf);

  while(1) {
    Get_Sys_DayTime(timebuf);

    Hi_LiteOs_OSD_Update(0, timebuf);
    Hi_LiteOs_OSD_Update(1, timebuf);

    msleep(500);
  }
}

int Get_Sys_DayTime(unsigned char *pTime) {
  if (NULL == pTime) {
    printf("[%s, %d] error, NULL pointer transfered.\n", __FUNCTION__, __LINE__);
    return -1;
  }
  memset(pTime, '\0', sizeof(pTime));

  time_t rawtime;
  struct tm * timeinfo = NULL;
  char tmp[32] = {0};
  memset(timeinfo, '\0', sizeof(timeinfo));

  time (&rawtime);
  timeinfo = localtime(&rawtime);
  sprintf(tmp, "%04d-%02d-%02d %02d:%02d:%02d", 
          timeinfo->tm_year+1900, timeinfo->tm_mon+1, timeinfo->tm_mday, 
          timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);
  tmp[strlen(tmp)] = '\0';
  strcpy(pTime, tmp);
  return 0;
}