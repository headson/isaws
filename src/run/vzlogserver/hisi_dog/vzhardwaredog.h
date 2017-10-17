#ifndef SRC_LIB_VZLOGGING_INCLUDE_VZHARDWAREDOG_H_
#define SRC_LIB_VZLOGGING_INCLUDE_VZHARDWAREDOG_H_

/*****************************************************
** Copyright:vz
** Author:rjx
** Date:2017-06-28
** Description:海思R相机硬件看门狗管理封装.
******************************************************/
#ifdef HISI_R

namespace vzlog {

/*
** Description: 硬件看门狗操作封装类
**				1.开启硬件看门狗
**              2.关闭硬件看门狗
**              3.喂狗及状态获取
*/
class PWMWatchDog {
 private:
  PWMWatchDog() {}
  ~PWMWatchDog() {}
  PWMWatchDog(const PWMWatchDog&);
  PWMWatchDog& operator=(PWMWatchDog&);

 public:
  /* 看门狗操作类型枚举 */
  typedef enum R3516D_WATCH_DOG_ {
    PWM_WDOG_START = 0xCB000000,    /* 开启看门口     */
    PWM_WDOG_STOP,                  /* 关闭PWM看门口  */
    PWM_WDOG_FEED                   /* 喂狗           */
  } R3516D_WATCH_DOG;

  typedef int PWM_DOG_FD;

  /*看门狗喂狗时间，默认设置为5s*/
  static const int INTERAL_FEED_TIME = 50;

 public:
  /*****************************************************
  ** Function: 获取看门狗状态.
  ** Output  : true为已开启,false为断开.
  ******************************************************/
  static bool GetDogState();

  /*****************************************************
  ** Function: 获取看门狗状态.
  ** Output  : true为已开启,false为断开.
  ******************************************************/
  static bool OpenWatchDogFd();

  /*****************************************************
  ** Function: 开启看门狗.
  ** Output  : true为已开启成功,false为开启失败.
  ******************************************************/
  static bool StartPwmWatchDog();

  /*****************************************************
  ** Function: 关闭看门狗
  ** Output  : space.
  ******************************************************/
  static void ClosePwmDog();

  /*****************************************************
  ** Function: 喂看门狗
  ** Output  : true为喂狗成功,false为喂狗失败.
  ******************************************************/
  static bool FeedPwmWatchDog();

 private:
  /*看门狗硬件设备文件描述符*/
  static PWM_DOG_FD pwm_dog_fd_;

  /*看门狗硬件设备设备名称*/
  static char DEV_DES_NAME[16];
};

}  // namespace vzlog

#endif  // HISI_R

#endif  // SRC_LIB_VZLOGGING_INCLUDE_VZHARDWAREDOG_H_