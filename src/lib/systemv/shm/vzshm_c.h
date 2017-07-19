/************************************************************************
*Author      : Sober.Peng 17-06-28
*Description :
************************************************************************/
#ifndef _VZSHM_C_H_
#define _VZSHM_C_H_

typedef struct {
  unsigned int  n_w_sec;      // 写秒
  unsigned int  n_w_usec;     // 写微妙

  unsigned int  reserved[60]; // 备用

  unsigned int  n_size;       // 数据长度
  unsigned int  n_data;       // 写长度
  unsigned char p_data[0];    // 数据指针
} TAG_SHM;

// 视频通道0
#define SHM_VIDEO_0       "/dev/shm/video_0"
#define SHM_VIDEO_0_SIZE  704*576*3/2 + 512

#define SHM_AUDIO_0       "/dev/shm/audio_0"
#define SHM_AUDIO_0_SIZE  1024

#ifdef __cplusplus
extern "C"{
#endif

/************************************************************************
*Description : 创建共享HANDLE
*Parameters  : s_key[IN]  共享内存:键
*              n_size[IN] 共享内存:数据长度
*Return      : NULL 创建失败,!=NULL HANDLE
************************************************************************/
void *Shm_Create(const char* s_key, unsigned int n_size);

/************************************************************************
*Description : 释放共享HANDLE
*Parameters  : 
*Return      : 
************************************************************************/
void  Shm_Release(void *p_hdl);

/************************************************************************
*Description : 写数据
*Parameters  : p_data[IN] 写入数据
*              n_data[IN] 写入数据长度
*              n_sec[IN] 当前时间,秒
*              n_usec[IN] 当前时间,微妙
*Return      : <0 失败, >0 写入数据长度
************************************************************************/
int   Shm_Write(void *p_hdl,
                const char* p_data, unsigned int n_data,
                unsigned int n_sec, unsigned int n_usec);

/************************************************************************
*Description : 读数据
*Parameters  : p_data[OUT] 读数据
*              n_data[IN] 数据buffer长度
*              n_sec[OUT] 写数据时间,秒
*              n_usec[OUT] 写数据时间,微妙
*Return      : <0 失败, >0 读数据长度, =0 已经被读过数据
************************************************************************/
int   Shm_Read(void *p_hdl,
               char *p_data, unsigned int n_data, 
               unsigned int *n_sec, unsigned int *n_usec);

/************************************************************************
*Description : 三个函数完成一次写数据事务
*              Shm_W_Begin调用写Lock
*              Shm_W_Write写数据,用户自己维护写偏移
*              Shm_W_End调用写Unlock,设置写完成时间
************************************************************************/

/************************************************************************
*Description : 写事务:调用写Lock
*Parameters  : 
*Return      : <0 失败, 0 成功
************************************************************************/
int   Shm_W_Begin(void *p_hdl);

/************************************************************************
*Description : 写事务:写数据,用户自己维护写偏移
*Parameters  : p_data[IN] 写入数据
*              n_data[IN] 写入数据长度
*              n_offset[IN] 此次写入数据偏移
*Return      : <0 失败, >0 写入数据长度
************************************************************************/
int   Shm_W_Write(void *p_hdl, 
                  const char* p_data, unsigned int n_data,
                  unsigned int n_offset);

/************************************************************************
*Description : 写事务:调用写Unlock,设置写完成时间
*Parameters  : n_sec[IN] 当前时间,秒
*              n_usec[IN] 当前时间,微妙
*Return      : <0 失败, 0 成功
************************************************************************/
int   Shm_W_End(void *p_hdl, unsigned int n_sec, unsigned int n_usec);

#ifdef __cplusplus
}
#endif 

#endif  // _VZSHM_C_H_
