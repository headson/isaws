#ifndef SRC_LIB_VZLOGGING_INCLUDE_VZHARDWAREDOG_H_
#define SRC_LIB_VZLOGGING_INCLUDE_VZHARDWAREDOG_H_

/*****************************************************
** Copyright:vz
** Author:rjx
** Date:2017-06-28
** Description:��˼R���Ӳ�����Ź������װ.
******************************************************/
#ifdef HISI_R

namespace vzlog {

/*
** Description: Ӳ�����Ź�������װ��
**				1.����Ӳ�����Ź�
**              2.�ر�Ӳ�����Ź�
**              3.ι����״̬��ȡ
*/
class PWMWatchDog {
 private:
  PWMWatchDog() {}
  ~PWMWatchDog() {}
  PWMWatchDog(const PWMWatchDog&);
  PWMWatchDog& operator=(PWMWatchDog&);

 public:
  /* ���Ź���������ö�� */
  typedef enum R3516D_WATCH_DOG_ {
    PWM_WDOG_START = 0xCB000000,    /* �������ſ�     */
    PWM_WDOG_STOP,                  /* �ر�PWM���ſ�  */
    PWM_WDOG_FEED                   /* ι��           */
  } R3516D_WATCH_DOG;

  typedef int PWM_DOG_FD;

  /*���Ź�ι��ʱ�䣬Ĭ������Ϊ5s*/
  static const int INTERAL_FEED_TIME = 50;

 public:
  /*****************************************************
  ** Function: ��ȡ���Ź�״̬.
  ** Output  : trueΪ�ѿ���,falseΪ�Ͽ�.
  ******************************************************/
  static bool GetDogState();

  /*****************************************************
  ** Function: ��ȡ���Ź�״̬.
  ** Output  : trueΪ�ѿ���,falseΪ�Ͽ�.
  ******************************************************/
  static bool OpenWatchDogFd();

  /*****************************************************
  ** Function: �������Ź�.
  ** Output  : trueΪ�ѿ����ɹ�,falseΪ����ʧ��.
  ******************************************************/
  static bool StartPwmWatchDog();

  /*****************************************************
  ** Function: �رտ��Ź�
  ** Output  : space.
  ******************************************************/
  static void ClosePwmDog();

  /*****************************************************
  ** Function: ι���Ź�
  ** Output  : trueΪι���ɹ�,falseΪι��ʧ��.
  ******************************************************/
  static bool FeedPwmWatchDog();

 private:
  /*���Ź�Ӳ���豸�ļ�������*/
  static PWM_DOG_FD pwm_dog_fd_;

  /*���Ź�Ӳ���豸�豸����*/
  static char DEV_DES_NAME[16];
};

}  // namespace vzlog

#endif  // HISI_R

#endif  // SRC_LIB_VZLOGGING_INCLUDE_VZHARDWAREDOG_H_