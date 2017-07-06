/************************************************************************
*Author      : Sober.Peng 17-06-28
*Description : 
************************************************************************/
#ifndef SHM_VZ_SHM_H_
#define SHM_VZ_SHM_H_

#ifdef WIN32
#include <windows.h>
#define HDL_NULL  NULL
#ifndef snprintf
#define snprintf    _snprintf
#endif // snprintf
#else   // LINUX
typedef int       HANDLE;
#define HDL_NULL  -1
#endif  // LINUX

#ifdef __cplusplus
extern "C"
{
#endif

// 写进程需要调用一次vzSemUnLock,用来释放
HANDLE vzSemCreate(const char* name);
HANDLE vzSemOpen(const char* name);
// -1 死等
int    vzSemLock(HANDLE sem_id, unsigned int n_msec);
int    vzSemUnLock(HANDLE sem_id);

HANDLE vzShmOpen(const char* name, int size);
void*  vzShmAt(HANDLE shm_id);
void   vzShmDt(void *p_ptr);

#ifdef __cplusplus
}
#endif
#endif  // SHM_VZ_SHM_H_
