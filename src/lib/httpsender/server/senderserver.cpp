#include "httpsender/server/senderserver.h"
#include "vzbase/helper/stdafx.h"
#include "ipc_comm/SystemServerComm.h"
#include "ipc_comm/dp_logging_comm.h"
#include "ipc_comm/HttpSenderComm.h"
#include "vzbase/base/base64.h"
#include "vzbase/thread/thread.h"

#include "stdlib.h"
#include "stdio.h"
#include "string.h"

#include <string>
#include <fstream>
#include <map>

#ifdef WIN32
#include <windows.h>
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#endif

namespace hs {

const unsigned REQ_TIMEOUT = 3 ;     // 请求超时时间（秒）
const unsigned REQ_WAIT_GAP = 10 ;   // 请求后等待时的查询间隔（毫秒）

enum {
  REQ_MSG_ERR = -2,
  REQ_MSG_TIMEOUT = -1,
  REQ_MSG_INIT = 0,
  REQ_MSG_WAIT,
  REQ_MSG_OK,
};

enum {
  HTTP_CB_TYPE_IVS_RESULT       = 100,
  HTTP_CB_TYPE_SERIAL           = 101,
  HTTP_CB_TYPE_GIO_TRIGGER      = 102,
  HTTP_CB_TYPE_CONFIG_CONTROL   = 103,
  HTTP_CB_TYPE_LOGGING_STATUS   = 104,
  HTTP_CB_TYPE_IP_CHANGE        = 105,
  HTTP_CB_TYPE_DEVICE_REG       = 106,
  HTTP_CB_TYPE_SNAPSHOT_IMAGE   = 107,
  HTTP_CB_TYPE_SERIAL_DATA      = 108,
  HTTP_CB_TYPE_FTP_POST_CONN    = 109,
};

#define TRUE 1
#define FALSE 0

SenderServer::SenderServer()
  : m_nReqMsgStatus(REQ_MSG_INIT)
  , m_pBuf4Reply(NULL)
  , m_nSizeBuf4Reply(0)
  , m_nSizeData4Reply(0)
  , curl_services_(NULL)
  , data_parse_() 
  , watchdog_handle_(NULL) {
  m_strLastReqMsg[0] = '\0';
  memcpy(server_settings_.hostname, DEFAULT_HOST_NAME,
         sizeof(DEFAULT_HOST_NAME));
  server_settings_.port = DEFAULT_NET_HTTP_PORT;
  server_settings_.enable_ssl = 0;
  server_settings_.ssl_port = 443;
  server_settings_.http_timeout = 5;
  //
  device_reg_settings_.enable = 1;
  memcpy(device_reg_settings_.uri, DEFAULT_REG_URI, sizeof(DEFAULT_REG_URI));
  //
  plateresult_settings_.enable = 0;
  memcpy(plateresult_settings_.uri,
         DEFAULT_PLATE_POST_URI,
         sizeof(DEFAULT_PLATE_POST_URI));
  plateresult_settings_.plateResultLevel = 0;
  plateresult_settings_.is_send_image = 0;
  plateresult_settings_.is_send_small_image = 0;
  //
  gpio_settings_.enable = 0;
  memcpy(gpio_settings_.uri,
         DEFAULT_GPIO_POST_URL,
         sizeof(DEFAULT_GPIO_POST_URL));
  //
  serial_settings_.enable = 0;
  memcpy(serial_settings_.uri,
         DEFAULT_SERIAL_POST_URL,
         sizeof(DEFAULT_SERIAL_POST_URL));
  //
  m_ipext[0] = 0;
  m_url[0] = 0;
  //
  m_nOfflineCheckStatus = 0;
  m_HttpStatusLine = true;
  m_HttpResendTimes = 0;
  m_HttpServerPoll = 0;
  m_HttpServerPollLineStatus = true;
  m_LoggingEnable = 0;
  //
  check_info_.bActive = 1;
  check_info_.nTimeout = 5;

  trigger_result_.result = 1;
  trigger_result_.source = TRIGGER_TCP;

  device_title_ = DEFAULT_DEVICE_TITLE;
  device_user_ = DEFAULT_DEVICE_USER;
  device_password_ = DEFAULT_DEVICE_PASSWORD;
}

SenderServer::~SenderServer(void) {
  Stop();
}

void SenderServer::InitSysJsonDefault() {
  Json::Value ftp_json;
  Json::Value email_json;

  ftp_json[B_ENABLE_FTPUPDATE] = DEFAULT_FTP_ENABLE;
  ftp_json[SERVIER_IP] = DEFAULT_FTP_SERVER_IP;
  ftp_json[USER_NAME] = DEFAULT_FTP_USER_NAME;
  ftp_json[PASSWORD] = DEFAULT_FTP_PASSWORD;
  ftp_json[FOLDER_NAME] = DEFAULT_FTP_FOLDER_NAME;
  ftp_json[IMAGE_ACOUNT] = DEFAULT_FTP_IMAGE_ACOUNT;
  ftp_json[P_ID] = DEFAULT_FTP_P_ID;
  ftp_json[PORT_STR] = DEFAULT_FTP_PORT_STR;

  email_json[B_ENABLE_MAILUPDATE] = DEFAULT_EMAIL_ENABLE;
  email_json[SERVIER_IP] = DEFAULT_EMAIL_SERVER_IP;
  email_json[USER_NAME] = DEFAULT_EMAIL_USER_NAME;
  email_json[PASSWORD] = DEFAULT_EMAIL_PASSWORD;
  email_json[AUTHENTICATION] = DEFAULT_EMAIL_AUTHENTICATION;
  email_json[SENDER_EMAIL] = DEFAULT_EMAIL_SENDER_EMAIL;
  email_json[RECEIVER_EMAIL] = DEFAULT_EMAIL_RECEIVE_EMAIL;
  email_json[CC_STR] = DEFAULT_EMAIL_CC_STR;
  email_json[SUBJECT] = DEFAULT_EMAIL_SUBJECT;
  email_json[TEXT_STR] = DEFAULT_EMAIL_TEXT_STR;
  email_json[ATTACHMENTS] = DEFAULT_EMAIL_ATTACHEMENTS;
  email_json[VIEW_STR] = DEFAULT_EMAIL_VIEW_STR;
  email_json[ASMT_PATTACH] = DEFAULT_EMAIL_ASMT_PATTACH;
  email_json[ATTFILE_FORMAT] = DEFAULT_EMAIL_ATTFILE_FORMAT;

  system_json_[DEVICE][NET][FTP_CONFIG] = ftp_json;
  system_json_[DEVICE][NET][SMTP_CONFIG] = email_json;
  system_json_[DEVICE][NET][HTTP_PORT] = DEFAULT_NET_HTTP_PORT;
}

void SenderServer::InitSysInfo() {
  if(!data_parse_.JsonReadSysInfoValue(system_json_)) {
    //如果失败，则赋默认值
    InitSysJsonDefault();
    LOG(L_ERROR) << "Init System Json Failure";
  } else {
    system_json_[DEVICE][NET][FTP_CONFIG][B_ENABLE_FTPUPDATE] =
      system_json_[ALARM_SHOT][0][B_ENABLE_FTPUPDATE].asInt();
    system_json_[DEVICE][NET][SMTP_CONFIG][B_ENABLE_MAILUPDATE] =
      system_json_[ALARM_SHOT][0][B_ENABLE_MAILUPDATE].asInt();
  }
  ftp_json_ = system_json_[DEVICE][NET][FTP_CONFIG];
  email_json_ = system_json_[DEVICE][NET][SMTP_CONFIG];
  std::string ftp_str = ftp_json_.toStyledString();
  std::string email_str = email_json_.toStyledString();

  memcpy(m_ftpbuffer,ftp_str.c_str(),ftp_str.length());
  memcpy(m_emailbuffer,email_str.c_str(),ftp_str.length());
  sprintf(m_httpport, "%d", system_json_[DEVICE][NET][HTTP_PORT].asInt());
#ifndef WIN32
  //if (InitFileMsgDrv(FILE_MSG_KEY, FILE_HTTPSENDER_MSG) != 0) {
  //  LOG(L_ERROR) << "Init System Info Failure";
  //  return;
  //}
  //system_info_ = GetSysInfo();
  //sprintf(m_httpport, "%d", system_info_->device.net.http_port);
#endif
}

static const char *MSG_METHOD_SET[] = {
  EVT_SRV_IVS_RESULT,
  EVT_SRV_TT_TCP,
  //EVT_SRV_TT,
  EVT_SRV_EXTERNAL_TRIGGER,

  HTTPSENDER_SET_HTTP_GPIO_NUM,
  HTTPSENDER_GET_HTTP_GPIO_NUM,
  HTTPSENDER_SET_HTTP_IP_EXT,
  HTTPSENDER_GET_HTTP_IP_EXT,
  HTTPSENDER_MSG_SET_CENTER_SERVER,
  HTTPSENDER_MSG_GET_CENTER_SERVER,
  HTTPSENDER_MSG_SET_CENTER_SERVER_DEVICE_REG,
  HTTPSENDER_MSG_GET_CENTER_SERVER_DEVICE_REG,
  HTTPSENDER_MSG_SET_CENTER_SERVER_ALARM_PLATE,
  HTTPSENDER_MSG_GET_CENTER_SERVER_ALARM_PLATE,
  HTTPSENDER_MSG_SET_CENTER_SERVER_ALARM_GIOIN,
  HTTPSENDER_MSG_GET_CENTER_SERVER_ALARM_GIOIN,
  HTTPSENDER_MSG_SET_CENTER_SERVER_SERIAL,
  HTTPSENDER_MSG_GET_CENTER_SERVER_SERIAL,
  HTTPSENDER_MSG_SET_OFFLINE_STATUS,
  HTTPSENDER_MSG_GET_OFFLINE_STATUS,
  HTTPSENDER_MSG_SET_REPUSH_NUMS,
  HTTPSENDER_MSG_GET_REPUSH_NUMS,
  HTTPSENDER_MSG_SET_SERVERPOLL_STATUS,
  HTTPSENDER_MSG_GET_SERVERPOLL_STATUS,

  HTTPSENDER_MSG_CONFIG_CONTROL,

  SYS_SRV_IP_CHANGED,
  DP_LOGGING_ENABLE
};
static const uint32 MSG_METHOD_SET_CNT = sizeof(MSG_METHOD_SET)/sizeof(char*);

bool SenderServer::Start(void) {
  LOG(L_INFO) << "InitSysInfo";
  InitSysInfo();
  LOG(L_INFO) << "try";
  bool res = true;
  try {
    // 1 file cached
    Cached_Start(FILECACHED_IP_ADDR, FILECACHED_PORT);

    LOG(L_INFO) << "InitKvdbClient";
    // 2 kvdb_client
    Kvdb_Start(KVDB_SERVER_IP_ADDR, KVDB_SERVER_PORT);
    //SKvdb_Start(KVDB_SERVER_IP_ADDR, KVDB_SERVER_PORT);

    LOG(L_INFO) << "InitHttpDataInfo";
    InitHttpDataInfo();

    // dp client
    DpClient_Init(DP_SERVER_IP_ADDR, DP_SERVER_PORT);
    DpClient_Start(0);
    dp_client_ = DpClient_CreatePollHandle(dp_cli_msg_cb, this,
                                           dp_cli_state_cb, this,
                                           vzbase::Thread::Current()->socketserver()->GetEvtService());
    if (!dp_client_) {
      LOG(L_ERROR) << "Failure to create dp client";
      return false;
    }

    vzconn::EVT_LOOP* p_evt_loop =
      (vzconn::EVT_LOOP*)DpClient_GetEvtLoopFromPoll(dp_client_);
    // curl_services
    curl_services_ = CurlServices::Create(p_evt_loop);
    if (!curl_services_) {
      LOG(L_ERROR) << "Failure to init CurlServices";
      return false;
    }

    c_deg_reg_timer_.Init(p_evt_loop, dev_reg_timer_cb, this);

    LOG(L_INFO) << "End";
  } catch (std::exception &e) {
    LOG(L_ERROR) << e.what();
    return false;
  }
  LOG(L_INFO) << "SenderServer Start";
  return true;
}

void SenderServer::Stop() {
  if (curl_services_) {
    CurlServices::Remove(curl_services_);
    curl_services_ = NULL;
  }
  if (dp_client_) {
    DpClient_ReleasePollHandle(dp_client_);
    dp_client_ = NULL;
  }

#ifndef WIN32
  // CleanupFileMsgDrv();
#endif
}

bool SenderServer::RunLoop() {
  static int times = 0;
  if (watchdog_handle_ == NULL) {
    watchdog_handle_ = RegisterWatchDogKey("MAIN", 4, DEF_WATCHDOG_TIMEOUT);
  }
  if (watchdog_handle_ && ((times++) % 5) == 0) {
    FeedDog(watchdog_handle_);
  }

  int32 n_ret = DpClient_PollDpMessage(dp_client_, DEF_TIMEOUT_MSEC);
  if (n_ret == VZNETDP_SUCCEED) {
    return true;
  }

  return false;
}

//////////////////////////////////////////////////////////////////////////
void SenderServer::HttpConnSnapImageComplete(hs::HttpConn *p_conn,
    const char   *data,
    std::size_t   data_size,
    int errcode) {
  if (p_conn == NULL)
    return;
}

void SenderServer::OnGetSnapShotImage(const DpMessage* dmp,
                                      std::string snap_image_url,
                                      int port,bool &dp_end) {
  if(dmp->type == TYPE_ERROR_TIMEOUT || dmp->data == NULL) {
    LOG(L_ERROR) << "OnGetSnapShotImage TYPE_ERROR_TIMEOUT";
    return ;
  }
  LOG(L_INFO) << snap_image_url;
  char sendtriggerimage[512] = {0};
  sprintf(sendtriggerimage, "send triggerimage to : %s", snap_image_url.c_str());
  HttpDpLogging(sendtriggerimage, strlen(sendtriggerimage));
  //GetSendUrl(server_settings_.hostname, (char *)snap_image_url.c_str());
  int timeout = server_settings_.http_timeout;
  Json::Value snap_image_js;
  std::string imagepath(dmp->data,dmp->data_size);
  data_parse_.GetSnapImage(imagepath.c_str(),snap_image_js);
  std::string postdata = snap_image_js.toStyledString();
  LOG(L_INFO) << "dmp->data : " << dmp->data;
  LOG(L_INFO) << "dmp->data_size : " << dmp->data_size;
  LOG(L_INFO) << "image_size : " << postdata.length();
  HttpConn *p_conn = curl_services_->CreateHttpConn(
                       snap_image_url,
                       postdata,
                       server_settings_.enable_ssl == 1,
                       port,
                       timeout,
                       0,
                       HTTP_CB_TYPE_SNAPSHOT_IMAGE,
                       this);
  if (p_conn == NULL)
    return;

  curl_services_->PostData(p_conn);
}

bool SenderServer::OnIvsPlateResponseHandle(const char* data) {
  LOG(L_INFO) << "OnResponseHandle... ... : " << data;
  if (data == NULL)
    return false;

  Json::Reader reader;
  Json::Value value;
  if (!reader.parse(data, value)) {
    LOG(L_ERROR) << "OnResponseHandle parse error... ...";
    return false;
  }

  if (value[JSON_RESPONSE_ALARMINFOPLATE][JSON_INFO] == "ok") {
    GPIO_WAVE_VALUE gpioValue;
    gpioValue.eValue = IO_SQUARE_HIGH;
    gpioValue.delay = 500;
    int io_num = 0;
    if(!value[JSON_RESPONSE_ALARMINFOPLATE][JSON_IO_CHANNEL_NUM].isNull()) {
      io_num = value[JSON_RESPONSE_ALARMINFOPLATE][JSON_IO_CHANNEL_NUM].asInt();
    }
    gpioValue.gpio = io_num;
    LOG(L_WARNING) << "gpioValue.gpio : " << gpioValue.gpio;
    DpClient_SendDpMessage(TCP_SRV_GPIO_IOCTL, 0,
                           (const char*)&gpioValue,
                           sizeof(GPIO_WAVE_VALUE));
  }
  if(!value[JSON_RESPONSE_ALARMINFOPLATE][JSON_TRIGGERIMAGE].isNull()) {
    Json::Value triggerimage =
      value[JSON_RESPONSE_ALARMINFOPLATE][JSON_TRIGGERIMAGE];
    int trigger_port = m_port;
    if(!triggerimage[JSON_TRIGGERPORT].isNull())
      trigger_port = triggerimage[JSON_TRIGGERPORT].asInt();
    if(!triggerimage[JSON_SNAP_IMAGE_ABSOLUTELY_URL].isNull()) {
      bool dp_end = false;
      std::string url = triggerimage[JSON_SNAP_IMAGE_ABSOLUTELY_URL].asString();

      //dp_client_->SendDpRequest(
      //  AVS_GET_SNAP, 0, NULL, 0,
      //  boost::bind(
      //    &SenderServer::OnGetSnapShotImage,
      //    this, _1, _2,
      //    url,
      //    trigger_port, dp_end),
      //  3);
      DpClient_SendDpRequest(AVS_GET_SNAP, 0, NULL, 0,
                             dp_cli_msg_cb, this, 3000);
      OnGetSnapShotImage(p_cur_dp_msg_, url, trigger_port, dp_end);
    } else if(!triggerimage[JSON_SNAP_IMAGE_RELATIVE_URL].isNull()) {
      bool dp_end = false;
      GetSendUrl(
        server_settings_.hostname,
        (char *)triggerimage[JSON_SNAP_IMAGE_RELATIVE_URL].asString().c_str());

      std::string url(m_url);
      //dp_client_->SendDpRequest(
      //  AVS_GET_SNAP, 0, NULL, 0,
      //  boost::bind(
      //    &SenderServer::OnGetSnapShotImage,
      //    this, _1, _2,
      //    url,trigger_port,dp_end), 3);
      DpClient_SendDpRequest(AVS_GET_SNAP, 0, NULL, 0,
                             dp_cli_msg_cb, this, 3000);
      OnGetSnapShotImage(p_cur_dp_msg_, url, trigger_port, dp_end);
    }
  }
  Json::Value serial_data = value[JSON_RESPONSE_ALARMINFOPLATE][JSON_SERIAL_DATA];
  for (size_t i = 0; i < serial_data.size(); i++) {
    int serialChannel = serial_data[i][JSON_SERIAL_CHANNEL].asInt();
    std::string data = serial_data[i][JSON_DATA].asString();
    int data_len = serial_data[i][JSON_DATA_LEN].asInt();
    std::string decode = vzbase::Base64::Decode(data, vzbase::Base64::DO_STRICT);
    OnSerialMessage(serialChannel, decode, data_len);
  }
  if(!value[JSON_RESPONSE_ALARMINFOPLATE][JSON_MANUAL_TRIGGER].isNull()) {
    if(value[JSON_RESPONSE_ALARMINFOPLATE][JSON_MANUAL_TRIGGER] == "ok") {
      DpClient_SendDpMessage(EVT_SRV_EXTERNAL_TRIGGER, 0,
                             (char *)&trigger_result_,
                             sizeof(TRIGGER_REULST));
    }
  }
  return true;
}

void SenderServer::OnSerialResponseHandle(const char* data) {

  if (data == NULL)
    return;
  Json::Reader reader;
  Json::Value value;
  if (!reader.parse(data, value)) {
    return;
  }
  Json::Value serial_data = value["Response_SerialData"]["serialData"];
  for (size_t i = 0; i<serial_data.size(); i++) {
    int serialChannel = serial_data[i]["serialChannel"].asInt();
    std::string data = serial_data[i]["data"].asString();
    int data_len = serial_data[i]["dataLen"].asInt();

    std::string decode = vzbase::Base64::Decode(data, vzbase::Base64::DO_STRICT);
    OnSerialMessage(serialChannel, decode, data_len);
  }
}

void SenderServer::HttpWriteResponseLogging(std::string url, const char* data) {

  if (url.c_str() == NULL || data == NULL)
    return;
  if (url.length() + strlen(data) >= 480)
    return;
  char rcvmsg[512];
  sprintf(rcvmsg, "response : %s : %s", url.c_str(), data);
  HttpDpLogging(rcvmsg, strlen(rcvmsg));
}

void SenderServer::OnSerialMessage(int serialchannel, std::string data, int datalen) {

  memset(m_serialbuffer, 0, MAX_SERIALDATA_LEN);
  //char* buffer = new char[sizeof(TT_Param) + datalen];
  //if (!buffer) return;
  TT_Param ts_data;
  ts_data.flag = 111;
  ts_data.source = serialchannel;
  memcpy(m_serialbuffer, &ts_data, sizeof(TT_Param));
  memcpy(m_serialbuffer + sizeof(TT_Param), data.c_str(), datalen);

  DpClient_SendDpMessage(TCP_SRV_TT_RS485, 0,
                         m_serialbuffer, sizeof(TT_Param) + datalen);
  //delete[] buffer;
}

int32 SenderServer::dev_reg_timer_cb(SOCKET fd,
                                     short events,
                                     const void *p_usr_arg) {
  if (p_usr_arg) {
    ((SenderServer*)p_usr_arg)->OnDevRegTimer();
  }
  return 0;
}

void SenderServer::OnDevRegTimer() {
  DeviceRegFunc();
}

void SenderServer::PostCallBack(HttpConn *p_conn, int errcode) {
  if (p_conn == NULL) {
    return;
  }

  if (p_conn->request_type_ == HTTP_CB_TYPE_IVS_RESULT) {
    HttpConnIvsResultComplete(p_conn,
                              p_conn->s_resp_data_.c_str(),
                              p_conn->s_resp_data_.size(),
                              errcode);
    return;
  } else if (p_conn->request_type_ == HTTP_CB_TYPE_SERIAL_DATA) {
    HttpConnSerialDataComplete(p_conn,
                               p_conn->s_resp_data_.c_str(),
                               p_conn->s_resp_data_.size(),
                               errcode);
  } else if (p_conn->request_type_ == HTTP_CB_TYPE_GIO_TRIGGER) {
    HttpConnGioTriggerComplete(p_conn,
                               p_conn->s_resp_data_.c_str(),
                               p_conn->s_resp_data_.size(),
                               errcode);
  } else if (p_conn->request_type_ == HTTP_CB_TYPE_SNAPSHOT_IMAGE) {
    HttpConnSnapImageComplete(p_conn,
                              p_conn->s_resp_data_.c_str(),
                              p_conn->s_resp_data_.size(),
                              errcode);
  } else if (p_conn->request_type_ == HTTP_CB_TYPE_FTP_POST_CONN) {
    FtpConnPostFileComplete(p_conn,
                            p_conn->s_resp_data_.c_str(),
                            p_conn->s_resp_data_.size(),
                            errcode);
  } else if (p_conn->request_type_ == HTTP_CB_TYPE_DEVICE_REG) {
    HttpConnDeviceRegComplete(p_conn,
                              p_conn->s_resp_data_.c_str(),
                              p_conn->s_resp_data_.size(),
                              errcode);
  }
}

void SenderServer::HttpConnIvsResultComplete(hs::HttpConn *p_conn,
    const char *data, std::size_t data_size,
    int errcode) {
  LOG(L_INFO) << "HttpConnIvsResultComplete";
  if (p_conn == NULL) {
    LOG(L_INFO) << "http_conn.get() == NULL";
    return;
  }
  if (OnIvsPlateResponseHandle(data)) {
    HttpWriteResponseLogging(p_conn->s_url_, data);
  }

  std::string serr = "";
  if (false == CurlServices::isSuccess(errcode, serr)) {
    if (m_nOfflineCheckStatus) {
      check_info_.bActive = 0;
      DpClient_SendDpMessage(TCP_SRV_SET_OFFLINE_CHECK,
                             0, (char*)&check_info_,
                             sizeof(TCP_SRV_OFFLINE_CHECK_INFO));
    }
    HttpDpLogging(serr.c_str(), serr.length());

    LOG(L_INFO) << p_conn->s_url_;

    int resend_times = p_conn->n_resend_times_;
    if (resend_times <= 0)
      return;
    HttpPostData(p_conn->s_url_.c_str(),
                 p_conn->s_post_data_.c_str(),
                 p_conn->b_ssl_enabel_,
                 p_conn->n_url_port_,
                 p_conn->n_timeout_,
                 --resend_times,
                 HTTP_CB_TYPE_IVS_RESULT);
    LOG(L_INFO) << "resend_times : " << " : " << resend_times;

    return;
  }

  if (m_nOfflineCheckStatus) {
    DpClient_SendDpMessage(TCP_SRV_OFFLINE_CHECK_RESP,
                           0, NULL, 0);
    check_info_.bActive = 1;
    DpClient_SendDpMessage(TCP_SRV_SET_OFFLINE_CHECK,
                           0, (char*)&check_info_,
                           sizeof(TCP_SRV_OFFLINE_CHECK_INFO));
    //脱机检查心跳包
  }
}

void SenderServer::HttpConnGioTriggerComplete(hs::HttpConn* p_conn,
    const char     *data,
    std::size_t     data_size,
    int errcode) {

  if (p_conn == NULL)
    return;
  
  HttpWriteResponseLogging(p_conn->s_url_, data);

  std::string serr = "";
  if (false == CurlServices::isSuccess(errcode, serr)) {
    HttpDpLogging(serr.c_str(), serr.length());

    int resend_times = p_conn->n_resend_times_;
    if (resend_times > 0) {
      HttpPostData(p_conn->s_url_,
                   p_conn->s_post_data_,
                   p_conn->b_ssl_enabel_,
                   p_conn->n_url_port_,
                   p_conn->n_timeout_,
                   --resend_times,
                   HTTP_CB_TYPE_GIO_TRIGGER);
    }
    LOG(L_ERROR) << serr;
    return;
  }
  LOG(L_INFO).write(data, data_size);
}

void SenderServer::HttpConnSerialDataComplete(hs::HttpConn *p_conn,
    const char *data,
    std::size_t data_size,
    int errcode) {

  if (p_conn == NULL)
    return;

  HttpWriteResponseLogging(p_conn->s_url_, data);

  std::string serr = "";
  if (false == CurlServices::isSuccess(errcode, serr)) {
    HttpDpLogging(serr.c_str(), serr.length());

    int resend_times = p_conn->n_resend_times_;
    if (resend_times > 0) {
      HttpPostData(p_conn->s_url_,
                   p_conn->s_post_data_,
                   p_conn->b_ssl_enabel_,
                   p_conn->n_url_port_,
                   p_conn->n_timeout_,
                   --resend_times,
                   HTTP_CB_TYPE_SERIAL_DATA);
    }
    LOG(L_ERROR) << serr;
    return;
  }
  LOG(L_INFO).write(data, data_size);
  OnSerialResponseHandle(data);
}

void SenderServer::GetHttpPostDeviceInfo() {

}

std::string SenderServer::GetSystemInfoAccount0User() {
  Json::Reader reader;
  Json::Value user_js;
  std::string user;
  if(0 >= Kvdb_GetKey(Acount_Info, strlen(Acount_Info),
                      kvdb_get_key_cb, &user))
    return "admin";
  reader.parse(user,user_js);
  return user_js[0]["user"].asString();
}

std::string SenderServer::GetSystemInfoAccount0Password() {
  Json::Reader reader;
  Json::Value pass_js;
  std::string pass;
  if(0 >= Kvdb_GetKey(Acount_Info, strlen(Acount_Info),
                      kvdb_get_key_cb, &pass))
    return "admin";
  reader.parse(pass,pass_js);
  return pass_js[0]["password"].asString();
}

std::string SenderServer::GetSystemInfoTitle() {
  Json::Reader reader;
  Json::Value title_js;
  std::string title;
  if(0 >= Kvdb_GetKey(Sys_Title, strlen(Sys_Title),
                      kvdb_get_key_cb, &title)) {
    LOG(L_ERROR) << "Get Device Title Failed";
    return "IVS";
  }
  if(!reader.parse(title,title_js)) {
    LOG(L_ERROR) << "Parse Device Title Failed";
    return "IVS";
  }
  return title_js["title"].asString();
}
std::string SenderServer::GetSystemInfoDeviceIp() {
  Json::Reader reader;
  Json::Value ip_js;
  std::string ip;
  //if(!kvdb_client_->GetKey(NetworkInterface_Cfg, &ip))
  if(0 >= Kvdb_GetKey(NetworkInterface_Cfg, strlen(NetworkInterface_Cfg),
                      kvdb_get_key_cb, &ip) == KVDB_FALSE)
    return "192.168.1.100";
  reader.parse(ip,ip_js);
  return ip_js["ip"].asString();
}

void SenderServer::DeviceRegFunc() {
  if (REG_CANCEL_HEARTBEAT != device_reg_settings_.enable) {
    Json::Value deviceregdata;

#ifndef WIN32
    std::string title = GetSystemInfoTitle();
    std::string user = GetSystemInfoAccount0User();
    std::string pass = GetSystemInfoAccount0Password();
    data_parse_.GetDeviceRegJson(deviceregdata,
                                 (char *)title.c_str(),
                                 (char *)user.c_str(),
                                 (char *)pass.c_str(),
                                 m_httpport);
#else
    data_parse_.GetDeviceRegJson(deviceregdata,
                                 "IVS",
                                 "admin",
                                 "admin",
                                 "80");
#endif
    GetSendUrl(server_settings_.hostname, device_reg_settings_.uri);
    HttpWriteRequestLogging(HTTP_DEVICE_REG, "");

    // dev reg
    DeviceRegData regdata;
    regdata.device_name = deviceregdata[JSON_DEVICE_NAME].asString();
    regdata.ipaddr = deviceregdata[JSON_IP_ADDRESS].asString();
    regdata.port = deviceregdata[JSON_PORT].asString();
    regdata.user_name = deviceregdata[JSON_USERNAME].asString();
    regdata.pass_wd = deviceregdata[JSON_PASSWORD].asString();
    regdata.serialno = deviceregdata[JSON_SERIALNO].asString();
    regdata.channel_num = deviceregdata[JSON_CHANNEL_NUM].asString();

    int timeout = device_reg_settings_.enable == REG_COMET_POLLING ?
                  COMET_CONNTIME : server_settings_.http_timeout;
    HttpConn *p_conn = curl_services_->CreateHttpConn(m_url,
                       "devecireg",
                       server_settings_.enable_ssl == 1,
                       m_port,
                       timeout,
                       0,
                       HTTP_CB_TYPE_DEVICE_REG,
                       this);

    if (p_conn == NULL)
      return;

    curl_services_->PostDevRegData(p_conn, regdata);
  }
}

void SenderServer::FtpConnPostFileComplete(hs::HttpConn *p_conn,
    const char   *data,
    std::size_t   data_size,
    int errcode) {
  std::string serr = "";
  if (false == CurlServices::isSuccess(errcode, serr)) {
    LOG(L_ERROR) << p_conn->s_url_;
  } else {
    LOG(L_INFO) << p_conn->s_url_ << " : " << "FtpConnPostFileComplete";
  }
}


void SenderServer::NewIPCFtpPostConn(char *filepath) {
  Json::Value ftpimgdata;
  UserGetImgInfo imginfo;
  std::string title = GetSystemInfoTitle();
  data_parse_.GetFtpPostJson(imginfo,
                             ftpimgdata,
                             (char*)ftp_json_[SERVIER_IP].asString().c_str(),
                             (char*)ftp_json_[FOLDER_NAME].asString().c_str(),
                             filepath,
                             title);
  imginfo.curpose = 0;
  imginfo.posturl = ftpimgdata[JSON_FTP_URL].asString();

  HttpConn *p_conn = curl_services_->CreateHttpConn(
                       imginfo.posturl,
                       "ftp_post",
                       false,
                       ftp_json_[PORT_STR].asInt(),
                       10,
                       0,
                       HTTP_CB_TYPE_FTP_POST_CONN,
                       this);
  if (p_conn == NULL)
    return;
  curl_services_->PostImageFile(p_conn,
                                &imginfo,
                                ftp_json_[USER_NAME].asString(),
                                ftp_json_[PASSWORD].asString());
}

void SenderServer::HttpConnDeviceRegComplete(hs::HttpConn *http_conn,
    const char *data,
    std::size_t data_size,
    int errcode) {
  //LOG(L_INFO).write(data, data_size);

  if (http_conn == NULL)
    return;

  std::string serr = "";
  if (REG_COMMON_HEARTBEAT == device_reg_settings_.enable) {
    int time_out = COMET_CONNTIME;
    if(false == CurlServices::isSuccess(errcode, serr)
       && m_nOfflineCheckStatus) {
      time_out = 1;
      check_info_.bActive = 0;
      DpClient_SendDpMessage(TCP_SRV_SET_OFFLINE_CHECK,
                             0, (char*)&check_info_,
                             sizeof(TCP_SRV_OFFLINE_CHECK_INFO));
    } else if(m_nOfflineCheckStatus) {
      check_info_.bActive = 1;
      DpClient_SendDpMessage(TCP_SRV_SET_OFFLINE_CHECK,
                             0, (char*)&check_info_,
                             sizeof(TCP_SRV_OFFLINE_CHECK_INFO));
    }

    c_deg_reg_timer_.Start(time_out*1000, 0);
  } else if (REG_COMET_POLLING == device_reg_settings_.enable) {

    if (false == CurlServices::isSuccess(errcode, serr)) {
      c_deg_reg_timer_.Start(2*1000, 0);
    } else {
      if (OnIvsPlateResponseHandle(data)) {
        HttpWriteResponseLogging(http_conn->s_url_, data);
      }
      DeviceRegFunc();
    }
  }

  if (false == CurlServices::isSuccess(errcode, serr)) {
    HttpDpLogging(serr.c_str(), serr.length());
    m_HttpServerPollLineStatus = false;
    LOG(L_ERROR) << http_conn->s_url_;
    return;
  }
  m_HttpServerPollLineStatus = true;
}

void SenderServer::OnIvsResult(const DpMessage* dmp) {
  LOG(L_INFO) << "New OnIvsResult !!!";
  if (dmp->data_size != sizeof(IVS_RESULT_PARAM)) {
    LOG(L_ERROR) << "The sizeof(IVS_RESULT_PARAM) is not " << dmp->data_size;
    return;
  }

  if(ftp_json_[B_ENABLE_FTPUPDATE].asInt())
    NewIPCFtpPostConn(((IVS_RESULT_PARAM *)(dmp->data))->imageSDPath);
  if (!plateresult_settings_.enable) {
    LOG(L_WARNING) << "plateresult_settings_ is disabled !!!";
    return;
  }

  Json::Value platedata;
#ifndef WIN32
  std::string title = GetSystemInfoTitle();
  data_parse_.GetPlateJson((IVS_RESULT_PARAM *)(dmp->data),
                           platedata,
                           &plateresult_settings_,
                           (char *)title.c_str());
#else
  data_parse_.GetPlateJson((IVS_RESULT_PARAM *)(dmp->data),
                           platedata,
                           &plateresult_settings_,
                           "IVS");
#endif
  GetSendUrl(server_settings_.hostname, plateresult_settings_.uri);
  HttpWriteRequestLogging(HTTP_IVS_RESULT,
                          platedata[JSON_ALARMINFO_PLATE][JSON_RESULT][JSON_PLATE_RESULT][JSON_LICENSE].asString());

  if (m_HttpServerPoll == 0) {
    NewIPCConn(platedata,
               m_HttpResendTimes,
               server_settings_.http_timeout,
               HTTP_CB_TYPE_IVS_RESULT);

    NewIPCConnExt(HTTP_IVS_RESULT,
                  platedata,
                  platedata[JSON_LICENSE].asString(),
                  plateresult_settings_.uri,
                  HTTP_CB_TYPE_IVS_RESULT);
  } else {
    if (ip_extjs_[JSON_IP_EXT].type() == Json::nullValue ||
        m_HttpServerPollLineStatus) {
      NewIPCConn(platedata,
                 m_HttpResendTimes,
                 server_settings_.http_timeout,
                 HTTP_CB_TYPE_IVS_RESULT);
    } else {
      NewIPCConnExt(HTTP_IVS_RESULT,
                    platedata,
                    platedata[JSON_LICENSE].asString(),
                    plateresult_settings_.uri,
                    HTTP_CB_TYPE_IVS_RESULT);
    }
  }
}


void SenderServer::OnSerial(const DpMessage* dmp) {
  if (dmp->data_size < sizeof(TT_Param)) {
    return;
  }

  if (!serial_settings_.enable) {

    LOG(L_WARNING) << "serial_settings_ is disabled !!!";
    return;
  }

  TT_Param data;
  memcpy(&data, dmp->data, sizeof(TT_Param));

  GetSendUrl(server_settings_.hostname,
             serial_settings_.uri);
  memcpy(m_serialdata, dmp->data + sizeof(TT_Param),
         dmp->data_size - sizeof(TT_Param));
  m_serialdata[dmp->data_size - sizeof(TT_Param)] = '\0';

  Json::Value serialdata;
  data_parse_.GetRs485Json(serialdata,
                           data.source,
                           m_serialdata,
                           dmp->data_size - sizeof(TT_Param));
  HttpWriteRequestLogging(HTTP_SERIAL_DATA, m_serialdata);

  if (m_HttpServerPoll == 0) {
    NewIPCConn(serialdata,
               m_HttpResendTimes,
               server_settings_.http_timeout,
               HTTP_CB_TYPE_SERIAL);
    NewIPCConnExt(HTTP_SERIAL_DATA,
                  serialdata,
                  m_serialdata,
                  serial_settings_.uri,
                  HTTP_CB_TYPE_SERIAL);
  } else {
    if (ip_extjs_[JSON_IP_EXT].type() == Json::nullValue ||
        m_HttpServerPollLineStatus) {
      NewIPCConn(serialdata,
                 m_HttpResendTimes,
                 server_settings_.http_timeout,
                 HTTP_CB_TYPE_SERIAL);
    } else {
      NewIPCConnExt(HTTP_SERIAL_DATA,
                    serialdata,
                    m_serialdata,
                    serial_settings_.uri,
                    HTTP_CB_TYPE_SERIAL);
    }
  }
}

void SenderServer::OnGioTrigger(const DpMessage* dmp) {
  if (dmp->data_size != sizeof(TRIGGER_REULST))return;
  if (!gpio_settings_.enable) {

    LOG(L_WARNING) << "gpio_settings_ is disabled !!!";
    return;
  }
  TRIGGER_REULST *trigger_result = (TRIGGER_REULST *)(dmp->data);
  Json::Value giodata;
#ifndef WIN32
  std::string title = GetSystemInfoTitle();
  data_parse_.GetGIOJson(giodata, (TRIGGER_REULST *)(dmp->data),
                         (char *)title.c_str());
#else
  data_parse_.GetGIOJson(giodata, (TRIGGER_REULST *)(dmp->data), "IVS");
#endif
  GetSendUrl(server_settings_.hostname,
             gpio_settings_.uri);

  if (m_HttpServerPoll == 0) {
    NewIPCConn(giodata,
               m_HttpResendTimes,
               server_settings_.http_timeout,
               HTTP_CB_TYPE_GIO_TRIGGER);
    NewIPCConnExt(HTTP_GIO_TRIGGER, giodata, "", gpio_settings_.uri,
                  HTTP_CB_TYPE_GIO_TRIGGER);
  } else {

    if (ip_extjs_[JSON_IP_EXT].type() == Json::nullValue ||
        m_HttpServerPollLineStatus) {
      NewIPCConn(giodata, m_HttpResendTimes, server_settings_.http_timeout,
                 HTTP_CB_TYPE_GIO_TRIGGER);
    } else {
      NewIPCConnExt(HTTP_GIO_TRIGGER, giodata, "", gpio_settings_.uri,
                    HTTP_CB_TYPE_GIO_TRIGGER);
    }
  }
}

int SenderServer::WriteDpLoggingMessage(unsigned char proc_id,
                                        unsigned short type,
                                        unsigned char msg_size,
                                        const char *logging_msg,
                                        unsigned int event_id) {
  static char buffer[sizeof(OptimizeLogging) + MAX_LOGGING_TEXT_SIZE];
  OptimizeLogging *pol = (OptimizeLogging *)(buffer);
  static struct timeval tv;
  //struct timezone tz;
#ifndef WIN32
  gettimeofday(&tv, NULL);
  pol->millisec = (int64_t)tv.tv_sec * 1000000 + (int64_t)tv.tv_usec;
#else
  pol->millisec = GetTickCount();
#endif
  pol->event_id = event_id;
  pol->proc_id = proc_id;
  pol->type = type;
  pol->msg_size = msg_size;
  if (pol->msg_size && logging_msg) {
    memcpy(buffer + sizeof(OptimizeLogging), logging_msg, msg_size);
  }
  return DpClient_SendDpMessage("OPTIMIZE_LOGGING",
                                0,
                                buffer,
                                sizeof(OptimizeLogging) + msg_size);
}

void SenderServer::HttpDpLogging(const char *logging_msg,
                                 unsigned char msg_size) {
  if (m_LoggingEnable)
    WriteDpLoggingMessage(ID_HTTP_SEND,
                          LOGGING_HTTP_SEND_SERVER,
                          msg_size,
                          logging_msg,
                          0
                         );
}

int SenderServer::SetIpExt(const char *pdata, int datalen) {
  memcpy(m_ipext, pdata, datalen);
  m_ipext[datalen] = '\0';
  Json::Reader reader;
  reader.parse(m_ipext, ip_extjs_);
  if (KVDB_RET_SUCCEED == Kvdb_SetKey(http_ipext_key,
                                      strlen(http_ipext_key),
                                      m_ipext,
                                      MAX_IP_EXT_LEN)) {
    return KVDB_TRUE;
  }
  return KVDB_FALSE;
}

int SenderServer::SetCenterServerNet(const char* net) {
  memcpy(&server_settings_, net, sizeof(VZ_CenterServer_Net));
  if(KVDB_RET_SUCCEED == Kvdb_SetKey(http_centerservernet_value,
                                     strlen(http_centerservernet_value),
                                     net, sizeof(VZ_CenterServer_Net))) {
    return KVDB_TRUE;
  }
  return KVDB_FALSE;
}

int SenderServer::SetCenterServerDeviceReg(const char* devicereg) {
  __u8 oldreg = device_reg_settings_.enable;
  memcpy(&device_reg_settings_,
         devicereg, sizeof(VZ_CenterServer_DeviceReg));
  if (oldreg != device_reg_settings_.enable && 0 == oldreg) {
    DeviceRegFunc();
  }
  if (KVDB_RET_SUCCEED == Kvdb_SetKey(http_centerserverreg_value,
                                      strlen(http_centerserverreg_value),
                                      devicereg, sizeof(VZ_CenterServer_DeviceReg))) {
    return KVDB_TRUE;
  }
  return KVDB_FALSE;
}

int SenderServer::SetCenterServerPlateResult(const char* plateresult) {
  memcpy(&plateresult_settings_,
         plateresult,
         sizeof(VZ_CenterServer_PlateResult));
  if (KVDB_RET_SUCCEED == Kvdb_SetKey(http_centerserveralarm_value,
                                      strlen(http_centerserveralarm_value),
                                      plateresult, sizeof(VZ_CenterServer_PlateResult))) {
    return KVDB_TRUE;
  }
  return KVDB_FALSE;
}

int SenderServer::SetCenterServerGio(const char* gio) {
  memcpy(&gpio_settings_, gio, sizeof(VZ_CenterServer_GioInAlarm));
  if (KVDB_RET_SUCCEED == Kvdb_SetKey(http_centerserverio_value,
                                      strlen(http_centerserverio_value),
                                      gio, sizeof(VZ_CenterServer_GioInAlarm))) {
    return KVDB_TRUE;
  }
  return KVDB_FALSE;
}


int SenderServer::SetCenterServerSerial(const char* serial) {
  memcpy(&serial_settings_,
         serial,
         sizeof(VZ_CenterServer_Serial));
  if (KVDB_RET_SUCCEED == Kvdb_SetKey(http_centerserverserial_value,
                                      strlen(http_centerserverserial_value),
                                      serial, sizeof(VZ_CenterServer_Serial))) {
    return KVDB_TRUE;
  }
  return KVDB_FALSE;
}

int SenderServer::SetHttpResendTimes(const char* resendtimes) {

  memcpy(&m_HttpResendTimes,
         resendtimes,
         sizeof(int));
  if (KVDB_RET_SUCCEED == Kvdb_SetKey(http_resendtimes_value,
                                      strlen(http_resendtimes_value),
                                      resendtimes, sizeof(int))) {
    return KVDB_TRUE;
  }
  return KVDB_FALSE;
}

int SenderServer::SetHttpServerPoll(const char* poll) {
  memcpy(&m_HttpServerPoll,
         poll,
         sizeof(int));
  if (KVDB_RET_SUCCEED == Kvdb_SetKey(http_serverpoll_value,
                                      strlen(http_serverpoll_value),
                                      poll, sizeof(int))) {
    return KVDB_TRUE;
  }
  return KVDB_FALSE;
}

int SenderServer::SetHttpOfflineCheckStatus(const char* status) {

  int newstatus = m_nOfflineCheckStatus;
  memcpy(&newstatus,
         status,
         sizeof(int));
  if (1 == newstatus && newstatus != m_nOfflineCheckStatus) {
    check_info_.bActive = 1;
    DpClient_SendDpMessage(TCP_SRV_SET_OFFLINE_CHECK,
                           0, (char*)&check_info_,
                           sizeof(TCP_SRV_OFFLINE_CHECK_INFO));
  }
  m_nOfflineCheckStatus = newstatus;

  if (KVDB_RET_SUCCEED == Kvdb_SetKey(http_offlinecheckstatus_value,
                                      strlen(http_offlinecheckstatus_value),
                                      status, sizeof(int))) {
    return KVDB_TRUE;
  }
  return KVDB_FALSE;
}

void SenderServer::SetReplyJsonStatus(Json::Value &replyjson,
                                      std::string type,
                                      int status,
                                      std::string errmsg,
                                      std::string body) {
  replyjson[JSON_STATE] = status;
  replyjson[JSON_ERROR_MSG] = errmsg;
}

bool SenderServer::GetFtpConfig(Json::Value &json) {
  SetReplyJsonStatus(json,ERROR_MSG_SUCCESS,200,ERROR_MSG_SUCCESS);
  json[JSON_BODY] = ftp_json_;
  return true;
}

bool SenderServer::GetEmailConfig(Json::Value &json) {
  SetReplyJsonStatus(json,ERROR_MSG_SUCCESS,200,ERROR_MSG_SUCCESS);
  json[JSON_BODY] = email_json_;
  return true;
}

bool SenderServer::SendFtpTestFile(Json::Value &json) {
  FILE *fp;
  fp = fopen(DEFAULT_FTP_TEST_FILE_PATH,"w+");
  if(fp == NULL) {
    SetReplyJsonStatus(json,WRONG_OPEN_TEST_FILE,401,WRONG_OPEN_TEST_FILE);
    LOG(L_ERROR) << "Create Ftp Test File Failure";
    return false;
  }
  char bufRead[128] = {0};
  struct tm *pTm = NULL;
  time_t itime;
  itime = time(NULL);
  pTm = localtime(&itime);
  if(pTm == NULL) {
    SetReplyJsonStatus(json,WRONG_GET_LOCAL_TIME,401,WRONG_GET_LOCAL_TIME);
    LOG(L_ERROR) << "Get localtime Failure";
    return false;
  }
  sprintf(bufRead,"%d年%d月%d日 %d:%d:%d",pTm->tm_year+1900, pTm->tm_mon+1, pTm->tm_mday, pTm->tm_hour, pTm->tm_min, pTm->tm_sec);
  fwrite(bufRead, 1, strlen(bufRead), fp);
  fclose(fp);

  SetReplyJsonStatus(json,ERROR_MSG_SUCCESS,200,ERROR_MSG_SUCCESS);
  NewIPCFtpPostConn(DEFAULT_FTP_TEST_FILE_PATH);
}

bool SenderServer::SetEmailConfig(Json::Value &json,Json::Value &repjson) {
  if(json[JSON_BODY].isNull()) {
    SetReplyJsonStatus(repjson,DEFAULT_BODY_DATA,401,DEFAULT_BODY_DATA);
    return false;
  }
  Json::Value body = json[JSON_BODY];
  if(!body[SERVIER_IP].isNull()) {
    email_json_[SERVIER_IP] = body[SERVIER_IP].asString();
  }
  if(!body[USER_NAME].isNull()) {
    email_json_[USER_NAME] = body[USER_NAME].asString();
  }
  if(!body[PASSWORD].isNull()) {
    email_json_[PASSWORD] = body[PASSWORD].asString();
  }
  if(!body[SENDER_EMAIL].isNull()) {
    email_json_[SENDER_EMAIL] = body[SENDER_EMAIL].asString();
  }
  if(!body[TEXT_STR].isNull()) {
    email_json_[TEXT_STR] = body[TEXT_STR].asString();
  }
  if(!body[RECEIVER_EMAIL].isNull()) {
    email_json_[RECEIVER_EMAIL] = body[RECEIVER_EMAIL].asString();
  }
  if(!body[CC_STR].isNull()) {
    email_json_[CC_STR] = body[CC_STR].asString();
  }
  if(!body[SUBJECT].isNull()) {
    email_json_[SUBJECT] = body[SUBJECT].asString();
  }
  if(!body[AUTHENTICATION].isNull()) {
    email_json_[AUTHENTICATION] = body[AUTHENTICATION].asInt();
  }
  if(!body[B_ENABLE_MAILUPDATE].isNull()) {
    email_json_[B_ENABLE_MAILUPDATE] = body[B_ENABLE_MAILUPDATE].asInt();
  }
  if(!body[ATTACHMENTS].isNull()) {
    email_json_[ATTACHMENTS] = body[ATTACHMENTS].asInt();
  }
  if(!body[VIEW_STR].isNull()) {
    email_json_[VIEW_STR] = body[VIEW_STR].asInt();
  }
  if(!body[ASMT_PATTACH].isNull()) {
    email_json_[ASMT_PATTACH] = body[ASMT_PATTACH].asInt();
  }
  if(!body[ATTFILE_FORMAT].isNull()) {
    email_json_[ATTFILE_FORMAT] = body[ATTFILE_FORMAT].asInt();
  }

  SetReplyJsonStatus(repjson,ERROR_MSG_SUCCESS,200,ERROR_MSG_SUCCESS);
  Kvdb_SetKey(http_email_config_value,
              strlen(http_email_config_value),
              email_json_.toStyledString().c_str(),
              MAX_IP_EXT_LEN);
  return true;
}

bool SenderServer::SetFtpConfig(Json::Value &json,Json::Value &repjson) {
  if(json[JSON_BODY].isNull()) {
    SetReplyJsonStatus(repjson,DEFAULT_BODY_DATA,401,DEFAULT_BODY_DATA);
    return false;
  }
  Json::Value body = json[JSON_BODY];
  if(!body[SERVIER_IP].isNull()) {
    ftp_json_[SERVIER_IP] = body[SERVIER_IP].asString();
  }
  if(!body[USER_NAME].isNull()) {
    ftp_json_[USER_NAME] = body[USER_NAME].asString();
  }
  if(!body[PASSWORD].isNull()) {
    ftp_json_[PASSWORD] = body[PASSWORD].asString();
  }
  if(!body[FOLDER_NAME].isNull()) {
    ftp_json_[FOLDER_NAME] = body[FOLDER_NAME].asString();
  }
  if(!body[IMAGE_ACOUNT].isNull()) {
    ftp_json_[IMAGE_ACOUNT] = body[IMAGE_ACOUNT].asInt();
  }
  if(!body[P_ID].isNull()) {
    ftp_json_[P_ID] = body[P_ID].asInt();
  }
  if(!body[PORT_STR].isNull()) {
    ftp_json_[PORT_STR] = body[PORT_STR].asInt();
  }
  if(!body[B_ENABLE_FTPUPDATE].isNull()) {
    ftp_json_[B_ENABLE_FTPUPDATE] = body[B_ENABLE_FTPUPDATE].asInt();
  }

  Kvdb_SetKey(http_ftp_config_value,
              strlen(http_ftp_config_value),
              ftp_json_.toStyledString().c_str(),
              MAX_IP_EXT_LEN);
  SetReplyJsonStatus(repjson,ERROR_MSG_SUCCESS,200,ERROR_MSG_SUCCESS);
  return true;
}

int SenderServer::HttpSenderReqFunc(const char* reqdata,
                                    int size,
                                    std::string &replydata) {

  Json::Value replyjson;
  if(size <= 0 || reqdata == NULL) {
    LOG(L_ERROR) << "request data is Error";
    SetReplyJsonStatus(replyjson,ERROR_DATA,401,ERROR_DATA);
    replydata = replyjson.toStyledString();
    return -1;
  }
  Json::Reader reader;
  Json::Value reqjson;
  std::string reqstr(reqdata,size);
  if(!reader.parse(reqstr,reqjson)) {
    replydata = "request data is not json";
    SetReplyJsonStatus(replyjson,ERROR_DATA,401,ERROR_DATA);
    replydata = replyjson.toStyledString();
    LOG(L_ERROR) << replydata;
    return -1;
  }
  std::string type = reqjson[JSON_TYPE].asString();
  bool ret;
  if(type == SET_FTP_CONFIG) {
    ret = SetFtpConfig(reqjson,replyjson);
  } else if(type == GET_FTP_CONFIG) {
    ret = GetFtpConfig(replyjson);
  } else if(type == GET_EMAIL_CONFIG) {
    ret = GetEmailConfig(replyjson);
  } else if(type == SET_EMAIL_CONFIG) {
    ret = SetEmailConfig(reqjson,replyjson);
  } else if(type == SEND_FTP_TEST_FILE) {
    ret = SendFtpTestFile(replyjson);
  } else {
    ret = false;
    SetReplyJsonStatus(replyjson,type,401,WRONG_REQ_TYPE);
  }
  replyjson[JSON_TYPE] = type;
  replydata = replyjson.toStyledString();
  return ret ? 0 : -1 ;
}

void SenderServer::OnConfigControl(const DpMessage* dmp) {
  LOG(L_INFO) << "Message : " << dmp->method;
  int   ret = -1;
  int   res_size = 0;
  void  *res_data = &ret;
  std::string replystr;
  if (strcmp(dmp->method, HTTPSENDER_SET_HTTP_IP_EXT) == 0) {
    ret = SetIpExt(dmp->data, dmp->data_size);
    res_size = sizeof(ret);
  } else if (strcmp(dmp->method, HTTPSENDER_GET_HTTP_IP_EXT) == 0) {
    res_data = GetIpExt();
    res_size = (res_data == NULL ? 0 : strlen((char*)res_data));
  } else if (strcmp(dmp->method, HTTPSENDER_MSG_SET_CENTER_SERVER) == 0
             && dmp->data_size == sizeof(VZ_CenterServer_Net)) {
    ret = SetCenterServerNet(dmp->data);
    res_size = sizeof(ret);
  } else if (strcmp(dmp->method, HTTPSENDER_MSG_GET_CENTER_SERVER) == 0) {
    res_data = GetCenterServerNet();
    res_size = sizeof(VZ_CenterServer_Net);
  } else if (strcmp(dmp->method, HTTPSENDER_MSG_SET_CENTER_SERVER_DEVICE_REG) == 0
             && dmp->data_size == sizeof(VZ_CenterServer_DeviceReg)) {
    ret = SetCenterServerDeviceReg(dmp->data);
    res_size = sizeof(ret);
  } else if (strcmp(dmp->method, HTTPSENDER_MSG_GET_CENTER_SERVER_DEVICE_REG) == 0) {
    res_data = GetCenterServerDeviceReg();
    res_size = sizeof(VZ_CenterServer_DeviceReg);
  } else if (strcmp(dmp->method, HTTPSENDER_MSG_SET_CENTER_SERVER_ALARM_PLATE) == 0
             && dmp->data_size == sizeof(VZ_CenterServer_PlateResult)) {
    ret = SetCenterServerPlateResult(dmp->data);
    res_size = sizeof(ret);
  } else if (strcmp(dmp->method, HTTPSENDER_MSG_GET_CENTER_SERVER_ALARM_PLATE) == 0) {
    res_data = GetCenterServerPlateResult();
    res_size = sizeof(VZ_CenterServer_PlateResult);
  } else if (strcmp(dmp->method, HTTPSENDER_MSG_SET_CENTER_SERVER_ALARM_GIOIN) == 0
             && dmp->data_size == sizeof(VZ_CenterServer_GioInAlarm)) {
    ret = SetCenterServerGio(dmp->data);
    res_size = sizeof(ret);
  } else if (strcmp(dmp->method, HTTPSENDER_MSG_GET_CENTER_SERVER_ALARM_GIOIN) == 0) {
    res_data = GetCenterServerGio();
    res_size = sizeof(VZ_CenterServer_GioInAlarm);
  } else if (strcmp(dmp->method, HTTPSENDER_MSG_SET_CENTER_SERVER_SERIAL) == 0
             && dmp->data_size == sizeof(VZ_CenterServer_Serial)) {
    ret = SetCenterServerSerial(dmp->data);
    res_size = sizeof(ret);
  } else if (strcmp(dmp->method, HTTPSENDER_MSG_GET_CENTER_SERVER_SERIAL) == 0) {
    res_data = GetCenterServerSerial();
    res_size = sizeof(VZ_CenterServer_Serial);
  } else if (strcmp(dmp->method, HTTPSENDER_MSG_SET_OFFLINE_STATUS) == 0) {
    ret = SetHttpOfflineCheckStatus(dmp->data);
    res_size = sizeof(ret);
  } else if (strcmp(dmp->method, HTTPSENDER_MSG_GET_OFFLINE_STATUS) == 0) {
    res_data = GetHttpOfflineCheckStatus();
    res_size = sizeof(int);
  } else if (strcmp(dmp->method, HTTPSENDER_MSG_SET_REPUSH_NUMS) == 0) {
    ret = SetHttpResendTimes(dmp->data);
    res_size = sizeof(ret);
  } else if (strcmp(dmp->method, HTTPSENDER_MSG_GET_REPUSH_NUMS) == 0) {
    res_data = GetHttpResendTimes();
    res_size = sizeof(int);
  } else if (strcmp(dmp->method, HTTPSENDER_MSG_SET_SERVERPOLL_STATUS) == 0) {
    ret = SetHttpServerPoll(dmp->data);
    res_size = sizeof(ret);
  } else if (strcmp(dmp->method, HTTPSENDER_MSG_GET_SERVERPOLL_STATUS) == 0) {
    res_data = GetHttpServerPoll();
    res_size = sizeof(int);
  } else if (strcmp(dmp->method, HTTPSENDER_MSG_CONFIG_CONTROL) == 0) {
    HttpSenderReqFunc(dmp->data,
                      dmp->data_size,
                      replystr);
    res_data = (void*)replystr.c_str();
    res_size = replystr.length();
  }

  DpClient_SendDpReply(dmp->method,
                       0,
                       dmp->id,
                       (const char *)res_data,
                       res_size);
}

void SenderServer::OnLoggingStatus(const DpMessage* dmp) {
  memcpy(&m_LoggingEnable, dmp->data, sizeof(int));
}

void SenderServer::OnIpChange(const DpMessage* dmp) {
  unsigned int data[3];
  memcpy(data, dmp->data, 3 * sizeof(unsigned int));
  struct in_addr ip;
  ip.s_addr = data[0];
  data_parse_.SetDeviceIp(inet_ntoa(ip));
}

void SenderServer::dp_cli_msg_cb(DPPollHandle p_hdl, const DpMessage *p_dpm, void* p_usr_arg) {
  if (p_usr_arg) {
    ((SenderServer*)p_usr_arg)->OnDpMessage(p_hdl, p_dpm);
    return;
  }
  LOG(L_ERROR) << "param is null.";
}

void SenderServer::OnDpMessage(DPPollHandle p_hdl, const DpMessage* p_dpm) {
  if (NULL == p_dpm) {
    LOG(L_ERROR) << "param is null.";
    return;
  }
  p_cur_dp_msg_ = const_cast<DpMessage*>(p_dpm);

  if (0 == strncmp(p_dpm->method, EVT_SRV_IVS_RESULT, MAX_METHOD_SIZE)) {
    // 来自ES处理后的的识别结果
    OnIvsResult(p_dpm);
  } else if (0 == strncmp(p_dpm->method, EVT_SRV_TT_TCP, MAX_METHOD_SIZE)) {
    OnSerial(p_dpm);
  } else if (0 == strncmp(p_dpm->method, EVT_SRV_EXTERNAL_TRIGGER, MAX_METHOD_SIZE)) {
    OnGioTrigger(p_dpm);
  } else if ((0 == strncmp(p_dpm->method, HTTPSENDER_SET_HTTP_GPIO_NUM, MAX_METHOD_SIZE)) ||
             (0 == strncmp(p_dpm->method, HTTPSENDER_GET_HTTP_GPIO_NUM, MAX_METHOD_SIZE)) ||
             (0 == strncmp(p_dpm->method, HTTPSENDER_SET_HTTP_IP_EXT, MAX_METHOD_SIZE)) ||
             (0 == strncmp(p_dpm->method, HTTPSENDER_GET_HTTP_IP_EXT, MAX_METHOD_SIZE)) ||
             (0 == strncmp(p_dpm->method, HTTPSENDER_MSG_SET_CENTER_SERVER, MAX_METHOD_SIZE)) ||
             (0 == strncmp(p_dpm->method, HTTPSENDER_MSG_GET_CENTER_SERVER, MAX_METHOD_SIZE)) ||
             (0 == strncmp(p_dpm->method, HTTPSENDER_MSG_SET_CENTER_SERVER_DEVICE_REG, MAX_METHOD_SIZE)) ||
             (0 == strncmp(p_dpm->method, HTTPSENDER_MSG_GET_CENTER_SERVER_DEVICE_REG, MAX_METHOD_SIZE)) ||
             (0 == strncmp(p_dpm->method, HTTPSENDER_MSG_SET_CENTER_SERVER_ALARM_PLATE, MAX_METHOD_SIZE)) ||
             (0 == strncmp(p_dpm->method, HTTPSENDER_MSG_GET_CENTER_SERVER_ALARM_PLATE, MAX_METHOD_SIZE)) ||
             (0 == strncmp(p_dpm->method, HTTPSENDER_MSG_SET_CENTER_SERVER_ALARM_GIOIN, MAX_METHOD_SIZE)) ||
             (0 == strncmp(p_dpm->method, HTTPSENDER_MSG_GET_CENTER_SERVER_ALARM_GIOIN, MAX_METHOD_SIZE)) ||
             (0 == strncmp(p_dpm->method, HTTPSENDER_MSG_SET_CENTER_SERVER_SERIAL, MAX_METHOD_SIZE)) ||
             (0 == strncmp(p_dpm->method, HTTPSENDER_MSG_GET_CENTER_SERVER_SERIAL, MAX_METHOD_SIZE)) ||
             (0 == strncmp(p_dpm->method, HTTPSENDER_MSG_SET_OFFLINE_STATUS, MAX_METHOD_SIZE)) ||
             (0 == strncmp(p_dpm->method, HTTPSENDER_MSG_GET_OFFLINE_STATUS, MAX_METHOD_SIZE)) ||
             (0 == strncmp(p_dpm->method, HTTPSENDER_MSG_SET_REPUSH_NUMS, MAX_METHOD_SIZE)) ||
             (0 == strncmp(p_dpm->method, HTTPSENDER_MSG_GET_REPUSH_NUMS, MAX_METHOD_SIZE)) ||
             (0 == strncmp(p_dpm->method, HTTPSENDER_MSG_SET_SERVERPOLL_STATUS, MAX_METHOD_SIZE)) ||
             (0 == strncmp(p_dpm->method, HTTPSENDER_MSG_GET_SERVERPOLL_STATUS, MAX_METHOD_SIZE)) ||
             (0 == strncmp(p_dpm->method, HTTPSENDER_MSG_CONFIG_INTERFACE, MAX_METHOD_SIZE)) ||
             (0 == strncmp(p_dpm->method, HTTPSENDER_MSG_CONFIG_CONTROL, MAX_METHOD_SIZE))) {
    OnConfigControl(p_dpm);
  } else if (0 == strncmp(p_dpm->method, SYS_SRV_IP_CHANGED, MAX_METHOD_SIZE)) {
    OnIpChange(p_dpm);
  } else if (0 == strncmp(p_dpm->method, DP_LOGGING_ENABLE, MAX_METHOD_SIZE)) {
    OnLoggingStatus(p_dpm);
  } else if (0 == strncmp(p_dpm->method, AVS_GET_SNAP, MAX_METHOD_SIZE)) {
    //OnGetSnapShotImage(p_dpm);
  } else {
    LOG(L_ERROR) << "no function process this method.";
  }
}

void SenderServer::dp_cli_state_cb(DPPollHandle p_hdl, uint32 n_state, void* p_usr_arg) {
  if (p_usr_arg) {
    ((SenderServer*)p_usr_arg)->OnDpState(p_hdl, n_state);
    return;
  }
  LOG(L_ERROR) << "param is null.";
}

void SenderServer::OnDpState(DPPollHandle p_hdl, uint32 n_state) {
  int32 n_ret = 0;
  if (n_state == DP_CLIENT_DISCONNECT) {
    n_ret = DpClient_HdlReConnect(p_hdl);
    if (n_ret == VZNETDP_SUCCEED) {
      // 重新注册消息
      n_ret = DpClient_HdlAddListenMessage(p_hdl,
                                           MSG_METHOD_SET, MSG_METHOD_SET_CNT);

      if (n_ret == VZNETDP_FAILURE) {
        // 断开链接
      } else {
        DeviceRegFunc();
      }
    }
  }
}

void SenderServer::kvdb_get_key_cb(const char *p_key, int n_key,
                                   const char *p_value, int n_value,
                                   void *p_user_data) {
  if (!p_key || !p_value || !p_user_data) {
    LOG(L_ERROR) << "param is null.";
    return;
  }

  if ((0 == strncmp(p_key, sys_hwinfo, MAX_KVDB_KEY_SIZE)) ||
      (0 == strncmp(p_key, Acount_Info, MAX_KVDB_KEY_SIZE)) ||
      (0 == strncmp(p_key, NetworkInterface_Cfg, MAX_KVDB_KEY_SIZE)) ||
      (0 == strncmp(p_key, Sys_Title, MAX_KVDB_KEY_SIZE))) {
    ((std::string*)p_user_data)->append(p_value, n_value);
    return;
  }
  LOG(L_ERROR) << "param is null." << p_key;
}

void SenderServer::InitHttpDataInfo() {
  Json::Reader reader;
  iGetKeyValueFunc(http_centerservernet_value,
                   strlen(http_centerservernet_value),
                   (char*)&server_settings_,
                   sizeof(VZ_CenterServer_Net), CENTERSERVER_NET);
  iGetKeyValueFunc(http_centerserverreg_value,
                   strlen(http_centerserverreg_value),
                   (char*)&device_reg_settings_,
                   sizeof(VZ_CenterServer_DeviceReg), CENTERSERVER_DEVICEREG);
  iGetKeyValueFunc(http_centerserveralarm_value,
                   strlen(http_centerserveralarm_value),
                   (char*)&plateresult_settings_,
                   sizeof(VZ_CenterServer_PlateResult), CENTERSERVER_PLATERESULT);
  iGetKeyValueFunc(http_centerserverio_value,
                   strlen(http_centerserverio_value),
                   (char*)&gpio_settings_,
                   sizeof(VZ_CenterServer_GioInAlarm), CENTERSERVER_GIOINALARM);
  iGetKeyValueFunc(http_centerserverserial_value,
                   strlen(http_centerserverserial_value),
                   (char*)&serial_settings_,
                   sizeof(VZ_CenterServer_Serial), CENTERSERVER_SERIAL);
  iGetKeyValueFunc(http_ipext_key, strlen(http_ipext_key), m_ipext,
                   MAX_IP_EXT_LEN, CENTERSERVER_IPEXT);

  iGetKeyValueFunc(http_offlinecheckstatus_value,
                   strlen(http_offlinecheckstatus_value),
                   (char*)&m_nOfflineCheckStatus, sizeof(int),
                   CENTERSERVER_OFFLINECHECK);
  iGetKeyValueFunc(http_resendtimes_value,
                   strlen(http_resendtimes_value),
                   (char*)&m_HttpResendTimes, sizeof(int),
                   CENTERSERVER_OFFLINECHECK);
  iGetKeyValueFunc(http_serverpoll_value,
                   strlen(http_serverpoll_value),
                   (char*)&m_HttpServerPoll, sizeof(int),
                   CENTERSERVER_SERVERPOLL);
  if(iGetKeyValueFunc(http_ftp_config_value,
                      strlen(http_ftp_config_value),
                      m_ftpbuffer, MAX_FTP_EXT_LEN,
                      HTTP_FTP_CONFIG))
    reader.parse(m_ftpbuffer, ftp_json_);
  if(iGetKeyValueFunc(http_email_config_value,
                      strlen(http_email_config_value),
                      m_emailbuffer, MAX_EMAIL_EXT_LEN,
                      HTTP_EMAIL_CONFIG))
    reader.parse(m_emailbuffer, email_json_);

  Json::Value hwinfo_js;
  std::string hwinfo;
  if(0 < Kvdb_GetKey(sys_hwinfo, strlen(sys_hwinfo),
                 kvdb_get_key_cb, &hwinfo))
    reader.parse(hwinfo, hwinfo_js);

  LOG(L_INFO) << "iInitHwiInfo";
  data_parse_.iInitHwiInfo(hwinfo_js);

  LOG(L_INFO) << "parse";
  reader.parse(m_ipext, ip_extjs_);
  //check_info_.bActive = m_nOfflineCheckStatus;
}

bool SenderServer::iGetKeyValueFunc(const char *p_key, int n_key,
                                    char *buffer, int bufferlen,
                                    int datatype) {
  LOG(L_INFO) << "GET " << p_key;
  if (0 < Kvdb_GetKeyAbsolutelyToBuffer(
        p_key, n_key, buffer, bufferlen)) {
    LOG(L_INFO) << "Read key form kvdb " << p_key;
    return true;
  }

  if(system_json_[JSON_DEVICE][JSON_NET][JSON_CENTER_SERVER].isNull()) {
    Kvdb_SetKey(p_key, n_key, buffer, bufferlen);
    return true;
  }
  Json::Value centerserver = system_json_[JSON_DEVICE][JSON_NET][JSON_CENTER_SERVER];
  switch (datatype) {
  case CENTERSERVER_NET:
    StringToChar(centerserver[JSON_HOST_NAME].asString(),
                 server_settings_.hostname,
                 MAX_HOSTNAME_LEN);
    server_settings_.port = centerserver[JSON_PORT].asInt();
    server_settings_.enable_ssl = centerserver[JSON_ENABLE_SSL].asInt();
    server_settings_.ssl_port = centerserver[JSON_SSL_PORT].asInt();
    server_settings_.http_timeout = centerserver[JSON_HTTP_TIMEOUT].asInt();
    memcpy(buffer,
           &server_settings_,
           bufferlen);
    break;
  case CENTERSERVER_DEVICEREG:
    device_reg_settings_.enable = centerserver[JSON_ENABLE_DEVICEREG].asInt();
    StringToChar(centerserver[JSON_DEVICEREG_URI].asString(),
                 device_reg_settings_.uri,
                 MAX_URI_LEN);
    memcpy(buffer,
           &device_reg_settings_,
           bufferlen);
    break;
  case CENTERSERVER_PLATERESULT:
    plateresult_settings_.enable = centerserver[JSON_ALARM_ENABLE].asInt();
    StringToChar(centerserver[JSON_ALARM_URI].asString(),
                 plateresult_settings_.uri,
                 MAX_URI_LEN);
    plateresult_settings_.plateResultLevel = centerserver[JSON_PLATERESULT_ENABLE].asInt();
    plateresult_settings_.is_send_image = centerserver[JSON_IS_SEND_IMAGE].asInt();
    plateresult_settings_.is_send_small_image = centerserver[JSON_IS_SEND_SMALLIMAGE].asInt();
    memcpy(buffer,
           &plateresult_settings_,
           bufferlen);
    break;
  case CENTERSERVER_GIOINALARM:
    gpio_settings_.enable = centerserver[JSON_GIO_ENABLE].asInt();
    StringToChar(centerserver[JSON_GIO_URI].asString(),
                 gpio_settings_.uri, MAX_URI_LEN);
    memcpy(buffer,
           &gpio_settings_,
           bufferlen);
    break;
  case CENTERSERVER_SERIAL:
    serial_settings_.enable = centerserver[JSON_SERIAL_ENABLE].asInt();
    StringToChar(centerserver[JSON_SERIAL_URI].asString(),
                 serial_settings_.uri, MAX_URI_LEN);
    memcpy(buffer,
           &serial_settings_,
           bufferlen);
    break;
  default:
    break;
  }
  return (KVDB_RET_SUCCEED == Kvdb_SetKey(p_key, n_key, buffer, bufferlen));
}

int SenderServer::iReadJsonFile(const char* file_path, Json::Value &value) {

  Json::Reader reader;
  std::ifstream is;
  is.open(file_path, std::ios::binary);
  if (!is.is_open()) {
    return -1;
  }
  if (!reader.parse(is, value))return -1;
  return 0;
}

size_t SenderServer::strlcpy(char *dst, const char *src, size_t siz) {
  register char *d = dst;
  register const char *s = src;
  register size_t n = siz;

  /* Copy as many bytes as will fit */
  if (n != 0 && --n != 0) {
    do {
      if ((*d++ = *s++) == 0)
        break;
    } while (--n != 0);
  }

  /* Not enough room in dst, add NUL and traverse rest of src */
  if (n == 0) {
    if (siz != 0)
      *d = '\0';		/* NUL-terminate dst */
    while (*s++)
      ;
  }

  return(s - src - 1);	/* count does not include NUL */
}
void SenderServer::StringToChar(const std::string &res_str, char *res, int len) {

  strlcpy(res, res_str.c_str(), len);
}

void SenderServer::HttpWriteRequestLogging(HTTP_MSGTYPE msgtype,
    std::string data) {
  if (data.length() > 256)
    return;
  char sendmsg[512] = {0};
  switch (msgtype) {
  case HTTP_IVS_RESULT:
    sprintf(sendmsg,
            "Send vehicle license plate to HTTP center server,address info: %s,"
            "PlateResult : %s", m_url, data.c_str());
    break;
  case HTTP_SERIAL_DATA:
    sprintf(sendmsg,
            "Send SerialData to HTTP center server,address info: %s ,"
            "SerialData : %s", m_url, data.c_str());
    break;
  case HTTP_GIO_TRIGGER:
    break;
  case HTTP_DEVICE_REG :
    sprintf(sendmsg, "Register to HTTP center server,address info: %s",m_url);
    break;
  default:
    break;
  }
  HttpDpLogging(sendmsg, strlen(sendmsg));
}

void SenderServer::GetSendUrl(char *hostname, char *uri) {
  m_port = server_settings_.port;
  if (hostname == NULL || uri == NULL)
    return;
  if (strlen(hostname) + strlen(uri) > 1000)
    return;
  if (server_settings_.enable_ssl) {
    sprintf(m_url, "https://%s%s", hostname, uri);
    m_port = server_settings_.ssl_port;
  } else {
    sprintf(m_url, "%s%s", hostname, uri);
  }
}

void SenderServer::NewIPCConn(Json::Value &value,
                              int          resendtimes,
                              int          timeout,
                              int          n_cb_type) {

  LOG(L_INFO) << "NewIPCConn";
  std::string postdata = value.toStyledString();

  HttpPostData(m_url,
               postdata,
               server_settings_.enable_ssl,
               m_port,
               timeout,
               resendtimes,
               n_cb_type);
}

void SenderServer::NewIPCConnExt(HTTP_MSGTYPE msgtype,
                                 Json::Value &value,
                                 std::string  data,
                                 std::string  uri,
                                 int          n_cb_type) {
  Json::Value ipextvalues = ip_extjs_[JSON_IP_EXT];
  for (int i = 0; i < ipextvalues.size(); i++) {
    GetSendUrl((char*)(ipextvalues[i][JSON_IP_EXT_IN].asString()).c_str(),
               (char*)uri.c_str());
    HttpWriteRequestLogging(msgtype, data);
    NewIPCConn(value,
               m_HttpResendTimes,
               server_settings_.http_timeout,
               n_cb_type);
  }
}

void SenderServer::HttpPostData(const std::string url,
                                const std::string postdata,
                                bool              ssl_enable,
                                int               port,
                                int               timeout,
                                int               resendtimes,
                                int               n_cb_type) {

  HttpConn *p_conn = curl_services_->CreateHttpConn(
                       url,
                       postdata,
                       server_settings_.enable_ssl == 1,
                       m_port,
                       timeout,
                       resendtimes,
                       n_cb_type,
                       this);
  if (p_conn == NULL)
    return;

  curl_services_->PostData(p_conn);
}

};
