/************************************************************************/
/* Author      : SoberPeng 2017-06-13
/* Description :
/************************************************************************/
#ifndef VMESSAGE_H
#define VMESSAGE_H

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




#endif  // VMESSAGE_H
