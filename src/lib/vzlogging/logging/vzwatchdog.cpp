/************************************************************************
*Author      : Sober.Peng 17-06-27
*Description : 
************************************************************************/
#include "vzlogging/logging/vzwatchdog.h"

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "vzlogging/base/vzbases.h"
#include "vzlogging/base/vzcommon.h"
#include "vzlogging/logging/vzlogging.h"

namespace vzlogging {

  // ���Ź��ṹ
typedef struct {
  unsigned int n_mark;                                  // У����
  unsigned int n_max_timeout;                           // ���ʱʱ��
  char         s_descrebe[LEN_DESCREBE+4];     // �û�����
  unsigned int n_descrebe_size;                         // �û�������С
} TAG_WATCHDOG;
TAG_WATCHDOG   k_watchdog[MAX_WATCHDOG_A_PROCESS] = {0};  // ���Ź��ṹ

}  // vzlogging
/************************************************************************
*Description : ע��һ��ι��KEY, ������۲������, ��������
*Parameters  : key ������ + KEY�γ�Ψһ����,
*              ���Ź�ͨ�����������������ж��Ƿ�ҵ�
*              max_timeout    ���ʱʱ��
*              descrebe       �û�����[MAX:8Byte]
*              descrebe_size  �û�������С
*Return      : != NULL ע��ɹ�, == NULL ע��ʧ��
************************************************************************/
void *RegisterWatchDogKey(const char   *s_descrebe,
                          unsigned int n_descrebe_size,
                          unsigned int n_max_timeout) {
  int n_empty = 0;

  // �ж�ע������
  for (n_empty = 0 ; n_empty < MAX_WATCHDOG_A_PROCESS; n_empty++) {
    if (vzlogging::k_watchdog[n_empty].n_mark == DEF_TAG_MARK &&
        strncmp(vzlogging::k_watchdog[n_empty].s_descrebe, s_descrebe, 8) == 0) {
      return &vzlogging::k_watchdog[n_empty];
    }
  }

  // �����µ�
  for (n_empty = 0 ; n_empty < MAX_WATCHDOG_A_PROCESS; n_empty++) {
    if (vzlogging::k_watchdog[n_empty].n_mark == 0) {
      vzlogging::k_watchdog[n_empty].n_mark = DEF_TAG_MARK;
      vzlogging::k_watchdog[n_empty].n_max_timeout = n_max_timeout;
      vzlogging::k_watchdog[n_empty].n_descrebe_size =
        (n_descrebe_size > LEN_DESCREBE) ? \
        LEN_DESCREBE : n_descrebe_size;
      memcpy(vzlogging::k_watchdog[n_empty].s_descrebe, s_descrebe,
             vzlogging::k_watchdog[n_empty].n_descrebe_size);
      return &vzlogging::k_watchdog[n_empty];
    }
  }
  return NULL;
}

/************************************************************************
*Description : �ж��Ƿ����е�ģ�鶼��������
*Parameters  :
*Return      : 1=����ģ����������,<0��1���򼸸�ģ������ʧ��
************************************************************************/
int IsAllModuleRuning() {
  vzlogging::TAG_SHM_ARG shm_arg;
  int res = k_shm_arg.GetShmArg(&shm_arg);

  int no_run_mod = 0;
  unsigned int nsec = vzlogging::GetSysSec();
  for (int i = 0; i < MAX_WATCHDOG_A_DEVICE; i++) {
    if (shm_arg.mod_state[i].mark == DEF_TAG_MARK 
        && shm_arg.mod_state[i].app_name[0] != '\0') {
      if ((nsec - shm_arg.mod_state[i].last_heartbeat) >= DEF_WATCHDOG_TIMEOUT) {
        no_run_mod++;
      }
    }
  }
  if (0 == no_run_mod) {
    return 1;
  }
  return -no_run_mod;
}

int IsModuleRuning(const char *name, const char *desc) {
  vzlogging::TAG_SHM_ARG shm_arg;
  int res = k_shm_arg.GetShmArg(&shm_arg);

  int no_run_mod = 0;
  unsigned int nsec = vzlogging::GetSysSec();
  for (int i = 0; i < MAX_WATCHDOG_A_DEVICE; i++) {
    if (shm_arg.mod_state[i].mark == DEF_TAG_MARK
        && strcmp(shm_arg.mod_state[i].app_name, name) == 0
        && strcmp(shm_arg.mod_state[i].descrebe, desc) == 0) {
      if ((nsec - shm_arg.mod_state[i].last_heartbeat) >= DEF_WATCHDOG_TIMEOUT) {
        return 0;
      } else {
        return 1;
      }
    }
  }
  return -1;
}

/************************************************************************
*Description : ι���ӿ�, ��ʱ����, �����Ź������жϴ�key����̹߳ҵ�
*Parameters  : key[IN] ע�ῴ�Ź�ʱʹ�ô����keyֵ
*Return      : true ι���ɹ�, false ι��ʧ��
************************************************************************/
int FeedDog(const void *p_arg) {
  vzlogging::TAG_WATCHDOG* p_wdg = (vzlogging::TAG_WATCHDOG*)p_arg;
  if (p_wdg && p_wdg->n_mark == DEF_TAG_MARK) {
    int n_ret = ::VzLog(L_HEARTBEAT, 0, __FILE__, __LINE__,
                        "%s %d %s",
                        k_app_name,
                        p_wdg->n_max_timeout,
                        p_wdg->s_descrebe);
    if (n_ret == 0) {
      return 0;
    }
  }
  return -1;
}

