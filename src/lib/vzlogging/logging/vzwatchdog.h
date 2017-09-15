/************************************************************************
*Author      : Sober.Peng 17 - 05 - 24
*Description : 看门狗;向vzlogserver定时喂狗,否则判断此线程挂掉
************************************************************************/
#ifndef SRC_LIB_VZLOGGING_INCLUDE_VZWATCHDOG_H_
#define SRC_LIB_VZLOGGING_INCLUDE_VZWATCHDOG_H_

#ifdef __cplusplus
extern "C" {
#endif

#define DEF_FEEDDOG_TIME     4
#define DEF_WATCHDOG_TIMEOUT 21   /* 看门狗超时时间 */

/************************************************************************
*Description : 注册一个喂狗KEY, 并传入观察进程名, 进程描述
*Parameters  : sec_timeout[IN] 看门狗超时时间(秒)
*              sdescrebe[IN] 用户描述[MAX:8Byte] : 进程名 + 描述形成唯一主键,
*              看门狗通过监听此主键心跳判断是否挂掉
*              ndescrebe[IN] 用户描述大小
*Return      : !=NULL 注册成功,==NULL 注册失败
*************************************************************************/
void *RegisterWatchDogKey(const char   *sdescrebe,
                          unsigned int  ndescrebe,
                          unsigned int  sec_timeout);

/************************************************************************
*Description : 判断是否所有的模块都正常运行
*Parameters  : 
*Return      : 1=所有模块运行正常,<0有1个或几个模块运行失败
************************************************************************/
int IsAllModuleRuning();

/************************************************************************
*Description : 判断某一模块是否正常运行
*Parameters  :
*Return      : 1=运行正常,=0未正常运行,-1没找到此模块
************************************************************************/
int IsModuleRuning(const char *name, const char *desc);

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
