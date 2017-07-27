#ifndef _SDK_IVA_INTERFACE_H_
#define _SDK_IVA_INTERFACE_H_

#ifdef __cplusplus
extern "C"
{
#endif

#ifdef WIN32
#ifdef ALG_DLL_API
#else
#define ALG_DLL_API //_declspec(dllimport)
#endif
#else
#define ALG_DLL_API
#endif

//返回值
#define IVA_ERROR_NO_ERROR						1
#define IVA_ERROR_NULL_HANDLE					2
#define IVA_ERROR_NULL_POINTER					3
#define IVA_ERROR_INVALID_COMMAND				4
#define IVA_ERROR_FRAME_BUFFER_IS_FULL			5
#define IVA_ERROR_INVALID_IMAGE_SIZE			6

//句柄
typedef void* iva_count_handle;

//控制指令
typedef enum
{
	CONTROL_COMMAND_RESET_COUNTER = 0,		//计数复位
}iva_control_command;

//事件类型
typedef enum
{
	IVA_EVENT_COUTING = 0,					//计数
}iva_event_type;

//数据帧格式
typedef struct
{
	//数据
	unsigned char *data;
	//数据长度
	int data_size_in_bytes;
	//参数
	int param[32];
}iva_frame_t;

//调试信息
typedef struct
{
	//调试图像宽度
	int debug_image_width;
	//调试图像高度
	int debug_image_height;
	//调试图像缓存
	unsigned char *debug_image;
	//调试信息
	unsigned char debug_msg[256];
}sdk_iva_debug_info;

//输出信息
typedef struct
{
	iva_event_type event_type;	//事件类型
	int positive_number;		//正向计数值
	int negative_number;		//反向计数值
}sdk_iva_output_info;

//调试回调函数
typedef void(*sdk_iva_debug_callback_fun)(sdk_iva_debug_info *info);

//输出回调函数
typedef void(*sdk_iva_output_callback_fun)(sdk_iva_output_info *info);

//创建信息
typedef struct
{
	//图像宽度
	int image_width;
	//图像高度
	int image_height;
	//调试回调函数
	sdk_iva_debug_callback_fun debug_callback_fun;
	//输出回调函数
	sdk_iva_output_callback_fun output_callback_fun;
	//配置文件名
	char config_filename[128];
}sdk_iva_create_param;

ALG_DLL_API int sdk_iva_create(iva_count_handle *handle, sdk_iva_create_param *create_param);

ALG_DLL_API int sdk_iva_destroy(iva_count_handle handle);

ALG_DLL_API int sdk_iva_process(iva_count_handle handle, iva_frame_t *frame);

ALG_DLL_API int sdk_iva_set_control_command(iva_count_handle handle, iva_control_command command);

#ifdef __cplusplus
};
#endif

#endif