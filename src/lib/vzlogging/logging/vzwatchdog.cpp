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

// 看门狗结构
typedef struct {
  unsigned int n_mark;                                  // 校验字
  unsigned int n_max_timeout;                           // 最大超时时长
  char         s_descrebe[LEN_DESCREBE+4];              // 用户描述
  unsigned int n_descrebe_size;                         // 用户描述大小
} TAG_WATCHDOG;
TAG_WATCHDOG   k_watchdog[MAX_WATCHDOG_A_PROCESS] = { 0 };  // 看门狗结构

/************************************************************************
*Description : 注册一个喂狗KEY, 并传入观察进程名, 进程描述
*Parameters  : key 进程名 + KEY形成唯一主键,
*              看门狗通过监听此主键心跳判断是否挂掉
*              max_timeout    最大超时时间
*              descrebe       用户描述[MAX:8Byte]
*              descrebe_size  用户描述大小
*Return      : != NULL 注册成功, == NULL 注册失败
************************************************************************/
void *RegisterWatchDogKey(const char  *s_descrebe,
                          unsigned int n_descrebe_size,
                          unsigned int n_max_timeout) {
  int n_empty = 0;

  // 判断注册两次
  for (n_empty = 0 ; n_empty < MAX_WATCHDOG_A_PROCESS; n_empty++) {
    if (k_watchdog[n_empty].n_mark == DEF_TAG_MARK &&
        strncmp(k_watchdog[n_empty].s_descrebe, s_descrebe, 8) == 0) {
      return &k_watchdog[n_empty];
    }
  }

  // 加入新的
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
*Description : 判断是否所有的模块都正常运行
*Parameters  :
*Return      : 1=所有模块运行正常,<0有1个或几个模块运行失败
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
      // 加入时间和最后一次心跳时间一致,认为没收到心跳
      // 最后一次心跳有变化, 判断是否超时
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

      // 加入时间和最后一次心跳时间一致,认为没收到心跳
      if (cmod.last_heartbeat == cmod.join_time) {
        return 0;
      }

      // 最后一次心跳有变化, 判断是否超时
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
*Description : 喂狗接口, 定时调用, 否则看门狗服务判断此key相关线程挂掉
*Parameters  : key[IN] 注册看门狗时使用传入的key值
*Return      : >0 喂狗成功, -1 喂狗失败
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

