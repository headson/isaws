/************************************************************************
*Author      : Sober.Peng 17-06-28
*Description :
************************************************************************/
#ifndef _VZSHM_C_H_
#define _VZSHM_C_H_

typedef struct {
  unsigned int  n_w_sec;      // д��
  unsigned int  n_w_usec;     // д΢��

  unsigned int  reserved[60]; // ����

  unsigned int  n_size;       // ���ݳ���
  unsigned int  n_data;       // д����
  unsigned char p_data[0];    // ����ָ��
} TAG_SHM;

// ��Ƶͨ��0
#define SHM_VIDEO_0       "/dev/shm/video_0"
#define SHM_VIDEO_0_SIZE  704*576*3/2 + 512

#define SHM_AUDIO_0       "/dev/shm/audio_0"
#define SHM_AUDIO_0_SIZE  1024

#ifdef __cplusplus
extern "C"{
#endif

/************************************************************************
*Description : ��������HANDLE
*Parameters  : s_key[IN]  �����ڴ�:��
*              n_size[IN] �����ڴ�:���ݳ���
*Return      : NULL ����ʧ��,!=NULL HANDLE
************************************************************************/
void *Shm_Create(const char* s_key, unsigned int n_size);

/************************************************************************
*Description : �ͷŹ���HANDLE
*Parameters  : 
*Return      : 
************************************************************************/
void  Shm_Release(void *p_hdl);

/************************************************************************
*Description : д����
*Parameters  : p_data[IN] д������
*              n_data[IN] д�����ݳ���
*              n_sec[IN] ��ǰʱ��,��
*              n_usec[IN] ��ǰʱ��,΢��
*Return      : <0 ʧ��, >0 д�����ݳ���
************************************************************************/
int   Shm_Write(void *p_hdl,
                const char* p_data, unsigned int n_data,
                unsigned int n_sec, unsigned int n_usec);

/************************************************************************
*Description : ������
*Parameters  : p_data[OUT] ������
*              n_data[IN] ����buffer����
*              n_sec[OUT] д����ʱ��,��
*              n_usec[OUT] д����ʱ��,΢��
*Return      : <0 ʧ��, >0 �����ݳ���, =0 �Ѿ�����������
************************************************************************/
int   Shm_Read(void *p_hdl,
               char *p_data, unsigned int n_data, 
               unsigned int *n_sec, unsigned int *n_usec);

/************************************************************************
*Description : �����������һ��д��������
*              Shm_W_Begin����дLock
*              Shm_W_Writeд����,�û��Լ�ά��дƫ��
*              Shm_W_End����дUnlock,����д���ʱ��
************************************************************************/

/************************************************************************
*Description : д����:����дLock
*Parameters  : 
*Return      : <0 ʧ��, 0 �ɹ�
************************************************************************/
int   Shm_W_Begin(void *p_hdl);

/************************************************************************
*Description : д����:д����,�û��Լ�ά��дƫ��
*Parameters  : p_data[IN] д������
*              n_data[IN] д�����ݳ���
*              n_offset[IN] �˴�д������ƫ��
*Return      : <0 ʧ��, >0 д�����ݳ���
************************************************************************/
int   Shm_W_Write(void *p_hdl, 
                  const char* p_data, unsigned int n_data,
                  unsigned int n_offset);

/************************************************************************
*Description : д����:����дUnlock,����д���ʱ��
*Parameters  : n_sec[IN] ��ǰʱ��,��
*              n_usec[IN] ��ǰʱ��,΢��
*Return      : <0 ʧ��, 0 �ɹ�
************************************************************************/
int   Shm_W_End(void *p_hdl, unsigned int n_sec, unsigned int n_usec);

#ifdef __cplusplus
}
#endif 

#endif  // _VZSHM_C_H_
