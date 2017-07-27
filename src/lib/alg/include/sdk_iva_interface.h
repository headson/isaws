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

//����ֵ
#define IVA_ERROR_NO_ERROR						1
#define IVA_ERROR_NULL_HANDLE					2
#define IVA_ERROR_NULL_POINTER					3
#define IVA_ERROR_INVALID_COMMAND				4
#define IVA_ERROR_FRAME_BUFFER_IS_FULL			5
#define IVA_ERROR_INVALID_IMAGE_SIZE			6

//���
typedef void* iva_count_handle;

//����ָ��
typedef enum
{
	CONTROL_COMMAND_RESET_COUNTER = 0,		//������λ
}iva_control_command;

//�¼�����
typedef enum
{
	IVA_EVENT_COUTING = 0,					//����
}iva_event_type;

//����֡��ʽ
typedef struct
{
	//����
	unsigned char *data;
	//���ݳ���
	int data_size_in_bytes;
	//����
	int param[32];
}iva_frame_t;

//������Ϣ
typedef struct
{
	//����ͼ����
	int debug_image_width;
	//����ͼ��߶�
	int debug_image_height;
	//����ͼ�񻺴�
	unsigned char *debug_image;
	//������Ϣ
	unsigned char debug_msg[256];
}sdk_iva_debug_info;

//�����Ϣ
typedef struct
{
	iva_event_type event_type;	//�¼�����
	int positive_number;		//�������ֵ
	int negative_number;		//�������ֵ
}sdk_iva_output_info;

//���Իص�����
typedef void(*sdk_iva_debug_callback_fun)(sdk_iva_debug_info *info);

//����ص�����
typedef void(*sdk_iva_output_callback_fun)(sdk_iva_output_info *info);

//������Ϣ
typedef struct
{
	//ͼ����
	int image_width;
	//ͼ��߶�
	int image_height;
	//���Իص�����
	sdk_iva_debug_callback_fun debug_callback_fun;
	//����ص�����
	sdk_iva_output_callback_fun output_callback_fun;
	//�����ļ���
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