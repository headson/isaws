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
  STATE_OK = 200,//请求的命令执行成功
  STATE_ERR_INVALID_VALUE =400,//客户端请求出错（因为错误的语法导致服务器无法理解请求信息
  STATE_ERR_NO_AUTHORITY=401,//客户没有权限，请求需要用户验证
  STATE_ERR_INVALID_URL=404,//请求的资源不存在
  STATE_ERR_INVALID_REQUEST=405,//请求的命令不存在
  STATE_ERR_TIME_OUT=408,//请求超时
  STATE_ERR_SERVER_ERR=500,//服务器内部错误
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


