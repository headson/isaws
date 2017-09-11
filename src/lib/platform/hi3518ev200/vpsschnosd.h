/************************************************************************
*Author      : Sober.Peng 17-09-11
*Description :
************************************************************************/
#ifndef LIBPLATFORM_VPSSCHNOSD_H
#define LIBPLATFORM_VPSSCHNOSD_H

#include <pthread.h>

#define MAX_OSD_HDL   MAX_ENC_CHN
#define MAX_OSD_SIZE  32

typedef struct {
  pthread_t     pid;
  char          ch1[MAX_OSD_SIZE];
  char          ch2[MAX_OSD_SIZE];
  char          ch3[MAX_OSD_SIZE];
} TAG_OSD;

#ifdef __cplusplus
extern "C"
{
#endif
#include "common/sample_comm.h"

void OSD_Init(TAG_OSD *posd);

void OSD_Overlay(TAG_OSD *posd);

int OSD_Adjust(HI_S32 chn_id, RGN_HANDLE Handle,
               int x, int y, int bgalpha, HI_BOOL bShow);

#ifdef __cplusplus
};
#endif

#endif  // LIBPLATFORM_VPSSCHNOSD_H
