/************************************************************************
*Author      : Sober.Peng 17-06-28
*Description :
************************************************************************/
#include "vz_shm.h"

#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>

#ifdef WIN32
#include <io.h>

HANDLE vzSemCreate(const char* name) {
  HANDLE sem_id = vzSemOpen(name);
  if (sem_id > 0) {
    vzSemUnLock(sem_id);
  }
  return sem_id;
}

HANDLE vzSemOpen(const char* name) {
  unsigned int lInitialCount = 1;
  unsigned int lMaximumCount = 1;
  HANDLE sem_id = CreateSemaphore(NULL,
                                  lInitialCount,
                                  lMaximumCount,
                                  (LPCTSTR)name);
  if (sem_id == NULL) {
    printf("CreateSemaphore failed.\n");
    return NULL;
  }
  return sem_id;
}

int vzSemLock(HANDLE sem_id, unsigned int n_msec) {
  if (sem_id == NULL) {
    return -1;
  }

  return WaitForSingleObject(sem_id, n_msec);
}

int vzSemUnLock(HANDLE sem_id) {
  if (sem_id == NULL) {
    return false;
  }
  unsigned int lReleaseCount = 1;
  unsigned int *lpPreviousCount = NULL;
  return (ReleaseSemaphore(sem_id,
                           lReleaseCount,
                           (LPLONG)lpPreviousCount) == TRUE);
}

//////////////////////////////////////////////////////////////////////////
HANDLE vzShmOpen(const char* name, int size) {
  HANDLE shm_id = ::OpenFileMapping(FILE_MAP_ALL_ACCESS, 0, (LPCSTR)name);
  if (shm_id == NULL) {
    shm_id = ::CreateFileMapping(INVALID_HANDLE_VALUE,
                                 NULL,
                                 PAGE_READWRITE,
                                 0,
                                 size,
                                 (LPCSTR)name);
  }
  if (shm_id == NULL) {
    printf("OpenFileMapping failed.\n");
    return NULL;
  }

  printf("shm_id = 0x%x\n", shm_id);
  return shm_id;
}

void *vzShmAt(HANDLE shm_id) {
  void *p_ptr = ::MapViewOfFile(shm_id, FILE_MAP_ALL_ACCESS, 0, 0, 0);
  if (p_ptr == NULL) {
    printf("MapViewOfFile failed.\n");
    return NULL;
  }
  return p_ptr;
}

void vzShmDt(void *p_ptr) {
  if (p_ptr) {
    if (FALSE == ::UnmapViewOfFile(p_ptr)) {
      printf("UnmapViewOfFile failed.\n");
    }
    p_ptr = NULL;
  }
}
#else  // LINUX
#include <unistd.h>
#include <sys/shm.h>
#include <sys/sem.h>

static int create_file(const char *filename) {
  if (access(filename, 0) == 0) {
    return 0;
  }

  if (creat(filename, 0755) < 0) {
    printf("create file %s failure!\n", filename);
    return -1;
  }

  printf("create file %s success!\n", filename);
  return 0;
}

HANDLE vzSemCreate(const char* name) {
  HANDLE sem_id = vzSemOpen(name);
  if (sem_id > 0) {
    vzSemUnLock(sem_id);
  }
  return sem_id;
}

HANDLE vzSemOpen(const char* name) {
  create_file(name);
  key_t k = ftok(name, 1);
  if (k < 0) {
    return -2;
  }

  HANDLE sem_id = semget(k, 1, IPC_CREAT | 0666);
  if (sem_id < 0) {
    sem_id = semget(k, 0, 0666);
  }
  if (sem_id < 0) {
    return -1;
  }
  printf("semid = %d\n", sem_id);
  return sem_id;
}

int vzSemLock(HANDLE sem_id, unsigned int n_msec) {
  struct sembuf sem_op = {0, -1, 0};
  sem_op.sem_num = 0;
  sem_op.sem_flg = IPC_NOWAIT;
  if (n_msec = (unsigned int)-1) {
    if ((semop(sem_id, &sem_op, 1)) == -1) {
      return -1;
    }
  } else {
    struct timespec ts;
    ts.tv_sec = n_msec / 1000;
    ts.tv_nsec = (n_msec % 1000) * 1000;
    if (semtimedop(sem_id, &sem_op, 1, &ts) == -1) {
      return -1;
    }
  }
  return 0;
}

int vzSemUnLock(HANDLE sem_id) {
  struct sembuf sem_op = {0, 1, 0};
  sem_op.sem_num = 0;
  sem_op.sem_flg = IPC_NOWAIT;
  if ((semop(sem_id, &sem_op, 1)) == -1) {
    return -1;
  }
  return 0;
}

//////////////////////////////////////////////////////////////////////////
HANDLE vzShmOpen(const char* name, int size) {
  create_file(name);
  key_t k = ftok(name, 1);
  if (k < 0) {
    return -2;
  }

  HANDLE shm_id = shmget(k, size, IPC_CREAT | 0660);
  if(shm_id < 0) {
    shm_id = shmget(k, 0, 0);
  }
  if(shm_id < 0) {
    return -1;
  }

  printf("shm_id = %d, %d\n", shm_id, k);
  return shm_id;
}

void *vzShmAt(HANDLE shm_id) {
  void *p_ptr = shmat(shm_id, 0, 0);
  if (p_ptr == NULL) {
    printf("shmat failed.\n");
    return NULL;
  }
  return p_ptr;
}

void vzShmDt(void *p_ptr) {
  if (p_ptr) {
    shmdt(p_ptr);
  }
}
#endif
