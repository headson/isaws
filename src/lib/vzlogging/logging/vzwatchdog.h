/************************************************************************
*Author      : Sober.Peng 17 - 05 - 24
*Description : ���Ź�;��vzlogserver��ʱι��,�����жϴ��̹߳ҵ�
************************************************************************/
#ifndef SRC_LIB_VZLOGGING_INCLUDE_VZWATCHDOG_H_
#define SRC_LIB_VZLOGGING_INCLUDE_VZWATCHDOG_H_

#ifdef __cplusplus
extern "C" {
#endif

/* ���Ź���ʱʱ�� */
#define DEF_WATCHDOG_TIMEOUT 21

/************************************************************************
*Description : ע��һ��ι��KEY, ������۲������, ��������
*Parameters  : n_wdg_timeout[IN] ���Ź���ʱʱ��
*              s_descrebe[IN] �û�����[MAX:8Byte] : ������ + �����γ�Ψһ����,
*              ���Ź�ͨ�����������������ж��Ƿ�ҵ�
*              n_descrebe[IN] �û�������С
*Return      : !=NULL ע��ɹ�,==NULL ע��ʧ��
*************************************************************************/
void *RegisterWatchDogKey(const char   *s_descrebe,
                          unsigned int  n_descrebe,
                          unsigned int  n_wdg_timeout);

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
