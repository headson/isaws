/************************************************************************/
/* 作者: SoberPeng 17-05-23
/* 描述: 
/************************************************************************/
#ifndef _VDEFINE_H
#define _VDEFINE_H

#include <stdio.h>

#include "verror.h"
#include "vmessage.h"

#define LOG_INFO(...)                             \
  do {                                            \
    printf("[%s]-%d: ", __FUNCTION__, __LINE__);  \
    printf(__VA_ARGS__);                          \
  } while (0)

#define LOG_ERROR(...)                             \
  do {                                            \
    printf("[%s]-%d: ", __FUNCTION__, __LINE__);  \
    printf(__VA_ARGS__);                          \
  } while (0)


/***共享内存*************************************************************/
#ifdef WIN32
#define DEF_SHM_VIDEO_0       "shm_video_0"
#else   // WIN32
#define DEF_SHM_VIDEO_0       0x00001000
#endif  // WIN32

/***信号量***************************************************************/
#ifdef WIN32
#define DEF_SEM_VIDEO_0_R     "sem_video_0_r"
#define DEF_SEM_VIDEO_0_W     "sem_video_0_w"
#else   // WIN32
#define DEF_SEM_VIDEO_0_R     0x00001001
#define DEF_SEM_VIDEO_0_W     0x00001002
#endif  // WIN32
#endif  // _VDEFINE_H