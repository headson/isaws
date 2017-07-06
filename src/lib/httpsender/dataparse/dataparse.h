#ifndef HTTPSENDER_DATAPARSE_DATAPARSE_H_
#define HTTPSENDER_DATAPARSE_DATAPARSE_H_

#include "json/json.h"
#ifndef WIN32
#include "vz_hwi_sharemem.h"
#endif
#include "vzbase/base/base64.h"

#include "cacheserver/client/cachedclient_c.h"

#include "VzClientSDK_LPDefine.h"
#include "httpsender/base/basedefines.h"
#include "ipc_comm/AVServerComm.h"
#include "ipc_comm/EventServerComm.h"

namespace hs {

class DataParse{
 public:
  DataParse();
  ~DataParse();
  int ReadJsonFile(
    const char* file_path,
    Json::Value &value);

  void GetSnapImage(const char  *path,
                    Json::Value &snapimage_js);
  bool JsonReadSysInfoValue(Json::Value &sysInfoJson);

  void GetPlateJson(
    IVS_RESULT_PARAM *result,
    Json::Value& platejson,
    VZ_CenterServer_PlateResult *plateresult,
    char* devicename);

  void GetDeviceRegJson(
    Json::Value& deviceregjson,
    char* devicename,
    char* username,
    char* passwd,
    char* httpport);
  int Code_Convert(char *from_charset, char *to_charset,
                   char *in_buf, int in_len, char *out_buf, int out_len);
  int UTF8_To_GB2312(char *in_buf, int in_len, char *out_buf,
                     int out_len);
  void GetFtpPostJson(
    UserGetImgInfo &imginfo,
    Json::Value& ftppostjson,
    char* ftpip,
    char* userfilepath,
    char* postfilepath,
    std::string title);

  void GetRs485Json(
    Json::Value& rs485json,
    int serialchannel,
    const char* serialdata,
    int datalen);

  void GetGIOJson(
    Json::Value& giojson,
    TRIGGER_REULST* result,
    char* devicename);

  void SetDeviceIp(char* deviceip);

  void iInitHwiInfo(Json::Value hw_info);

 private:

  //static char* gb2312_to_utf8(char* gb2312);
  static int h2i(char* s);
  //static char* base64_encode(const unsigned char *value, int vlen);
  static int url_decode(char *str, int len);
  static char* url_encode(
    char const *s,
    int len,
    int *new_length);

  static void http_dp_logging(
    bool loggingenable,
    const char *logging_msg,
    unsigned char msg_size);

  static void OnCachedGetFileCallback(
    const char *path,
    int path_size,
    const char *data,
    int data_size,
    void* user_data);

 private:
  bool cachedGetFileCallbackIndex;
  char device_ip_[32];
  char serial_no_[32];
};

};

#endif  // HTTPSENDER_DATAPARSE_DATAPARSE_H_