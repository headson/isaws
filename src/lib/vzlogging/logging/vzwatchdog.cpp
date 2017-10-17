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

// ���Ź��ṹ
typedef struct {
  unsigned int n_mark;                                  // У����
  unsigned int n_max_timeout;                           // ���ʱʱ��
  char         s_descrebe[LEN_DESCREBE+4];              // �û�����
  unsigned int n_descrebe_size;                         // �û�������С
} TAG_WATCHDOG;
TAG_WATCHDOG   k_watchdog[MAX_WATCHDOG_A_PROCESS] = { 0 };  // ���Ź��ṹ

/************************************************************************
*Description : ע��һ��ι��KEY, ������۲������, ��������
*Parameters  : key ������ + KEY�γ�Ψһ����,
*              ���Ź�ͨ�����������������ж��Ƿ�ҵ�
*              max_timeout    ���ʱʱ��
*              descrebe       �û�����[MAX:8Byte]
*              descrebe_size  �û�������С
*Return      : != NULL ע��ɹ�, == NULL ע��ʧ��
************************************************************************/
void *RegisterWatchDogKey(const char  *s_descrebe,
                          unsigned int n_descrebe_size,
                          unsigned int n_max_timeout) {
  int n_empty = 0;

  // �ж�ע������
  for (n_empty = 0 ; n_empty < MAX_WATCHDOG_A_PROCESS; n_empty++) {
    if (k_watchdog[n_empty].n_mark == DEF_TAG_MARK &&
        strncmp(k_watchdog[n_empty].s_descrebe, s_descrebe, 8) == 0) {
      return &k_watchdog[n_empty];
    }
  }

  // �����µ�
  for (n_empty = 0 ; n_empty < MAX_WATCHDOG_A_PROCESS; n_empty++) {
    if (k_watchdog[n_empty].n_mark == 0) {
      k_watchdog[n_empty].n_mark = DEF_TAG_MARK;
      k_watchdog[n_empty].n_max_timeout = n_max_timeout;
      k_watchdog[n_empty].n_descrebe_size =
        (n_descrebe_size > LEN_DESCREBE) ? \
        LEN_DESCREBE : n_descrebe_size;
      memcpy(k_watchdog[n_empty].s_descrebe, s_descrebe,
             k_watchdog[n_empty].n_descrebe_size);
      return &k_watchdog[n_empty];
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
  vzlogging::TAG_SHM_ARG *shm_arg =
    (vzlogging::TAG_SHM_ARG*)k_shm_arg.GetData();
  if (shm_arg == NULL) {
    VZ_ERROR("get share arg failed.");
    return -1;
  }

  int no_run_mod = 0;
  unsigned int nsec = vzlogging::GetSysSec();
  for (int i = 0; i < MAX_WATCHDOG_A_DEVICE; i++) {
    vzlogging::TAG_MODULE_STATE &cmod = shm_arg->mod_state[i];
    if (cmod.mark == DEF_TAG_MARK
        && cmod.app_name[0] != '\0') {
      // ����ʱ������һ������ʱ��һ��,��Ϊû�յ�����
      // ���һ�������б仯, �ж��Ƿ�ʱ
      if (cmod.last_heartbeat == cmod.join_time ||
          (nsec - cmod.last_heartbeat) >= cmod.timeout) {
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
  vzlogging::TAG_SHM_ARG *shm_arg =
    (vzlogging::TAG_SHM_ARG*)k_shm_arg.GetData();
  if (shm_arg == NULL) {
    VZ_ERROR("get share arg failed.");
    return -1;
  }

  unsigned int nsec = vzlogging::GetSysSec();
  for (int i = 0; i < MAX_WATCHDOG_A_DEVICE; i++) {
    vzlogging::TAG_MODULE_STATE &cmod = shm_arg->mod_state[i];
    //printf("0x%x app_name %s, desc %s. heartbeat %d.\n",
    //       cmod.mark, cmod.app_name, cmod.descrebe,
    //       cmod.last_heartbeat);
    if (cmod.mark == DEF_TAG_MARK
        && strcmp(cmod.app_name, name) == 0
        && strcmp(cmod.descrebe, desc) == 0) {

      // ����ʱ������һ������ʱ��һ��,��Ϊû�յ�����
      if (cmod.last_heartbeat == cmod.join_time) {
        return 0;
      }

      // ���һ�������б仯, �ж��Ƿ�ʱ
      if ((nsec - cmod.last_heartbeat) >= cmod.timeout) {
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
*Return      : >0 ι���ɹ�, -1 ι��ʧ��
************************************************************************/
int FeedDog(const void *p_arg) {
  TAG_WATCHDOG* p_wdg = (TAG_WATCHDOG*)p_arg;
  if (p_wdg && p_wdg->n_mark == DEF_TAG_MARK) {
    int res = ::VzLog(L_HEARTBEAT, 0, __FILE__, __LINE__,
                        "%s %d %s",
                        k_app_name,
                        p_wdg->n_max_timeout,
                        p_wdg->s_descrebe);
    return res;
  }
  return -1;
}

