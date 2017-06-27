/************************************************************************
*Author      : Sober.Peng 17 - 05 - 24
*Description : 看门狗;向vzlogserver定时喂狗,否则判断此线程挂掉
************************************************************************/
#ifndef SRC_LIB_VZLOGGING_INCLUDE_VZWATCHDOG_H_
#define SRC_LIB_VZLOGGING_INCLUDE_VZWATCHDOG_H_

#ifdef __cplusplus
extern "C" {
#endif

/* 看门狗超时时间 */
#define DEF_WATCHDOG_TIMEOUT 21

/************************************************************************
*Description : 注册一个喂狗KEY, 并传入观察进程名, 进程描述
*Parameters  : n_wdg_timeout[IN] 看门狗超时时间
*              s_descrebe[IN] 用户描述[MAX:8Byte] : 进程名 + 描述形成唯一主键,
*              看门狗通过监听此主键心跳判断是否挂掉
*              n_descrebe[IN] 用户描述大小
*Return      : !=NULL 注册成功,==NULL 注册失败
*************************************************************************/
void *RegisterWatchDogKey(const char   *s_descrebe,
                          unsigned int  n_descrebe,
                          unsigned int  n_wdg_timeout);

/************************************************************************
*Description: 喂狗接口, 定时调用, 否则看门狗服务判断此key相关线程挂掉
*Parameters : key[IN] 注册看门狗时使用传入的key值
*Return     : 0 喂狗成功,-1 喂狗失败
*************************************************************************/
int FeedDog(const void *p_wdg_hdl);

#ifdef __cplusplus
}
#endif

#endif  // SRC_LIB_VZLOGGING_INCLUDE_VZWATCHDOG_H_
