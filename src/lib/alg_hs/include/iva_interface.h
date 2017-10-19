/****************************
��Ȩ����,����Ѷ��˾
�ļ���: iva_interface.h
����:   �ܽ���
�汾:   1.0.0
�������:   2013/5/3
����:       DS������Ƶ�����ӿ�
����:
��Ҫ�����б�:

�޸���ʷ��¼�б�:
<�޸���>   <�޸�����>  <�汾>    <����>

***************************/
#ifndef _IVA_INTERFACE_H_
#define _IVA_INTERFACE_H_

#ifdef __cplusplus
extern "C"
{
#endif

#ifdef WIN32

#ifdef BVS_DLL_API
#else
#define BVS_DLL_API //_declspec(dllimport)
#endif

#else
#define BVS_DLL_API
#endif

#include "define.h"

#define MAX_FILE_LEN 1024       //�ļ�����󳤶�
    
//����֡����(����ӿ�)
#define FRAME_TYPE_DOOR_VIDEO           0				//�Ż���Ƶ
#define FRAME_TYPE_FACE_VIDEO           1				//������Ƶ
#define FRAME_TYPE_ENV_VIDEO			2				//������Ƶ

//�㷨ģʽ(����ӿ�)
#define IVA_MODE_START									1
#define IVA_MODE_ADVANCED_DOOR_SURVEILLANCE				1	//(��Ŀ)�Ż����(����������)
#define IVA_MODE_INDOOR_FACE_RECOGNITION				2	//(˫Ŀ/��Ŀ)���ڿ���(��������)
#define IVA_MODE_ADVANCED_INDOOR_DOOR_CONTROLLING		3	//(��Ŀ)���ڿ���(����������)
#define IVA_MODE_DOOR_SURVEILLANCE						4	//˫Ŀ/��Ŀ �Ż���أ���������ʶ��
#define IVA_MODE_OUTDOOR_FACE_RECOGNITION				5	//˫Ŀ/��Ŀ ���⿼�ڣ�����������
#define IVA_MODE_BALCONY_SURVEILLANCE					6	//˫Ŀ ��԰���
#define IVA_MODE_END									10

/************************************************* ����ָ�� ************************************************************/
//�ⲿָ��(����ӿ�)
#define USER_COMMAND_FACE_ENROLLMENT                1       //����ע��
#define USER_COMMAND_FAST_FACE_ENROLLMENT           2       //��������ע��
#define USER_COMMAND_FACE_ENROLLMENT_CANCLING       3       //����ע��ȡ��
#define USER_COMMAND_DELETE_USER_INFO               4       //ɾ���û���Ϣ
#define USER_COMMAND_MODIFY_USER_INFO               5       //�޸��û���Ϣ
#define USER_COMMAND_AUX_CONFIG_PARAM_UPDATE        6       //�������ò�������
#define USER_COMMAND_OPEN_DOOR1                     7       //����
#define USER_COMMAND_OPEN_DOOR2                     8       //����
#define USER_COMMAND_OPEN_DOOR3                     9       //����
#define USER_COMMAND_OPEN_DOOR4                     10      //����
#define USER_COMMAND_OPEN_VOICE_INTERCOM			18		//���������Խ�	
#define USER_COMMAND_DOORBELL						19      //����
#define USER_COMMAND_ENTERING_ENROLLMENT_INTERFACE	20		//��������ע��
#define USER_COMMAND_LEAVING_ENROLLMENT_INTERFACE	21		//�뿪����ע��
#define USER_COMMAND_OPEN_DOOR                      69      //����
#define USER_COMMAND_USER_SET_DOOR_OPEN             70      //�û������ſ�
#define USER_COMMAND_USER_SET_DOOR_CLOSE            71      //�û������Ź�
#define USER_COMMAND_USER_SET_TURN_ON_LIGHT         72      //�û����ÿ���
#define USER_COMMAND_USER_SET_DOOR_RESET            73      //�û���λ�Ż�״̬
#define USER_COMMAND_CHANGE_LIGHT_INTENSITY			74		//��������������
#define USER_COMMAND_VIDEO_PREVIEW_TURN_ON_LIGHT    75      //��ƵԤ������
#define USER_COMMAND_IDENTITY_VERIFICATION_SUCCESS  82      //�����֤�ɹ�
#define USER_COMMAND_STOP_ALARM                     83      //ֹͣ����
#define USER_COMMAND_ALG_RESET                      84      //�㷨��λ
#define USER_COMMAND_FACE_SNAPSHOT		            85      //����ͼƬץ��
#define USER_COMMAND_ARMING				            86      //����
#define USER_COMMAND_DISARMING				        87      //����
#define USER_COMMAND_AUX_PARAM_UPDATE               88      //�������ò�������
#define USER_COMMAND_DOOR_SNAPSHOT                  89      //�Ż�ͼƬץ��
#define USER_COMMAND_HIJACKING                      90      //�ٳ�
#define USER_COMMAND_ANTI_TAMPER_ALARM              91      //���𱨾�
#define USER_COMMAND_ENV_SNAPSHOT					92      //����ͼƬץ��
#define USER_COMMAND_CTRL                           4000    //�ⲿ����

/************************************************* ������� ************************************************************/
//���ض���ָ��(����ӿ�)
#define LOCAL_ACTION_START													1
#define LOCAL_ACTION_VIDEO_START_SAVING										1		//�������Ż��������ʼ������Ƶ
#define LOCAL_ACTION_VIDEO_STOP_SAVING										2		//�������Ż������ֹͣ������Ƶ
#define LOCAL_ACTION_TURN_ON_LIGHT											3		//��������
#define LOCAL_ACTION_TURN_OFF_LIGHT											4		//�ر�����
#define LOCAL_ACTION_ADJUST_IR_LIGHT_INTENSITY							    5		//�������ⲹ���ǿ��
#define LOCAL_ACTION_TURN_OFF_IR_LIGHT										6		//�رպ�������
#define LOCAL_ACTION_UNLOCK_STOP_ALARM										7		//�����쳣ֹͣ����
#define LOCAL_ACTION_ADJUST_DOOR_CAMERA_EXPOSUER							8		//�����Ż�����ع�ʱ��
#define LOCAL_ACTION_ADJUST_FACE_CAMERA_EXPOSUER							9		//������������ع�ʱ��
#define LOCAL_ACTION_ADJUST_LIGHT_INTENSITY									10		//������������ǿ��
#define LOCAL_ACTION_PLAY_SOUND												11		//������Ƶ
#define LOCAL_ACTION_FACE_ENROLLMENT_CANCLLING								12		//ȡ������ע��
#define LOCAL_ACTION_HEART_BEAT_RESPONDENCE									13		//������Ӧ
#define LOCAL_ACTION_PLAY_EDITABLE_SOUND								    14		//���ſɱ༭����Ƶ
#define LOCAL_ACTION_TURN_ON_LASER_LIGHT							        15		//��������
#define LOCAL_ACTION_TURN_OFF_LASER_LIGHT									16		//�رռ���
#define LOCAL_ACTION_TURN_ON_DOOR_GUARD									    17		//���Ž�����
#define LOCAL_ACTION_TURN_OFF_DOOR_GUARD								    18		//�ر��Ž�����
#define LOCAL_ACTION_SET_MOUNTING_HEIGHT								    19		//���ð�װ�߶�
#define LOCAL_ACTION_RECOVER_SOUND                                          20      //�ָ���������
#define LOCAL_ACTION_RESTART_DEVICE                                         21      //�����豸
#define LOCAL_ACTION_CHANGE_TO_DOOR_RECORDING								22      //�л�Ϊ�Ż�¼��
#define LOCAL_ACTION_CHANGE_TO_ENV_RECORDING								23      //�л�Ϊ����¼��
#define LOCAL_ACTION_TURN_ON_INDICATOR_LIGHT								24		//����ָʾ��
#define LOCAL_ACTION_TURN_OFF_INDICATOR_LIGHT								25		//�ر�ָʾ��
#define LOCAL_ACTION_RESET_NETWORK											26		//��������
#define LOCAL_ACTION_RESET_DOOR_SENSOR										27		//���ü����Ƶ������
#define LOCAL_ACTION_END													500

//Զ�̶���ָ��(����ӿ�)
#define REMOTE_ACTION_START													501
#define REMOTE_ACTION_FACE_ENROLLMENT_START									501		//����ע�Ὺʼ
#define REMOTE_ACTION_FACE_ENROLLMENT_SUCCESS								502		//����ע��ɹ�
#define REMOTE_ACTION_FACE_ENROLLMENT_TIMEDOUT								503		//����ע�ᳬʱ
#define REMOTE_ACTION_CONNECTING_VIDEO_AND_AUDIO							504		//��������Ƶ����
#define REMOTE_ACTION_DISCONNECTING_VIDEO_AND_AUDIO							505		//��������Ƶ����
#define REMOTE_ACTION_ALARM_MODE_SYNC                                       506     //����ģʽͬ��
#define REMOTE_ACTION_END													1000

//ͼƬץ��ָ��(����ӿ�)
#define SAVING_SNAPSHOT_IMAGE_START											1001
#define SAVING_FACE_CAMERA_SNAPSHOT_IMAGE									1001   //�������������ץ��ͼ��
#define SAVING_DOOR_CAMERA_SNAPSHOT_IMAGE									1002   //�����Ż������ץ��ͼ��
#define SAVING_ENV_CAMERA_SNAPSHOT_IMAGE									1003   //���滷�������ץ��ͼ��
#define SAVING_SNAPSHOT_IMAGE_END											1050

/************************************************* ����¼� ************************************************************/
//�����¼�(����ӿ�)
#define IVA_BASIC_EVENT_START												1	
#define IVA_BASIC_EVENT_SYSTEM_INIT											1       //ϵͳ��ʼ��
#define IVA_BASIC_EVENT_FACE_ENROLLMENT										2       //����ע��
#define IVA_BASIC_EVENT_FACE_RECOGNITION									3       //����ʶ��
#define IVA_BASIC_EVENT_VIDEO_START_SAVING									4       //��Ƶ���濪ʼ
#define IVA_BASIC_EVENT_VIDEO_STOP_SAVING									5       //��Ƶ�������
#define IVA_BASIC_EVENT_ADJUST_LIGHT_INTENSITY								6		//������������ǿ��
#define IVA_BASIC_EVENT_VIDEO_AND_AUDIO_CONNECTION							7		//����Ƶ����
#define IVA_BASIC_EVENT_VIDEO_AND_AUDIO_DISCONNECTION						8		//����Ƶ���߹ر�
#define IVA_BASIC_EVENT_HEART_BEAT											9		//����
#define IVA_BASIC_EVENT_IMAGE_SNAPSHOT                                      10      //ͼƬץ��
#define IVA_BASIC_EVENT_OTHER_EVENT											49		//�����¼�
#define IVA_BASIC_EVENT_END													50

//ͼƬץ���¼�(����ӿ�)
#define IVA_SNAPSHOT_IMAGE_EVENT_START										51
#define IVA_SNAPSHOT_IMAGE_EVENT_REGION_ABNORMAL_ALARM					    52		//�Ż��쳣����
#define IVA_SNAPSHOT_IMAGE_EVENT_REGION_ABNORMAL_PRE_ALARM				    53		//�Ż��쳣Ԥ��
#define IVA_SNAPSHOT_IMAGE_EVENT_PERSON_HIJACKING_ALARM						54		//��Ա�ٳֱ���
#define IVA_SNAPSHOT_IMAGE_EVENT_FORGET_TO_LOCK_THE_DOOR_ALARM				55		//������������
#define IVA_SNAPSHOT_IMAGE_EVENT_PERSON_ENTERING_NORMAL						56		//��������
#define IVA_SNAPSHOT_IMAGE_EVENT_PERSON_LEAVING_NORMAL						57		//��������
#define IVA_SNAPSHOT_IMAGE_EVENT_PERSON_PASSING_BY							58		//��Ա·��
#define IVA_SNAPSHOT_IMAGE_EVENT_ENTERING_VERIFICATION                      59      //��֤����
#define IVA_SNAPSHOT_IMAGE_EVENT_LEAVING_VERIFICATION                       60      //��֤����
#define IVA_SNAPSHOT_IMAGE_EVENT_OCCLUSION_PRE_ALARM				        62		//������ڵ�Ԥ��
#define IVA_SNAPSHOT_IMAGE_EVENT_OCCLUSION_ALARM                            63      //������ڵ�����
#define IVA_SNAPSHOT_IMAGE_EVENT_DISARMING_RECORDING                        64      //����¼��
#define IVA_SNAPSHOT_IMAGE_EVENT_PERSON_HIJACKING_PRE_ALARM                 65      //��Ա�ٳ�Ԥ��
#define IVA_SNAPSHOT_IMAGE_EVENT_DOORBELL                                   66      //����
#define IVA_SNAPSHOT_IMAGE_EVENT_INTRUSIOM_PRE_ALARM                        67      //�Ƿ�����Ԥ��
#define IVA_SNAPSHOT_IMAGE_EVENT_INTRUSIOM_ALARM                            68      //�Ƿ����뱨��
#define IVA_SNAPSHOT_IMAGE_EVENT_REMOTE_OPEN_DOOR                           69      //Զ�̿���
#define IVA_SNAPSHOT_IMAGE_EVENT_ANTI_TAMPER_ALARM                          70      //���𱨾�
#define IVA_SNAPSHOT_IMAGE_EVENT_SET_ARMING                                 71      //����
#define IVA_SNAPSHOT_IMAGE_EVENT_SET_DISARMING                              72      //����
#define IVA_SNAPSHOT_IMAGE_EVENT_SET_COMMON_MODE                            73      //����Ϊ�ճ�ģʽ
#define IVA_SNAPSHOT_IMAGE_EVENT_SET_HOLIDAY_MODE                           74      //����Ϊ���ģʽ
#define IVA_SNAPSHOT_IMAGE_EVENT_DOOR_UNCLOSE_ALARM                         75      //��δ������
#define IVA_SNAPSHOT_IMAGE_EVENT_OPEN_DOOR_ALARM                            76      //���ű���
#define IVA_SNAPSHOT_IMAGE_EVENT_OPEN_DOOR                                  77      //������Ϣ
#define IVA_SNAPSHOT_IMAGE_EVENT_CLOSE_DOOR                                 78      //������Ϣ
#define IVA_SNAPSHOT_IMAGE_EVENT_DOOR_SENSOR_FAILURE                        79      //�ŴŹ���
#define IVA_SNAPSHOT_IMAGE_EVENT_DOOR_VIDEO_LOST                            80      //�����Ƶ��ʧ
#define IVA_SNAPSHOT_IMAGE_EVENT_FACE_VIDEO_LOST                            81      //������Ƶ��ʧ
#define IVA_SNAPSHOT_IMAGE_EVENT_DEVICE_STATUS                              82      //�豸״̬
#define IVA_SNAPSHOT_IMAGE_EVENT_WINDOW_VIDEO_LOST                          83      //������Ƶ��ʧ
#define	IVA_SNAPSHOT_IMAGE_EVENT_MIC_ERROR									84		//ʰ��������
#define	IVA_SNAPSHOT_IMAGE_EVENT_SPEAKER_ERROR								85		//����������
#define	IVA_SNAPSHOT_IMAGE_EVENT_IR_LIGHT_ERROR								86		//���ⲹ��ƹ���
#define	IVA_SNAPSHOT_IMAGE_EVENT_FACE_ENROLLMENT							87		//����ע��
#define IVA_SNAPSHOT_IMAGE_EVENT_ENV_VIDEO_LOST								88      //������Ƶ��ʧ
#define IVA_SNAPSHOT_IMAGE_EVENT_DOOR_SIGN_LOST								89      //�ű궪ʧ
#define IVA_SNAPSHOT_IMAGE_EVENT_PERSON_ENTERING_ABNORMAL					90      //��������
#define IVA_SNAPSHOT_IMAGE_EVENT_STOP_ALARM									97      //��������
#define IVA_SNAPSHOT_IMAGE_EVENT_INTERRUPTION_EVENT                         98      //�ж��¼�
#define IVA_SNAPSHOT_IMAGE_EVENT_OTHER_EVENT                                99      //�����¼�
#define IVA_SNAPSHOT_IMAGE_EVENT_END                                        100

//��Ƶ�����¼�(����ӿ�)
#define IVA_PLAY_SOUND_EVENT_START						101		 //������Ƶ�ļ��Ļ�ַ
#define IVA_PLAY_SOUND_EVENT_PLAY_SOUND_S0				101		 //������Ƶ�ļ�s0.wav    ����ע��
#define IVA_PLAY_SOUND_EVENT_PLAY_SOUND_S1				102      //������Ƶ�ļ�s1.wav	����ע��
#define IVA_PLAY_SOUND_EVENT_PLAY_SOUND_S2				103      //������Ƶ�ļ�s2.wav	����ע��
#define IVA_PLAY_SOUND_EVENT_PLAY_SOUND_S3				104      //������Ƶ�ļ�s3.wav	����ע��
#define IVA_PLAY_SOUND_EVENT_PLAY_SOUND_S4				105      //������Ƶ�ļ�s4.wav	����ע��
#define IVA_PLAY_SOUND_EVENT_PLAY_SOUND_S5				106      //������Ƶ�ļ�s5.wav	����ע��
#define IVA_PLAY_SOUND_EVENT_PLAY_SOUND_S6				107      //������Ƶ�ļ�s6.wav	����ע��
#define IVA_PLAY_SOUND_EVENT_PLAY_SOUND_S7				108      //������Ƶ�ļ�s7.wav	����ע��
#define IVA_PLAY_SOUND_EVENT_PLAY_SOUND_S8				109      //������Ƶ�ļ�s8.wav	����ע��
#define IVA_PLAY_SOUND_EVENT_PLAY_SOUND_S9				110      //������Ƶ�ļ�s9.wav	����ע��
#define IVA_PLAY_SOUND_EVENT_PLAY_SOUND_S10				111      //������Ƶ�ļ�s10.wav	����ע��
#define IVA_PLAY_SOUND_EVENT_PLAY_SOUND_S11				112      //������Ƶ�ļ�s11.wav	����ע��
#define IVA_PLAY_SOUND_EVENT_PLAY_SOUND_S12				113      //������Ƶ�ļ�s12.wav	����ע��
#define IVA_PLAY_SOUND_EVENT_PLAY_SOUND_S13				114      //������Ƶ�ļ�s13.wav	����ע��
#define IVA_PLAY_SOUND_EVENT_PLAY_SOUND_S14				115      //������Ƶ�ļ�s14.wav	����ע��
#define IVA_PLAY_SOUND_EVENT_PLAY_SOUND_S15				116      //������Ƶ�ļ�s15.wav	����ע��
#define IVA_PLAY_SOUND_EVENT_PLAY_SOUND_S16				117      //������Ƶ�ļ�s16.wav	����ע��
#define IVA_PLAY_SOUND_EVENT_PLAY_SOUND_S17				118      //������Ƶ�ļ�s17.wav	�Ż��쳣��������ʶ��
#define IVA_PLAY_SOUND_EVENT_PLAY_SOUND_S18				119      //������Ƶ�ļ�s18.wav	�����֤�ɹ�
#define IVA_PLAY_SOUND_EVENT_PLAY_SOUND_S19				120      //������Ƶ�ļ�s19.wav	�����֤ʧ��
#define IVA_PLAY_SOUND_EVENT_PLAY_SOUND_S20				121      //������Ƶ�ļ�s20.wav  ϵͳ��ʼ��
#define IVA_PLAY_SOUND_EVENT_PLAY_SOUND_S21				122      //������Ƶ�ļ�s21.wav  ϵͳ��ʼ����
#define IVA_PLAY_SOUND_EVENT_PLAY_SOUND_S22				123      //������Ƶ�ļ�s22.wav  ϵͳ��ʼ�����
#define IVA_PLAY_SOUND_EVENT_PLAY_SOUND_S23				124      //������Ƶ�ļ�s23.wav  ��������
#define IVA_PLAY_SOUND_EVENT_PLAY_SOUND_S24				125		 //������Ƶ�ļ�s24.wav  ��Ա�쳣����
#define IVA_PLAY_SOUND_EVENT_PLAY_SOUND_S25				126		 //������Ƶ�ļ�s25.wav  �����ŷ���Ԥ��
#define IVA_PLAY_SOUND_EVENT_PLAY_SOUND_S26				127		 //������Ƶ�ļ�s26.wav  �ǳ�+��ӭ�ؼ�
#define IVA_PLAY_SOUND_EVENT_PLAY_SOUND_S27				128		 //������Ƶ�ļ�s27.wav  �ǳ�+һ·ƽ��
#define IVA_PLAY_SOUND_EVENT_PLAY_SOUND_S28				129		 //������Ƶ�ļ�s28.wav  ���
#define IVA_PLAY_SOUND_EVENT_PLAY_SOUND_S29				130		 //������Ƶ�ļ�s29.wav  �ټ�
#define IVA_PLAY_SOUND_EVENT_PLAY_SOUND_S30				131		 //������Ƶ�ļ�s30.wav  
#define IVA_PLAY_SOUND_EVENT_PLAY_SOUND_S31				132		 //������Ƶ�ļ�s31.wav  
#define IVA_PLAY_SOUND_EVENT_PLAY_SOUND_S32				133		 //������Ƶ�ļ�s32.wav  
#define IVA_PLAY_SOUND_EVENT_PLAY_SOUND_S33				134		 //������Ƶ�ļ�s33.wav  
#define IVA_PLAY_SOUND_EVENT_PLAY_SOUND_S34				135		 //������Ƶ�ļ�s34.wav  
#define IVA_PLAY_SOUND_EVENT_PLAY_SOUND_S35				136		 //������Ƶ�ļ�s35.wav  
#define IVA_PLAY_SOUND_EVENT_END						500

/************************************************* ������ ************************************************************/
#define IVA_NO_ERROR								1			//�޴���
#define IVA_ERROR_INVALID_VIDEO_SIZE				-901		//��Ч��ͼ��ߴ�
#define IVA_ERROR_LOAD_CONFIG_FILE_FAILED			-902		//�����ļ�����ʧ��
#define IVA_ERROR_WRITE_CONFIG_FILE_FAILED			-903		//�����ļ�дʧ��
#define IVA_ERROR_INVALID_MODE						-904		//��Ч���㷨ģʽ
#define IVA_ERROR_MALLOC_FAILED						-905		//�ڴ����ʧ��
#define IVA_ERROR_PARAM_ERROR						-906		//���ò�������
#define IVA_ERROR_LOAD_FACE_DATABASE_FAILED			-907		//�������ݿ����ʧ��
#define IVA_ERROR_INVALID_HANDLE					-908		//��Ч���㷨���
#define IVA_ERROR_ALG_READ_ERROR					-909		//�������ݴ���
#define IVA_ERROR_ALG_READ_TIMEOUT					-910		//�������ݳ�ʱ
#define IVA_ERROR_RECEIVE_INVALID_USER_COMMAND		-911		//���յ���Ч���û�ָ��
#define IVA_ERROR_RECEIVE_USER_COMMAND_TIMEDOUT		-912		//�����û�ָ�ʱ
#define IVA_ERROR_INVALID_ARG_POINTER               -913		//��Ч���������ָ��
#define IVA_ERROR_INVALID_CONFIG_FILE_PATH          -914		//��Ч�������ļ�·��
#define IVA_ERROR_INVALID_CALLBACK_FUNCTION         -915		//��Ч�Ļص�����
#define IVA_ERROR_GET_USER_INFORMATION_FAILED       -916		//��ȡ�û�ע����Ϣʧ��
#define IVA_ERROR_FACE_DATABASE_IS_FULL             -917		//�������ݿ�����
#define IVA_ERROR_DELETE_USER_INFO_FAILED           -918		//ɾ���û���Ϣʧ��
#define IVA_ERROR_MODIFY_USER_INFO_FAILED           -919		//�޸��û���Ϣʧ��
#define IVA_ERROR_INVALID_ENROLLMENT_USER_INFO      -920		//��Ч��ע���û���Ϣ
#define IVA_ERROR_WRITE_AUX_CONFIG_PARAM_FAILED     -921		//���������ļ�дʧ��
#define IVA_ERROR_READ_AUX_CONFIG_PARAM_FAILED      -922		//���������ļ���ʧ��
#define IVA_ERROR_FACE_ENROLLMENT_FAILED            -923		//����ע��ʧ��
#define IVA_ERROR_FACE_VIDEO_LOST                   -924		//������Ƶ��ʧ
#define IVA_ERROR_SET_DOOR_STATUS_FAILED            -925		//�����Ż�״̬ʧ��
#define IVA_ERROR_WRITE_ALARM_SCHEDUAL_FAILED       -926		//�����ƻ�дʧ��
#define IVA_ERROR_READ_ALARM_SCHEDUAL_FAILED        -927		//�����ƻ���ʧ��
#define IVA_ERROR_WRITE_WORKING_SCHEDUAL_FAILED     -928		//�����ƻ�дʧ��
#define IVA_ERROR_READ_WORKING_SCHEDUAL_FAILED      -929		//�����ƻ���ʧ��
#pragma pack(push, 2)

/************************************************* ������� ************************************************************/

//���ο�����궨��
typedef struct TAG_IVA_RECT 
{
    uint32_t left;          //ռͼ���ȵİٷֱ�
    uint32_t right;         //ռͼ���ȵİٷֱ�
    uint32_t top;           //ռͼ��߶ȵİٷֱ�
    uint32_t bottom;        //ռͼ��߶ȵİٷֱ�
}IVA_RECT;

typedef struct TAG_IVA_POINT
{
    uint32_t x;             //ռͼ���ȵ�ǧ�ֱ�
    uint32_t y;             //ռͼ��߶ȵ�ǧ�ֱ�
}IVA_POINT;

//ָ��ص��ṹ��
typedef struct TAG_IVA_ACTION_OUTPUT
{
    uint8_t *speech;                            //����
    uint8_t *imgbuf;                            //ץ��ͼ��(YUV420P��ʽ)
    uint32_t img_w;                             //ץ��ͼ��Ŀ��
    uint32_t img_h;                             //ץ��ͼ��ĸ߶�
    uint32_t action_code;                       //ָ���룬�μ�ָ���붨��
    uint32_t event_type;                        //�¼����ͣ��μ��¼����Ͷ���
    uint32_t event_priority;                    //�¼����ȼ�[0-100]��ֵԽ�����ȼ�Խ��
    uint32_t upload_flg;                        //�¼��Ƿ���Ҫ�ϴ��ı�ʶ 0������Ҫ 1����Ҫ
    uint32_t save_flg;                          //�¼��Ƿ���Ҫ���ر���ı�ʶ 0������Ҫ 1����Ҫ
    uint32_t arming_flg;                        //��/����״̬��1������ 0������
    uint32_t alarm_mode;                        //����ģʽ��1������ģʽ 0��ͨ��ģʽ
    uint32_t command;                           //����ָ��
    uint32_t gain;                              //�������
    uint32_t exposure_time;                     //����ع�ʱ��(΢��)
    uint32_t user_arg;                          //�ⲿ�û�����
    uint32_t address;                           //�ⲿ��ַ
    uint32_t unique_code;                       //Ψһ��
}IVA_ACTION_OUTPUT;

//������Ƶ����������Ϣ���
typedef struct TAG_IVA_DEBUG_OUTPUT
{
    uint32_t iva_mode;                          //���ģʽ
    IVA_RECT door_intrusion_roi;                //�Ż����ּ������(�Ż������)
    IVA_RECT door_roi;                          //�Ż�����(�Ż������)
    IVA_RECT lock_roi;                          //�����˶��������(�Ż������)
    IVA_RECT face_pos;                          //������ͼ���е�λ��(�Ż������)
    IVA_POINT lefteye_pos;                      //������ͼ���е�λ��(�Ż������)
    IVA_POINT righteye_pos;                     //������ͼ���е�λ��(�Ż������)
    IVA_RECT door_obj_pos[5];                   //����Ŀ����ͼ���е�λ��(�Ż������)
    int32_t person_id;                          //ʶ�������ԱID��
    uint32_t progress_rate;                     //����ע�����(0-100)
    uint32_t img_w;                             //����ͼ��Ŀ��
    uint32_t img_h;                             //����ͼ��ĸ߶�
    uint8_t *debug_img;                         //����ͼ��
    int8_t debug_msg[200];                      //������Ϣ
    uint32_t user_arg;                          //�ⲿ�û�����
}IVA_DEBUG_OUTPUT;

#pragma pack(pop)

/************************************************* �ӿں��� ************************************************************/

typedef void* IVA_HANDLE;       //������Ƶ�������

//������Ƶ�����ص����������ڵ���
typedef void (*IVA_DEBUG_CALLBACK_FUN)(IVA_DEBUG_OUTPUT *debug_output);

//������Ƶ�����ص����������ڷ���ָ��
typedef void (*IVA_ACTION_CALLBACK_FUN)(IVA_ACTION_OUTPUT *action_output);

//�㷨�����ṹ��
typedef struct TAG_ALG_CREATE_ARG
{
    int8_t* config_filename;                        //�����ļ���(��������·��)
    int8_t* aux_config_filename;                    //���������ļ���(��������·��)
    IVA_DEBUG_CALLBACK_FUN iva_debug_callback;      //���Իص�����
    IVA_ACTION_CALLBACK_FUN iva_action_callback;    //ָ��ص�����
    uint32_t face_img_w;                            //������Ƶ�Ŀ��
    uint32_t face_img_h;                            //������Ƶ�ĸ߶�
    uint32_t door_img_w;                            //�Ż���Ƶ�Ŀ��
    uint32_t door_img_h;                            //�Ż���Ƶ�ĸ߶�
	uint32_t env_img_w;								//������Ƶ�Ŀ��
	uint32_t env_img_h;								//������Ƶ�ĸ߶�
    uint32_t user_arg;                              //�ⲿ�û�����
    uint8_t iva_mode;                               //�㷨ģʽ
}ALG_CREATE_ARG;

//����֡����
typedef struct TAG_IVA_FRAME
{
    uint32_t type;                                          //����֡���ͣ��μ�����֡����
    uint32_t datalen;                                       //���ݳ���
    uint8_t *data;                                          //����
}IVA_FRAME;

//����������Ƶ�����㷨������ֵ��������
BVS_DLL_API int32_t iva_alg_create(IVA_HANDLE *handle, ALG_CREATE_ARG *alg_arg);

//������Ƶ�����㷨��ȡһ֡���ݣ�����ֵ��������
BVS_DLL_API int32_t iva_alg_read_one_frame(IVA_HANDLE handle, IVA_FRAME *frame);

//������Ƶ���������û�ָ��(xml�ӿ�)�����û�ָ���룬����ֵ��������
BVS_DLL_API int32_t iva_alg_receive_user_command_in_xml_format(IVA_HANDLE handle, uint8_t user_command, char *user_info_xml, 
                                                                uint32_t info_length, uint32_t address);

//������Ƶ������ȡ�û���Ϣ(xml�ӿ�)������ֵ��������
BVS_DLL_API int32_t iva_alg_get_all_user_information_in_xml_format(IVA_HANDLE handle, char *user_info_xml);

//������Ƶ������ȡ�û�ע������ͼ��(yuv420��ʽ��100x100),����ֵ��������
BVS_DLL_API int32_t iva_alg_get_user_face_image(IVA_HANDLE handle, int8_t *user_name, uint8_t *face_image);

//������Ƶ����д����������Ϣ(xml�ӿ�)������ֵ��������
BVS_DLL_API int32_t iva_alg_write_aux_config_file_in_xml_format(IVA_HANDLE handle, char *aux_config_param_xml, int8_t aux_config_filename[]);

//������Ƶ����������������Ϣ(xml�ӿ�)������ֵ��������
BVS_DLL_API int32_t iva_alg_read_aux_config_file_in_xml_format(IVA_HANDLE handle, char *aux_config_param_xml, int8_t aux_config_filename[]);

//������Ƶ������������������Ϣ(xml�ӿ�)������ֵ��������
BVS_DLL_API int32_t iva_alg_read_shared_aux_config_file_in_xml_format(IVA_HANDLE handle, char *aux_config_param_xml, int8_t aux_config_filename[]);

//������Ƶ����д�����ƻ�
BVS_DLL_API int32_t iva_alg_write_alarm_schedual_file_in_xml_format(IVA_HANDLE handle, char *aux_config_param_xml);

//������Ƶ�����������ƻ�
BVS_DLL_API int32_t iva_alg_read_alarm_schedual_in_xml_format(IVA_HANDLE handle, char *alarm_schedual_xml);

//������Ƶ����д�����ƻ�
BVS_DLL_API int32_t iva_alg_write_working_schedual_file_in_xml_format(IVA_HANDLE handle, char *working_schedual_xml);

//������Ƶ�����������ƻ�
BVS_DLL_API int32_t iva_alg_read_working_schedual_in_xml_format(IVA_HANDLE handle, char *working_schedual_xml);

//������Ƶ��������ʱ��
BVS_DLL_API int32_t iva_alg_set_time_zone(IVA_HANDLE handle, int32_t time_zone);

//����������Ƶ�����㷨
BVS_DLL_API void iva_alg_destroy(IVA_HANDLE handle);

#ifdef __cplusplus
};
#endif

#endif //_IVA_INTERFACE_H_
