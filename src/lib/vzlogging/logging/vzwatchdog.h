/************************************************************************
*Author      : Sober.Peng 17 - 05 - 24
*Description : ���Ź�;��vzlogserver��ʱι��,�����жϴ��̹߳ҵ�
************************************************************************/
#ifndef SRC_LIB_VZLOGGING_INCLUDE_VZWATCHDOG_H_
#define SRC_LIB_VZLOGGING_INCLUDE_VZWATCHDOG_H_

#ifdef __cplusplus
extern "C" {
#endif

#define DEF_FEEDDOG_TIME     4
#define DEF_WATCHDOG_TIMEOUT 21   /* ���Ź���ʱʱ�� */

/************************************************************************
*Description : ע��һ��ι��KEY, ������۲������, ��������
*Parameters  : sec_timeout[IN] ���Ź���ʱʱ��(��)
*              sdescrebe[IN] �û�����[MAX:8Byte] : ������ + �����γ�Ψһ����,
*              ���Ź�ͨ�����������������ж��Ƿ�ҵ�
*              ndescrebe[IN] �û�������С
*Return      : !=NULL ע��ɹ�,==NULL ע��ʧ��
*************************************************************************/
void *RegisterWatchDogKey(const char   *sdescrebe,
                          unsigned int  ndescrebe,
                          unsigned int  sec_timeout);

/************************************************************************
*Description : �ж��Ƿ����е�ģ�鶼��������
*Parameters  : 
*Return      : 1=����ģ����������,<0��1���򼸸�ģ������ʧ��
************************************************************************/
int IsAllModuleRuning();

/************************************************************************
*Description : �ж�ĳһģ���Ƿ���������
*Parameters  :
*Return      : 1=��������,=0δ��������,-1û�ҵ���ģ��
************************************************************************/
int IsModuleRuning(const char *name, const char *desc);

/************************************************************************
*Description: ι���ӿ�, ��ʱ����, �����Ź������жϴ�key����̹߳ҵ�
*Parameters : key[IN] ע�ῴ�Ź�ʱʹ�ô����keyֵ
*Return     : 0 ι���ɹ�,-1 ι��ʧ��
*************************************************************************/
int FeedDog(const void *p_wdg_hdl);

#ifdef __cplusplus
}
#endif

#endif  // SRC_LIB_VZLOGGING_INCLUDE_VZWATCHDOG_H_
