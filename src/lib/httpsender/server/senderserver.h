#ifndef HTTPSENDER_SERVER_SENDERSERVER_H_
#define HTTPSENDER_SERVER_SENDERSERVER_H_

#include "vzbase/base/basictypes.h"

#include "httpsender/curl/curlservices.h"
#include "dispatcher/sync/dpclient_c.h"

#include "httpsender/dataparse/dataparse.h"

#include "json/json.h"
#include "sys_env_type.h"

#ifndef _WIN32
#include "file_msg_drv.h"
#endif

#define MSG_OUT stdout

namespace hs {

#define IPC_DISP_FAILED     -1
#define IPC_DISP_SUCCESS    0
#define KVDB_FALSE          -1
#define KVDB_TRUE           0

typedef int IPC_DISP_RET;

class SenderServer : public PostInterface {
 public:
  SenderServer();
  virtual ~SenderServer(void);

  bool Start();
  void Stop();

  bool RunLoop();

  //////////////////////////////////////////////////////////////////////////
  int SetIpExt(const char *pdata, int datalen);
  void* GetIpExt() {
    return m_ipext;
  }
  int SetCenterServerNet(const char* net);
  void* GetCenterServerNet() {
    return &server_settings_;
  }
  int SetCenterServerDeviceReg(const char* devicereg);
  void* GetCenterServerDeviceReg() {
    return &device_reg_settings_;
  }
  int SetCenterServerPlateResult(const char* plateresult);
  void* GetCenterServerPlateResult() {
    return &plateresult_settings_;
  }
  int SetCenterServerGio(const char* gio);
  void* GetCenterServerGio() {
    return &gpio_settings_;
  }
  int SetCenterServerSerial(const char* serial);
  void* GetCenterServerSerial() {
    return &serial_settings_;
  }
  int SetHttpOfflineCheckStatus(const char* status);
  void* GetHttpOfflineCheckStatus() {
    return &m_nOfflineCheckStatus;
  }
  int GetHttpOfflineCheckStatusInt() {
    return m_nOfflineCheckStatus;
  }
  int SetHttpResendTimes(const char* status);
  void* GetHttpResendTimes() {
    return &m_HttpResendTimes;
  }
  int GetHttpResendTimesInt() {
    return m_HttpResendTimes;
  }
  int SetHttpServerPoll(const char* poll);
  void* GetHttpServerPoll() {
    return &m_HttpServerPoll;
  }
  int GetHttpServerPollInt() {
    return m_HttpServerPoll;
  }

 private:
  static void dp_cli_msg_cb(DPPollHandle p_hdl, const DpMessage *p_dpm, void* p_usr_arg);
  void OnDpMessage(DPPollHandle p_hdl, const DpMessage* dmp);

  static void dp_cli_state_cb(DPPollHandle p_hdl, uint32 n_state, void* p_usr_arg);
  void OnDpState(DPPollHandle p_hdl, uint32 n_state);

  static void kvdb_get_key_cb(const char *p_key,
                              int         n_key,
                              const char *p_value,
                              int         n_value,
                              void       *p_user_data);

  static int32 dev_reg_timer_cb(SOCKET          fd,
                                short           events,
                                const void      *p_usr_arg);
  void OnDevRegTimer();

  /************************************************************************
  *Description : 接收完成回调,n_result=0发送成功,n_result!=0接收失败,
  *              需要用户在回调中释放HttpConn
  *Parameters  :
  *Return      :
  ************************************************************************/
  virtual void PostCallBack(HttpConn *cfg, int errcode);

  void HttpConnIvsResultComplete(hs::HttpConn *p_conn,
                                 const char *p_resp, std::size_t n_resp,
                                 int errcode);

  void HttpConnGioTriggerComplete(hs::HttpConn* p_conn,
                                  const char   *data,
                                  std::size_t   data_size,
                                  int errcode);

  void HttpConnSerialDataComplete(hs::HttpConn *p_conn,
                                  const char   *data,
                                  std::size_t   data_size,
                                  int errcode);

  void HttpConnDeviceRegComplete(hs::HttpConn *http_conn,
                                 const char *data,
                                 std::size_t data_size,
                                 int errcode);
  void HttpConnSnapImageComplete(hs::HttpConn *p_conn,
                                 const char   *data,
                                 std::size_t   data_size,
                                 int errcode);

  void FtpConnPostFileComplete(hs::HttpConn *p_conn,
                               const char   *data,
                               std::size_t   data_size,
                               int errcode);

  bool OnIvsPlateResponseHandle(const char* data);

  void OnSerialResponseHandle(const char* data);

  void OnSerialMessage(int serialchannel, std::string data, int datalen);

  bool iSetLastReqMsg(const char *pMsg, void *pBuf4Reply, int nSizeBuf4Reply);

  void iCleanLastReqMsg();
  bool iCheckAndHandleReqMsg();
  bool iGetKeyValueFunc(const char *pkey,
                        int keylen,
                        char *buffer,
                        int bufferlen,
                        int datatype);
  int iReadJsonFile(const char* file_path, Json::Value &value);
  size_t strlcpy(char *dst, const char *src, size_t siz);
  void StringToChar(const std::string &res_str, char *res, int len);
  void InitHttpDataInfo();
  void InitSysInfo();

  void GetSendUrl(char *hostname, char *uri);

  //////////////////////////////////////////////////////////////////////////
  void NewIPCConn(Json::Value &value,
                  int          resendtimes,
                  int          timeout,
                  int          n_cb_type);
  void NewIPCConnExt(HTTP_MSGTYPE   msgtype,
                     Json::Value   &value,
                     std::string    data,
                     std::string    uri,
                     int            n_cb_type);

  void HttpPostData(const std::string url,
                    const std::string postdata,
                    bool              ssl_enable,
                    int               port,
                    int               timeout,
                    int               resendtimes,
                    int               n_cb_type);

  void DeviceRegFunc();

  void NewIPCFtpPostConn(char *filepath);

 private:
  //////////////////////////////////////////////////////////////////////////
  void OnIvsResult(const DpMessage* dmp);
  void OnSerial(const DpMessage* dmp);
  void OnGioTrigger(const DpMessage* dmp);
  void OnConfigControl(const DpMessage* dmp);

  void OnLoggingStatus(const DpMessage* dmp);
  void OnIpChange(const DpMessage* dmp);

  void OnGetSnapShotImage(const DpMessage* dmp,
                          std::string snap_image_url,
                          int port,
                          bool &dp_end);

  void InitSysJsonDefault();

  void GetHttpPostDeviceInfo();

 private:
  int WriteDpLoggingMessage(unsigned char proc_id,
                            unsigned short type,
                            unsigned char msg_size,
                            const char *logging_msg,
                            unsigned int event_id);

  void HttpDpLogging(const char *logging_msg,
                     unsigned char msg_size);

  void HttpWriteResponseLogging(std::string url, const char* data);

  void HttpWriteRequestLogging(HTTP_MSGTYPE msgtype,
                               std::string data);

  void SetReplyJsonStatus(Json::Value &replyjson,
                          std::string type,
                          int status,
                          std::string errmsg,
                          std::string body = DEFAULT_BODY_DATA);

  int HttpSenderReqFunc(const char* reqdata,
                        int size,
                        std::string &replydata);

  bool SetFtpConfig(Json::Value &json,Json::Value &repjson);

  bool GetFtpConfig(Json::Value &json);

  bool SetEmailConfig(Json::Value &json,Json::Value &repjson);

  bool GetEmailConfig(Json::Value &json);

  bool SendFtpTestFile(Json::Value &json);

  std::string GetSystemInfoAccount0User();
  std::string GetSystemInfoAccount0Password();
  std::string GetSystemInfoTitle();
  std::string GetSystemInfoDeviceIp();

 private:
  DpMessage                 *p_cur_dp_msg_;           // 慎用此指针,只有阻塞使用

 private:
  int                         m_nReqMsgStatus;        //请求消息的状态
  char                        m_strLastReqMsg[1024];  //记录最后一次请求的消息，用于匹配回调中的消息
  void                       *m_pBuf4Reply;           //仅指向外界发送请求命令时给的反馈缓冲区地址；
  int                         m_nSizeBuf4Reply;
  int                         m_nSizeData4Reply;
  int                         m_nOfflineCheckStatus;
  int                         m_LoggingEnable;
  bool                        m_HttpStatusLine;
  int                         m_HttpResendTimes;
  int                         m_HttpServerPoll;
  bool                        m_HttpServerPollLineStatus;
  char                        m_httpport[MAX_DEVICE_IP_LEN];
  char                        m_ipext[MAX_IP_EXT_LEN];
  char                        m_serialdata[MAX_SERIALDATA_LEN];
  char                        m_serialbuffer[MAX_SERIALDATA_LEN];
  char                        m_ftpbuffer[MAX_FTP_EXT_LEN];
  char                        m_emailbuffer[MAX_EMAIL_EXT_LEN];
  char                        m_url[128];
  int                         m_port;
  TRIGGER_REULST              trigger_result_;

  Json::Value                 ip_extjs_;

  Json::Value                 system_json_;
  Json::Value                 ftp_json_;
  Json::Value                 email_json_;

#ifndef WIN32
  SysInfo                    *system_info_;
#endif
  VZ_CenterServer_Net         server_settings_;
  VZ_CenterServer_DeviceReg   device_reg_settings_;
  VZ_CenterServer_PlateResult plateresult_settings_;
  VZ_CenterServer_GioInAlarm  gpio_settings_;
  VZ_CenterServer_Serial      serial_settings_;
  TCP_SRV_OFFLINE_CHECK_INFO  check_info_;

  std::string                 device_title_;
  std::string                 device_user_;
  std::string                 device_password_;

  static const short          COMET_CONNTIME = 30;
  static const short          WATCHDOG_TIME = 4;

  DPPollHandle                dp_client_;
  DataParse                   data_parse_;
  CurlServices               *curl_services_;

  vzconn::EVT_TIMER           c_deg_reg_timer_;

  void*                       watchdog_handle_;
};
};

#endif  // HTTPSENDER_SERVER_SENDERSERVER_H_