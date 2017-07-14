#ifndef SRC_LIB_ONVIF_BASE_BASEDEFINE_H_
#define SRC_LIB_ONVIF_BASE_BASEDEFINE_H_

namespace onvifserver {
  static const char * HW_INFO = "sys_hwinfo";
  static const char * HW_WRITE_STATUS = "write_status";
  static const char * HW_MAC = "mac";
  static const char * HW_SERIALNO = "serialno";
  static const char * HW_DEV_TYPE = "device_type";
  static const char * HW_EXDATA_CHECKER = "exdata_checker";
  static const char * HW_OEM_INFO = "oem_info";
  static const char * HW_VERSION = "hw_version";
  static const char * HW_FLAG = "hw_flag";
  static const char * HW_EXDATA_SIZE = "exdataSize";
  static const char * HW_BOARD_VERSION = "board_version";

typedef enum 
{
  STATE_OK = 200,//���������ִ�гɹ�
  STATE_ERR_INVALID_VALUE =400,//�ͻ������������Ϊ������﷨���·������޷����������Ϣ
  STATE_ERR_NO_AUTHORITY=401,//�ͻ�û��Ȩ�ޣ�������Ҫ�û���֤
  STATE_ERR_INVALID_URL=404,//�������Դ������
  STATE_ERR_INVALID_REQUEST=405,//������������
  STATE_ERR_TIME_OUT=408,//����ʱ
  STATE_ERR_SERVER_ERR=500,//�������ڲ�����
}
ONVIF_STATE_RET;

static const char *  TYPE = "type";
static const char *  ONVIF_ERROR_MSG = "error_msg";
static const char *  ONVIF_STATE_CODE = "state";
static const char *  ONVIF_BODY = "body";
static const char ONVIF_OK_STR[] = "success";
static const char ONVIF_FAIL_STR[] = "service inner error";
static const char ONVIF_UnsupportedCmd[]	=	"{\n"
																			"\"response\":\"unsupported cmd, supported cmd including: 'set_adminpass'\"\n"
																			"}\n";
}  // namespace onvifserver


#endif  // SRC_LIB_ONVIF_BASE_BASEDEFINE_H_


