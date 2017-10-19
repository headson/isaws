/****************************
版权所有,百威讯公司
文件名: iva_interface.h
作者:   周建雄
版本:   1.0.0
完成日期:   2013/5/3
描述:       DS智能视频分析接口
其它:
主要函数列表:

修改历史记录列表:
<修改者>   <修改日期>  <版本>    <描述>

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

#define MAX_FILE_LEN 1024       //文件的最大长度
    
//数据帧类型(输入接口)
#define FRAME_TYPE_DOOR_VIDEO           0				//门户视频
#define FRAME_TYPE_FACE_VIDEO           1				//人像视频
#define FRAME_TYPE_ENV_VIDEO			2				//环境视频

//算法模式(输入接口)
#define IVA_MODE_START									1
#define IVA_MODE_ADVANCED_DOOR_SURVEILLANCE				1	//(三目)门户监控(带报警功能)
#define IVA_MODE_INDOOR_FACE_RECOGNITION				2	//(双目/单目)室内考勤(不带报警)
#define IVA_MODE_ADVANCED_INDOOR_DOOR_CONTROLLING		3	//(三目)室内考勤(带报警功能)
#define IVA_MODE_DOOR_SURVEILLANCE						4	//双目/单目 门户监控（不带人脸识别）
#define IVA_MODE_OUTDOOR_FACE_RECOGNITION				5	//双目/单目 室外考勤（不带报警）
#define IVA_MODE_BALCONY_SURVEILLANCE					6	//双目 花园监控
#define IVA_MODE_END									10

/************************************************* 输入指令 ************************************************************/
//外部指令(输入接口)
#define USER_COMMAND_FACE_ENROLLMENT                1       //人脸注册
#define USER_COMMAND_FAST_FACE_ENROLLMENT           2       //快速人脸注册
#define USER_COMMAND_FACE_ENROLLMENT_CANCLING       3       //人脸注册取消
#define USER_COMMAND_DELETE_USER_INFO               4       //删除用户信息
#define USER_COMMAND_MODIFY_USER_INFO               5       //修改用户信息
#define USER_COMMAND_AUX_CONFIG_PARAM_UPDATE        6       //辅助配置参数更新
#define USER_COMMAND_OPEN_DOOR1                     7       //开门
#define USER_COMMAND_OPEN_DOOR2                     8       //开门
#define USER_COMMAND_OPEN_DOOR3                     9       //开门
#define USER_COMMAND_OPEN_DOOR4                     10      //开门
#define USER_COMMAND_OPEN_VOICE_INTERCOM			18		//开启语音对讲	
#define USER_COMMAND_DOORBELL						19      //门铃
#define USER_COMMAND_ENTERING_ENROLLMENT_INTERFACE	20		//进入人脸注册
#define USER_COMMAND_LEAVING_ENROLLMENT_INTERFACE	21		//离开人脸注册
#define USER_COMMAND_OPEN_DOOR                      69      //开门
#define USER_COMMAND_USER_SET_DOOR_OPEN             70      //用户设置门开
#define USER_COMMAND_USER_SET_DOOR_CLOSE            71      //用户设置门关
#define USER_COMMAND_USER_SET_TURN_ON_LIGHT         72      //用户设置开灯
#define USER_COMMAND_USER_SET_DOOR_RESET            73      //用户复位门户状态
#define USER_COMMAND_CHANGE_LIGHT_INTENSITY			74		//调整照明灯亮度
#define USER_COMMAND_VIDEO_PREVIEW_TURN_ON_LIGHT    75      //视频预览开灯
#define USER_COMMAND_IDENTITY_VERIFICATION_SUCCESS  82      //身份验证成功
#define USER_COMMAND_STOP_ALARM                     83      //停止报警
#define USER_COMMAND_ALG_RESET                      84      //算法复位
#define USER_COMMAND_FACE_SNAPSHOT		            85      //人脸图片抓拍
#define USER_COMMAND_ARMING				            86      //布防
#define USER_COMMAND_DISARMING				        87      //撤防
#define USER_COMMAND_AUX_PARAM_UPDATE               88      //辅助配置参数更新
#define USER_COMMAND_DOOR_SNAPSHOT                  89      //门户图片抓拍
#define USER_COMMAND_HIJACKING                      90      //劫持
#define USER_COMMAND_ANTI_TAMPER_ALARM              91      //防拆报警
#define USER_COMMAND_ENV_SNAPSHOT					92      //环境图片抓拍
#define USER_COMMAND_CTRL                           4000    //外部控制

/************************************************* 输出动作 ************************************************************/
//本地动作指令(输出接口)
#define LOCAL_ACTION_START													1
#define LOCAL_ACTION_VIDEO_START_SAVING										1		//环境和门户摄像机开始保存视频
#define LOCAL_ACTION_VIDEO_STOP_SAVING										2		//环境和门户摄像机停止保存视频
#define LOCAL_ACTION_TURN_ON_LIGHT											3		//开启照明
#define LOCAL_ACTION_TURN_OFF_LIGHT											4		//关闭照明
#define LOCAL_ACTION_ADJUST_IR_LIGHT_INTENSITY							    5		//调整红外补光灯强度
#define LOCAL_ACTION_TURN_OFF_IR_LIGHT										6		//关闭红外照明
#define LOCAL_ACTION_UNLOCK_STOP_ALARM										7		//开锁异常停止报警
#define LOCAL_ACTION_ADJUST_DOOR_CAMERA_EXPOSUER							8		//调整门户相机曝光时间
#define LOCAL_ACTION_ADJUST_FACE_CAMERA_EXPOSUER							9		//调整人像相机曝光时间
#define LOCAL_ACTION_ADJUST_LIGHT_INTENSITY									10		//调整环境照明强度
#define LOCAL_ACTION_PLAY_SOUND												11		//播放音频
#define LOCAL_ACTION_FACE_ENROLLMENT_CANCLLING								12		//取消人脸注册
#define LOCAL_ACTION_HEART_BEAT_RESPONDENCE									13		//心跳回应
#define LOCAL_ACTION_PLAY_EDITABLE_SOUND								    14		//播放可编辑的音频
#define LOCAL_ACTION_TURN_ON_LASER_LIGHT							        15		//开启激光
#define LOCAL_ACTION_TURN_OFF_LASER_LIGHT									16		//关闭激光
#define LOCAL_ACTION_TURN_ON_DOOR_GUARD									    17		//打开门禁开关
#define LOCAL_ACTION_TURN_OFF_DOOR_GUARD								    18		//关闭门禁开关
#define LOCAL_ACTION_SET_MOUNTING_HEIGHT								    19		//设置安装高度
#define LOCAL_ACTION_RECOVER_SOUND                                          20      //恢复播放语音
#define LOCAL_ACTION_RESTART_DEVICE                                         21      //重启设备
#define LOCAL_ACTION_CHANGE_TO_DOOR_RECORDING								22      //切换为门户录像
#define LOCAL_ACTION_CHANGE_TO_ENV_RECORDING								23      //切换为环境录像
#define LOCAL_ACTION_TURN_ON_INDICATOR_LIGHT								24		//开启指示灯
#define LOCAL_ACTION_TURN_OFF_INDICATOR_LIGHT								25		//关闭指示灯
#define LOCAL_ACTION_RESET_NETWORK											26		//重置网络
#define LOCAL_ACTION_RESET_DOOR_SENSOR										27		//重置监控视频传感器
#define LOCAL_ACTION_END													500

//远程动作指令(输出接口)
#define REMOTE_ACTION_START													501
#define REMOTE_ACTION_FACE_ENROLLMENT_START									501		//人脸注册开始
#define REMOTE_ACTION_FACE_ENROLLMENT_SUCCESS								502		//人脸注册成功
#define REMOTE_ACTION_FACE_ENROLLMENT_TIMEDOUT								503		//人脸注册超时
#define REMOTE_ACTION_CONNECTING_VIDEO_AND_AUDIO							504		//开启音视频连线
#define REMOTE_ACTION_DISCONNECTING_VIDEO_AND_AUDIO							505		//结束音视频连线
#define REMOTE_ACTION_ALARM_MODE_SYNC                                       506     //报警模式同步
#define REMOTE_ACTION_END													1000

//图片抓拍指令(输出接口)
#define SAVING_SNAPSHOT_IMAGE_START											1001
#define SAVING_FACE_CAMERA_SNAPSHOT_IMAGE									1001   //保存人像摄像机抓拍图像
#define SAVING_DOOR_CAMERA_SNAPSHOT_IMAGE									1002   //保存门户摄像机抓拍图像
#define SAVING_ENV_CAMERA_SNAPSHOT_IMAGE									1003   //保存环境摄像机抓拍图像
#define SAVING_SNAPSHOT_IMAGE_END											1050

/************************************************* 输出事件 ************************************************************/
//基础事件(输出接口)
#define IVA_BASIC_EVENT_START												1	
#define IVA_BASIC_EVENT_SYSTEM_INIT											1       //系统初始化
#define IVA_BASIC_EVENT_FACE_ENROLLMENT										2       //人脸注册
#define IVA_BASIC_EVENT_FACE_RECOGNITION									3       //人脸识别
#define IVA_BASIC_EVENT_VIDEO_START_SAVING									4       //视频保存开始
#define IVA_BASIC_EVENT_VIDEO_STOP_SAVING									5       //视频保存结束
#define IVA_BASIC_EVENT_ADJUST_LIGHT_INTENSITY								6		//调整环境照明强度
#define IVA_BASIC_EVENT_VIDEO_AND_AUDIO_CONNECTION							7		//音视频连线
#define IVA_BASIC_EVENT_VIDEO_AND_AUDIO_DISCONNECTION						8		//音视频连线关闭
#define IVA_BASIC_EVENT_HEART_BEAT											9		//心跳
#define IVA_BASIC_EVENT_IMAGE_SNAPSHOT                                      10      //图片抓拍
#define IVA_BASIC_EVENT_OTHER_EVENT											49		//其他事件
#define IVA_BASIC_EVENT_END													50

//图片抓拍事件(输出接口)
#define IVA_SNAPSHOT_IMAGE_EVENT_START										51
#define IVA_SNAPSHOT_IMAGE_EVENT_REGION_ABNORMAL_ALARM					    52		//门户异常报警
#define IVA_SNAPSHOT_IMAGE_EVENT_REGION_ABNORMAL_PRE_ALARM				    53		//门户异常预警
#define IVA_SNAPSHOT_IMAGE_EVENT_PERSON_HIJACKING_ALARM						54		//人员劫持报警
#define IVA_SNAPSHOT_IMAGE_EVENT_FORGET_TO_LOCK_THE_DOOR_ALARM				55		//忘记锁门提醒
#define IVA_SNAPSHOT_IMAGE_EVENT_PERSON_ENTERING_NORMAL						56		//正常进门
#define IVA_SNAPSHOT_IMAGE_EVENT_PERSON_LEAVING_NORMAL						57		//正常出门
#define IVA_SNAPSHOT_IMAGE_EVENT_PERSON_PASSING_BY							58		//人员路过
#define IVA_SNAPSHOT_IMAGE_EVENT_ENTERING_VERIFICATION                      59      //认证进门
#define IVA_SNAPSHOT_IMAGE_EVENT_LEAVING_VERIFICATION                       60      //认证出门
#define IVA_SNAPSHOT_IMAGE_EVENT_OCCLUSION_PRE_ALARM				        62		//摄像机遮挡预警
#define IVA_SNAPSHOT_IMAGE_EVENT_OCCLUSION_ALARM                            63      //摄像机遮挡报警
#define IVA_SNAPSHOT_IMAGE_EVENT_DISARMING_RECORDING                        64      //撤防录像
#define IVA_SNAPSHOT_IMAGE_EVENT_PERSON_HIJACKING_PRE_ALARM                 65      //人员劫持预警
#define IVA_SNAPSHOT_IMAGE_EVENT_DOORBELL                                   66      //门铃
#define IVA_SNAPSHOT_IMAGE_EVENT_INTRUSIOM_PRE_ALARM                        67      //非法闯入预警
#define IVA_SNAPSHOT_IMAGE_EVENT_INTRUSIOM_ALARM                            68      //非法闯入报警
#define IVA_SNAPSHOT_IMAGE_EVENT_REMOTE_OPEN_DOOR                           69      //远程开门
#define IVA_SNAPSHOT_IMAGE_EVENT_ANTI_TAMPER_ALARM                          70      //防拆报警
#define IVA_SNAPSHOT_IMAGE_EVENT_SET_ARMING                                 71      //布防
#define IVA_SNAPSHOT_IMAGE_EVENT_SET_DISARMING                              72      //撤防
#define IVA_SNAPSHOT_IMAGE_EVENT_SET_COMMON_MODE                            73      //设置为日常模式
#define IVA_SNAPSHOT_IMAGE_EVENT_SET_HOLIDAY_MODE                           74      //设置为外出模式
#define IVA_SNAPSHOT_IMAGE_EVENT_DOOR_UNCLOSE_ALARM                         75      //门未关提醒
#define IVA_SNAPSHOT_IMAGE_EVENT_OPEN_DOOR_ALARM                            76      //开门报警
#define IVA_SNAPSHOT_IMAGE_EVENT_OPEN_DOOR                                  77      //开门消息
#define IVA_SNAPSHOT_IMAGE_EVENT_CLOSE_DOOR                                 78      //关门消息
#define IVA_SNAPSHOT_IMAGE_EVENT_DOOR_SENSOR_FAILURE                        79      //门磁故障
#define IVA_SNAPSHOT_IMAGE_EVENT_DOOR_VIDEO_LOST                            80      //监控视频丢失
#define IVA_SNAPSHOT_IMAGE_EVENT_FACE_VIDEO_LOST                            81      //人像视频丢失
#define IVA_SNAPSHOT_IMAGE_EVENT_DEVICE_STATUS                              82      //设备状态
#define IVA_SNAPSHOT_IMAGE_EVENT_WINDOW_VIDEO_LOST                          83      //窗户视频丢失
#define	IVA_SNAPSHOT_IMAGE_EVENT_MIC_ERROR									84		//拾音器故障
#define	IVA_SNAPSHOT_IMAGE_EVENT_SPEAKER_ERROR								85		//扬声器故障
#define	IVA_SNAPSHOT_IMAGE_EVENT_IR_LIGHT_ERROR								86		//红外补光灯故障
#define	IVA_SNAPSHOT_IMAGE_EVENT_FACE_ENROLLMENT							87		//人脸注册
#define IVA_SNAPSHOT_IMAGE_EVENT_ENV_VIDEO_LOST								88      //环境视频丢失
#define IVA_SNAPSHOT_IMAGE_EVENT_DOOR_SIGN_LOST								89      //门标丢失
#define IVA_SNAPSHOT_IMAGE_EVENT_PERSON_ENTERING_ABNORMAL					90      //其他进门
#define IVA_SNAPSHOT_IMAGE_EVENT_STOP_ALARM									97      //修正报警
#define IVA_SNAPSHOT_IMAGE_EVENT_INTERRUPTION_EVENT                         98      //中断事件
#define IVA_SNAPSHOT_IMAGE_EVENT_OTHER_EVENT                                99      //其它事件
#define IVA_SNAPSHOT_IMAGE_EVENT_END                                        100

//音频播放事件(输出接口)
#define IVA_PLAY_SOUND_EVENT_START						101		 //播放音频文件的基址
#define IVA_PLAY_SOUND_EVENT_PLAY_SOUND_S0				101		 //播放音频文件s0.wav    人脸注册
#define IVA_PLAY_SOUND_EVENT_PLAY_SOUND_S1				102      //播放音频文件s1.wav	人脸注册
#define IVA_PLAY_SOUND_EVENT_PLAY_SOUND_S2				103      //播放音频文件s2.wav	人脸注册
#define IVA_PLAY_SOUND_EVENT_PLAY_SOUND_S3				104      //播放音频文件s3.wav	人脸注册
#define IVA_PLAY_SOUND_EVENT_PLAY_SOUND_S4				105      //播放音频文件s4.wav	人脸注册
#define IVA_PLAY_SOUND_EVENT_PLAY_SOUND_S5				106      //播放音频文件s5.wav	人脸注册
#define IVA_PLAY_SOUND_EVENT_PLAY_SOUND_S6				107      //播放音频文件s6.wav	人脸注册
#define IVA_PLAY_SOUND_EVENT_PLAY_SOUND_S7				108      //播放音频文件s7.wav	人脸注册
#define IVA_PLAY_SOUND_EVENT_PLAY_SOUND_S8				109      //播放音频文件s8.wav	人脸注册
#define IVA_PLAY_SOUND_EVENT_PLAY_SOUND_S9				110      //播放音频文件s9.wav	人脸注册
#define IVA_PLAY_SOUND_EVENT_PLAY_SOUND_S10				111      //播放音频文件s10.wav	人脸注册
#define IVA_PLAY_SOUND_EVENT_PLAY_SOUND_S11				112      //播放音频文件s11.wav	人脸注册
#define IVA_PLAY_SOUND_EVENT_PLAY_SOUND_S12				113      //播放音频文件s12.wav	人脸注册
#define IVA_PLAY_SOUND_EVENT_PLAY_SOUND_S13				114      //播放音频文件s13.wav	人脸注册
#define IVA_PLAY_SOUND_EVENT_PLAY_SOUND_S14				115      //播放音频文件s14.wav	人脸注册
#define IVA_PLAY_SOUND_EVENT_PLAY_SOUND_S15				116      //播放音频文件s15.wav	人脸注册
#define IVA_PLAY_SOUND_EVENT_PLAY_SOUND_S16				117      //播放音频文件s16.wav	人脸注册
#define IVA_PLAY_SOUND_EVENT_PLAY_SOUND_S17				118      //播放音频文件s17.wav	门户异常请配合身份识别
#define IVA_PLAY_SOUND_EVENT_PLAY_SOUND_S18				119      //播放音频文件s18.wav	身份验证成功
#define IVA_PLAY_SOUND_EVENT_PLAY_SOUND_S19				120      //播放音频文件s19.wav	身份验证失败
#define IVA_PLAY_SOUND_EVENT_PLAY_SOUND_S20				121      //播放音频文件s20.wav  系统初始化
#define IVA_PLAY_SOUND_EVENT_PLAY_SOUND_S21				122      //播放音频文件s21.wav  系统初始化中
#define IVA_PLAY_SOUND_EVENT_PLAY_SOUND_S22				123      //播放音频文件s22.wav  系统初始化完成
#define IVA_PLAY_SOUND_EVENT_PLAY_SOUND_S23				124      //播放音频文件s23.wav  开锁报警
#define IVA_PLAY_SOUND_EVENT_PLAY_SOUND_S24				125		 //播放音频文件s24.wav  人员异常报警
#define IVA_PLAY_SOUND_EVENT_PLAY_SOUND_S25				126		 //播放音频文件s25.wav  忘记门反锁预警
#define IVA_PLAY_SOUND_EVENT_PLAY_SOUND_S26				127		 //播放音频文件s26.wav  昵称+欢迎回家
#define IVA_PLAY_SOUND_EVENT_PLAY_SOUND_S27				128		 //播放音频文件s27.wav  昵称+一路平安
#define IVA_PLAY_SOUND_EVENT_PLAY_SOUND_S28				129		 //播放音频文件s28.wav  你好
#define IVA_PLAY_SOUND_EVENT_PLAY_SOUND_S29				130		 //播放音频文件s29.wav  再见
#define IVA_PLAY_SOUND_EVENT_PLAY_SOUND_S30				131		 //播放音频文件s30.wav  
#define IVA_PLAY_SOUND_EVENT_PLAY_SOUND_S31				132		 //播放音频文件s31.wav  
#define IVA_PLAY_SOUND_EVENT_PLAY_SOUND_S32				133		 //播放音频文件s32.wav  
#define IVA_PLAY_SOUND_EVENT_PLAY_SOUND_S33				134		 //播放音频文件s33.wav  
#define IVA_PLAY_SOUND_EVENT_PLAY_SOUND_S34				135		 //播放音频文件s34.wav  
#define IVA_PLAY_SOUND_EVENT_PLAY_SOUND_S35				136		 //播放音频文件s35.wav  
#define IVA_PLAY_SOUND_EVENT_END						500

/************************************************* 错误码 ************************************************************/
#define IVA_NO_ERROR								1			//无错误
#define IVA_ERROR_INVALID_VIDEO_SIZE				-901		//无效的图像尺寸
#define IVA_ERROR_LOAD_CONFIG_FILE_FAILED			-902		//配置文件加载失败
#define IVA_ERROR_WRITE_CONFIG_FILE_FAILED			-903		//配置文件写失败
#define IVA_ERROR_INVALID_MODE						-904		//无效的算法模式
#define IVA_ERROR_MALLOC_FAILED						-905		//内存分配失败
#define IVA_ERROR_PARAM_ERROR						-906		//配置参数错误
#define IVA_ERROR_LOAD_FACE_DATABASE_FAILED			-907		//人脸数据库加载失败
#define IVA_ERROR_INVALID_HANDLE					-908		//无效的算法句柄
#define IVA_ERROR_ALG_READ_ERROR					-909		//读入数据错误
#define IVA_ERROR_ALG_READ_TIMEOUT					-910		//读入数据超时
#define IVA_ERROR_RECEIVE_INVALID_USER_COMMAND		-911		//接收到无效的用户指令
#define IVA_ERROR_RECEIVE_USER_COMMAND_TIMEDOUT		-912		//接收用户指令超时
#define IVA_ERROR_INVALID_ARG_POINTER               -913		//无效的输入参数指针
#define IVA_ERROR_INVALID_CONFIG_FILE_PATH          -914		//无效的配置文件路径
#define IVA_ERROR_INVALID_CALLBACK_FUNCTION         -915		//无效的回调函数
#define IVA_ERROR_GET_USER_INFORMATION_FAILED       -916		//获取用户注册信息失败
#define IVA_ERROR_FACE_DATABASE_IS_FULL             -917		//人脸数据库已满
#define IVA_ERROR_DELETE_USER_INFO_FAILED           -918		//删除用户信息失败
#define IVA_ERROR_MODIFY_USER_INFO_FAILED           -919		//修改用户信息失败
#define IVA_ERROR_INVALID_ENROLLMENT_USER_INFO      -920		//无效的注册用户信息
#define IVA_ERROR_WRITE_AUX_CONFIG_PARAM_FAILED     -921		//辅助配置文件写失败
#define IVA_ERROR_READ_AUX_CONFIG_PARAM_FAILED      -922		//辅助配置文件读失败
#define IVA_ERROR_FACE_ENROLLMENT_FAILED            -923		//人脸注册失败
#define IVA_ERROR_FACE_VIDEO_LOST                   -924		//人像视频丢失
#define IVA_ERROR_SET_DOOR_STATUS_FAILED            -925		//设置门户状态失败
#define IVA_ERROR_WRITE_ALARM_SCHEDUAL_FAILED       -926		//报警计划写失败
#define IVA_ERROR_READ_ALARM_SCHEDUAL_FAILED        -927		//报警计划读失败
#define IVA_ERROR_WRITE_WORKING_SCHEDUAL_FAILED     -928		//工作计划写失败
#define IVA_ERROR_READ_WORKING_SCHEDUAL_FAILED      -929		//工作计划读失败
#pragma pack(push, 2)

/************************************************* 输出参数 ************************************************************/

//矩形框的坐标定义
typedef struct TAG_IVA_RECT 
{
    uint32_t left;          //占图像宽度的百分比
    uint32_t right;         //占图像宽度的百分比
    uint32_t top;           //占图像高度的百分比
    uint32_t bottom;        //占图像高度的百分比
}IVA_RECT;

typedef struct TAG_IVA_POINT
{
    uint32_t x;             //占图像宽度的千分比
    uint32_t y;             //占图像高度的千分比
}IVA_POINT;

//指令回调结构体
typedef struct TAG_IVA_ACTION_OUTPUT
{
    uint8_t *speech;                            //语音
    uint8_t *imgbuf;                            //抓拍图像(YUV420P格式)
    uint32_t img_w;                             //抓拍图像的宽度
    uint32_t img_h;                             //抓拍图像的高度
    uint32_t action_code;                       //指令码，参见指令码定义
    uint32_t event_type;                        //事件类型，参见事件类型定义
    uint32_t event_priority;                    //事件优先级[0-100]，值越大优先级越高
    uint32_t upload_flg;                        //事件是否需要上传的标识 0：不需要 1：需要
    uint32_t save_flg;                          //事件是否需要本地保存的标识 0：不需要 1：需要
    uint32_t arming_flg;                        //布/撤防状态：1：布防 0：撤防
    uint32_t alarm_mode;                        //报警模式：1：假期模式 0：通用模式
    uint32_t command;                           //反馈指令
    uint32_t gain;                              //相机增益
    uint32_t exposure_time;                     //相机曝光时间(微秒)
    uint32_t user_arg;                          //外部用户参数
    uint32_t address;                           //外部地址
    uint32_t unique_code;                       //唯一码
}IVA_ACTION_OUTPUT;

//智能视频分析调试信息输出
typedef struct TAG_IVA_DEBUG_OUTPUT
{
    uint32_t iva_mode;                          //检测模式
    IVA_RECT door_intrusion_roi;                //门户入侵检测区域(门户摄像机)
    IVA_RECT door_roi;                          //门户区域(门户摄像机)
    IVA_RECT lock_roi;                          //锁孔运动检测区域(门户摄像机)
    IVA_RECT face_pos;                          //人脸在图像中的位置(门户摄像机)
    IVA_POINT lefteye_pos;                      //左眼在图像中的位置(门户摄像机)
    IVA_POINT righteye_pos;                     //右眼在图像中的位置(门户摄像机)
    IVA_RECT door_obj_pos[5];                   //入侵目标在图像中的位置(门户摄像机)
    int32_t person_id;                          //识别出的人员ID号
    uint32_t progress_rate;                     //人脸注册进度(0-100)
    uint32_t img_w;                             //调试图像的宽度
    uint32_t img_h;                             //调试图像的高度
    uint8_t *debug_img;                         //调试图像
    int8_t debug_msg[200];                      //调试信息
    uint32_t user_arg;                          //外部用户参数
}IVA_DEBUG_OUTPUT;

#pragma pack(pop)

/************************************************* 接口函数 ************************************************************/

typedef void* IVA_HANDLE;       //智能视频分析句柄

//智能视频分析回调函数，用于调试
typedef void (*IVA_DEBUG_CALLBACK_FUN)(IVA_DEBUG_OUTPUT *debug_output);

//智能视频分析回调函数，用于发送指令
typedef void (*IVA_ACTION_CALLBACK_FUN)(IVA_ACTION_OUTPUT *action_output);

//算法创建结构体
typedef struct TAG_ALG_CREATE_ARG
{
    int8_t* config_filename;                        //配置文件名(包括完整路径)
    int8_t* aux_config_filename;                    //辅助配置文件名(包括完整路径)
    IVA_DEBUG_CALLBACK_FUN iva_debug_callback;      //调试回调函数
    IVA_ACTION_CALLBACK_FUN iva_action_callback;    //指令回调函数
    uint32_t face_img_w;                            //人像视频的宽度
    uint32_t face_img_h;                            //人像视频的高度
    uint32_t door_img_w;                            //门户视频的宽度
    uint32_t door_img_h;                            //门户视频的高度
	uint32_t env_img_w;								//环境视频的宽度
	uint32_t env_img_h;								//环境视频的高度
    uint32_t user_arg;                              //外部用户参数
    uint8_t iva_mode;                               //算法模式
}ALG_CREATE_ARG;

//数据帧定义
typedef struct TAG_IVA_FRAME
{
    uint32_t type;                                          //数据帧类型，参见数据帧定义
    uint32_t datalen;                                       //数据长度
    uint8_t *data;                                          //数据
}IVA_FRAME;

//创建智能视频分析算法：返回值见错误码
BVS_DLL_API int32_t iva_alg_create(IVA_HANDLE *handle, ALG_CREATE_ARG *alg_arg);

//智能视频分析算法读取一帧数据：返回值见错误码
BVS_DLL_API int32_t iva_alg_read_one_frame(IVA_HANDLE handle, IVA_FRAME *frame);

//智能视频分析接收用户指令(xml接口)，见用户指令码，返回值见错误码
BVS_DLL_API int32_t iva_alg_receive_user_command_in_xml_format(IVA_HANDLE handle, uint8_t user_command, char *user_info_xml, 
                                                                uint32_t info_length, uint32_t address);

//智能视频分析获取用户信息(xml接口)，返回值见错误码
BVS_DLL_API int32_t iva_alg_get_all_user_information_in_xml_format(IVA_HANDLE handle, char *user_info_xml);

//智能视频分析获取用户注册人脸图像(yuv420格式，100x100),返回值见错误码
BVS_DLL_API int32_t iva_alg_get_user_face_image(IVA_HANDLE handle, int8_t *user_name, uint8_t *face_image);

//智能视频分析写报警配置信息(xml接口)，返回值见错误码
BVS_DLL_API int32_t iva_alg_write_aux_config_file_in_xml_format(IVA_HANDLE handle, char *aux_config_param_xml, int8_t aux_config_filename[]);

//智能视频分析读报警配置信息(xml接口)，返回值见错误码
BVS_DLL_API int32_t iva_alg_read_aux_config_file_in_xml_format(IVA_HANDLE handle, char *aux_config_param_xml, int8_t aux_config_filename[]);

//智能视频分析读共享辅助配置信息(xml接口)，返回值见错误码
BVS_DLL_API int32_t iva_alg_read_shared_aux_config_file_in_xml_format(IVA_HANDLE handle, char *aux_config_param_xml, int8_t aux_config_filename[]);

//智能视频分析写报警计划
BVS_DLL_API int32_t iva_alg_write_alarm_schedual_file_in_xml_format(IVA_HANDLE handle, char *aux_config_param_xml);

//智能视频分析读报警计划
BVS_DLL_API int32_t iva_alg_read_alarm_schedual_in_xml_format(IVA_HANDLE handle, char *alarm_schedual_xml);

//智能视频分析写工作计划
BVS_DLL_API int32_t iva_alg_write_working_schedual_file_in_xml_format(IVA_HANDLE handle, char *working_schedual_xml);

//智能视频分析读工作计划
BVS_DLL_API int32_t iva_alg_read_working_schedual_in_xml_format(IVA_HANDLE handle, char *working_schedual_xml);

//智能视频分析设置时区
BVS_DLL_API int32_t iva_alg_set_time_zone(IVA_HANDLE handle, int32_t time_zone);

//销毁智能视频分析算法
BVS_DLL_API void iva_alg_destroy(IVA_HANDLE handle);

#ifdef __cplusplus
};
#endif

#endif //_IVA_INTERFACE_H_
