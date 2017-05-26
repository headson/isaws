/************************************************************************/
/* Author      : Sober.Peng 17-05-24
/* Description : ���Ź�;��vzlogserver��ʱ(10S)ι��,�����жϴ��̹߳ҵ�
/************************************************************************/
#ifndef SRC_LIB_VZLOGGING_INCLUDE_VZWATCHDOG_H_
#define SRC_LIB_VZLOGGING_INCLUDE_VZWATCHDOG_H_

namespace vzlogging {

#ifdef __cplusplus
extern "C" {
#endif

/* ���Ź���ʱʱ�� */
#define DEF_MAX_TIMEOUT_SEC 21

/************************************************************************/
/* Description : ע��һ��ι��KEY,������۲������,��������
/* Parameters  : max_timeout[IN] ���ʱʱ��
                 descrebe[IN] �û�����[MAX:8Byte]:������+�����γ�Ψһ����,
                              ���Ź�ͨ�����������������ж��Ƿ�ҵ�
                 descrebe_size[IN] �û�������С
/* Return      : !=NULL ע��ɹ�,==NULL ע��ʧ��
/************************************************************************/
void *RegisterWatchDogKey(const char   *s_descrebe,
                          unsigned int n_descrebe_size,
                          unsigned int n_max_timeout = DEF_MAX_TIMEOUT_SEC);

/************************************************************************/
/* Description : ι���ӿ�,��ʱ����,�����Ź������жϴ�key����̹߳ҵ�
/* Parameters  : key[IN] ע�ῴ�Ź�ʱʹ�ô����keyֵ
/* Return      : true ι���ɹ�,false ι��ʧ��
/************************************************************************/
bool FeedDog(const void *p_arg);

#ifdef __cplusplus
}
#endif

}  // namespace vzlogging

#endif  // SRC_LIB_VZLOGGING_INCLUDE_VZWATCHDOG_H_
