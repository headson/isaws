#ifndef HTTPSENDER_SERVER_SENDERSERVER_H_
#define HTTPSENDER_SERVER_SENDERSERVER_H_

#include "httpsender/dataparse/dataparse.h"
#include "kvdb/client/kvdbclient.h"
#include "vznetdp/netdp/dpclient.h"
#include "httpsender/curl/curlservice.h"
#include "httpsender/watchdog/watchdog.h"
#include "ipc_comm/EventServerComm.h"
#ifndef WIN32
#include "sys_env_type.h"
#include "file_msg_drv.h"
#endif

#define MSG_OUT stdout

namespace hs {

//typedef boost::shared_ptr<cached::CachedClient> CachedClientPtr;

typedef boost::shared_ptr<boost::asio::io_service> io_service_ptr;

#define IPC_DISP_FAILED     -1
#define IPC_DISP_SUCCESS    0
#define KVDB_FALSE          -1
#define KVDB_TRUE           0

typedef int IPC_DISP_RET;

class SenderServer : public boost::noncopyable,
  public boost::enable_shared_from_this<SenderServer> {
 public:

  typedef boost::shared_ptr<SenderServer> Ptr;

  SenderServer(io_service_ptr io_service);

  ~SenderServer(void);
  //非阻塞地启动消息分发
  bool Start();

  typedef boost::function<
  void(hs::HttpConn::Ptr http_conn,
       const char *data,
       std::size_t data_size,
       const boost::system::error_code& err,
       hs::CurlServices::Ptr curl_services)> CallbackNewConnMessage;

  //停止消息分发
  void Stop(void);
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
  void OnDpConnectSucceed(vznetdp::DpClient::Ptr client);
  void OnDpErrorEvent(vznetdp::DpClient::Ptr client,
                      const boost::system::error_code& err);
  void OnDpMessage(vznetdp::DpClient::Ptr client, const DpMessage* dmp);
  static void iOnReply(const DpMessage *pDmp, void *pUserData);
  static void iOnGetIpMsg(const DpMessage *pDmp, void *pUserData);

  void OnGetIpMsg(vznetdp::DpClient::Ptr client,
                  const DpMessage* dmp);
#ifdef WIN32
  void ResendIVSResult(hs::HttpConn::Ptr http_conn,
                       boost::shared_ptr<boost::asio::deadline_timer> t,
                       const boost::system::error_code& err);
#else
  void ResendIVSResult(hs::HttpConn::Ptr http_conn,
                       boost::shared_ptr<boost::asio::steady_timer> t,
                       const boost::system::error_code& err);
#endif

  void HttpConnIvsResultComplete(hs::HttpConn::Ptr http_conn,
                                 const char *data,
                                 std::size_t data_size,
                                 const boost::system::error_code& err,
                                 hs::CurlServices::Ptr curl_services);

  void HttpConnGioTriggerComplete(hs::HttpConn::Ptr http_conn,
                                  const char *data,
                                  std::size_t data_size,
                                  const boost::system::error_code& err,
                                  hs::CurlServices::Ptr curl_services);

  void HttpConnSerialDataComplete(hs::HttpConn::Ptr http_conn,
                                  const char *data,
                                  std::size_t data_size,
                                  const boost::system::error_code& err,
                                  hs::CurlServices::Ptr curl_services);

  void HttpConnDeviceRegComplete(hs::HttpConn::Ptr http_conn,
                                 const char *data,
                                 std::size_t data_size,
                                 const boost::system::error_code& err,
                                 hs::CurlServices::Ptr curl_services);
  void HttpConnSnapImageComplete(hs::HttpConn::Ptr http_conn,
                                 const char *data,
                                 std::size_t data_size,
                                 const boost::system::error_code& err,
                                 hs::CurlServices::Ptr curl_services);

  void FtpConnPostFileComplete(hs::HttpConn::Ptr http_conn,
                               const char *data,
                               std::size_t data_size,
                               const boost::system::error_code& err,
                               hs::CurlServices::Ptr curl_services);

  bool OnIvsPlateResponseHandle(const char* data);

  void OnSerialResponseHandle(const char* data);

  void OnSerialMessage(int serialchannel, std::string data, int datalen);

  static void OnTTRS485(const DpMessage* dmp, void* user_data);

  static void iOnDPMsg(const DpMessage* dmp, void* user_data);

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
  void InitWatchDog();
  void iOnWatchDogTimerCB(const boost::system::error_code& err);
  void GetSendUrl(char *hostname, char *uri);
  void NewIPCConn(Json::Value &value,
                  int resendtimes,
                  int timeout,
                  CallbackNewConnMessage call_back);

  void NewIPCConnExt(HTTP_MSGTYPE msgtype,
                     Json::Value &value,
                     std::string data,
                     std::string uri,
                     CallbackNewConnMessage call_back);

  void HttpPostData(const std::string url,
                    const std::string postdata,
                    bool ssl_enable,
                    int port,
                    int timeout,
                    int resendtimes,
                    CallbackNewConnMessage call_back);
  void DeviceRegFunc(const boost::system::error_code& err);

  void NewIPCDevRegConn(Json::Value devicereg);

  void NewIPCFtpPostConn(char *filepath);

 private:
  void OnIvsResult(vznetdp::DpClient::Ptr client,
                   const DpMessage* dmp);
  void OnSerial(vznetdp::DpClient::Ptr client,
                const DpMessage* dmp);
  void OnGioTrigger(vznetdp::DpClient::Ptr client,
                    const DpMessage* dmp);
  void OnConfigControl(vznetdp::DpClient::Ptr client,
                       const DpMessage* dmp);
  void OnLoggingStatus(vznetdp::DpClient::Ptr client,
                       const DpMessage* dmp);
  void OnIpChange(vznetdp::DpClient::Ptr client,
                  const DpMessage* dmp);
  void OnDeviceReg(vznetdp::DpClient::Ptr client,
                   const DpMessage* dmp);
  void OnGetSnapShotImage(vznetdp::DpClient::Ptr client,
                          const DpMessage* dmp,
                          std::string snap_image_url,
                          int port,bool &dp_end);
  //
  void ConstInit();

  void InitSysJsonDefault();

  void GetHttpPostDeviceInfo();
 private:
  void OnPostEvent(hs::HttpConn::Ptr http_conn,
                   const char *data,
                   std::size_t data_size,
                   const boost::system::error_code& err);

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
  int           m_nReqMsgStatus;        //请求消息的状态
  char          m_strLastReqMsg[1024]; //记录最后一次请求的消息，用于匹配回调中的消息
  void          *m_pBuf4Reply;         //仅指向外界发送请求命令时给的反馈缓冲区地址；
  int           m_nSizeBuf4Reply;
  int           m_nSizeData4Reply;
  int			m_nOfflineCheckStatus;
  int			m_LoggingEnable;
  bool			m_HttpStatusLine;
  int			m_HttpResendTimes;
  int			m_HttpServerPoll;
  bool			m_HttpServerPollLineStatus;
  char			m_httpport[MAX_DEVICE_IP_LEN];
  char          m_ipext[MAX_IP_EXT_LEN];
  char			m_serialdata[MAX_SERIALDATA_LEN];
  char			m_serialbuffer[MAX_SERIALDATA_LEN];
  char			m_ftpbuffer[MAX_FTP_EXT_LEN];
  char			m_emailbuffer[MAX_EMAIL_EXT_LEN];
  char			m_url[128];
  int			m_port;
  TRIGGER_REULST trigger_result_;

  Json::Value ip_extjs_;

  Json::Value       system_json_;
  Json::Value       ftp_json_;
  Json::Value       email_json_;

#ifndef WIN32
  SysInfo					  *system_info_;
#endif
  VZ_CenterServer_Net         server_settings_;
  VZ_CenterServer_DeviceReg   device_reg_settings_;
  VZ_CenterServer_PlateResult plateresult_settings_;
  VZ_CenterServer_GioInAlarm  gpio_settings_;
  VZ_CenterServer_Serial      serial_settings_;
  TCP_SRV_OFFLINE_CHECK_INFO check_info_;

  std::string device_title_;
  std::string device_user_;
  std::string device_password_;

  static const short COMET_CONNTIME = 30;
  static const short WATCHDOG_TIME = 4;
  io_service_ptr io_service_;
  CurlServices::Ptr curl_services_;
  DataParse::Ptr data_parse_;
  WatchDog::Ptr watch_dog_;
  vznetdp::DpClient::Ptr dp_client_;
  kvdb::KvdbClient::Ptr kvdb_client_;
  cached::CachedClient  file_cached_;
#ifdef WIN32
  boost::asio::deadline_timer timer_;
  boost::asio::deadline_timer resend_timer_;
  boost::asio::deadline_timer watchdog_timer_;
#else
  boost::asio::steady_timer timer_;
  boost::asio::steady_timer resend_timer_;
  boost::asio::steady_timer watchdog_timer_;
#endif
};
};

#endif  // HTTPSENDER_SERVER_SENDERSERVER_H_