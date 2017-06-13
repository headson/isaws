/************************************************************************/
/* 作者: SoberPeng 17-05-23
/* 描述: 
/************************************************************************/
#ifndef _VDEFINE_H
#define _VDEFINE_H

#include <time.h>
#include <stdio.h>
#ifdef WIN32
#include <windows.h>
#include <winsock2.h>
#endif

#include "verror.h"
#include "vmessage.h"
#include "vzlogging/logging/vzlogging.h"
#include "vzlogging/logging/vzloggingcpp.h"

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