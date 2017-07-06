#include "httpsender/server/senderserver.h"
#include "ipc_comm/SystemServerComm.h"
#include "ipc_comm/dp_logging_comm.h"
#include "ipc_comm/HttpSenderComm.h"
#include "vzbase/base/base64.h"

#include "stdlib.h"
#include "stdio.h"
#include "string.h"

#include <string>
#include <fstream>
#include <map>

#ifdef WIN32
#include <windows.h>
#endif

namespace hs {

const unsigned REQ_TIMEOUT = 3 ; //请求超时时间（秒）
const unsigned REQ_WAIT_GAP = 10 ;   //请求后等待时的查询间隔（毫秒）

enum {
  REQ_MSG_ERR = -2,
  REQ_MSG_TIMEOUT = -1,
  REQ_MSG_INIT = 0,
  REQ_MSG_WAIT,
  REQ_MSG_OK,
};

#define TRUE 1
#define FALSE 0

SenderServer::SenderServer(io_service_ptr io_service)
  : m_nReqMsgStatus(REQ_MSG_INIT)
  , m_pBuf4Reply(NULL)
  , m_nSizeBuf4Reply(0)
  , m_nSizeData4Reply(0)
  , io_service_(io_service)
  , timer_(*io_service_)
  , watchdog_timer_(*io_service_)
  , kvdb_client_(new kvdb::KvdbClient())
  , curl_services_(new hs::CurlServices(*io_service))
  , data_parse_(new hs::DataParse())
  , watch_dog_(new hs::WatchDog())
  , resend_timer_(*io_service_) {
  m_strLastReqMsg[0] = '\0';
  ConstInit();
}

SenderServer::~SenderServer(void) {

  dp_client_->Stop();
  file_cached_.UinitCachedClient();
  kvdb_client_->UinitKvdbClient();
  curl_services_->UninitCurlServices();
#ifndef WIN32
  // CleanupFileMsgDrv();
#endif
}

void SenderServer::InitWatchDog() {

  if (watch_dog_->Init() == -1) {
    LOG(L_ERROR) << "Init WatchDog Failure";
    return;
  }

#ifdef WIN32
  watchdog_timer_.expires_from_now(boost::posix_time::seconds(WATCHDOG_TIME));
#else
  watchdog_timer_.expires_from_now(boost::chrono::seconds(WATCHDOG_TIME));
#endif
  watchdog_timer_.async_wait(boost::bind(&SenderServer::iOnWatchDogTimerCB,
                                         shared_from_this(),
                                         boost::asio::placeholders::error));
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

  if(!data_parse_->JsonReadSysInfoValue(system_json_)) {
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

void SenderServer::iOnWatchDogTimerCB(const boost::system::error_code& err) {

  if (err) {
    LOG(L_ERROR) << err.message();
    return;
  }
  watch_dog_->WatchDogFeed();
#ifdef WIN32
  watchdog_timer_.expires_from_now(boost::posix_time::seconds(WATCHDOG_TIME));
#else
  watchdog_timer_.expires_from_now(boost::chrono::seconds(WATCHDOG_TIME));
#endif
  watchdog_timer_.async_wait(boost::bind(&SenderServer::iOnWatchDogTimerCB,
                                         shared_from_this(),
                                         boost::asio::placeholders::error));
}

bool SenderServer::Start(void) {

  LOG(L_INFO) << "InitWatchDog";
  InitWatchDog();
  LOG(L_INFO) << "InitSysInfo";
  InitSysInfo();
  LOG(L_INFO) << "try";
  bool res = true;
  try {
    // 1 file cached
#ifndef WIN32
    if (!file_cached_.InitCachedClient(FILECACHED_IP_ADDR,
                                       FILECACHED_PORT)) {
      LOG(L_ERROR) << "Init the cached file failure";
      return false;
    }
#endif
    LOG(L_INFO) << "InitKvdbClient";
    // kvdb_client
    BOOST_ASSERT(kvdb_client_);
    res = kvdb_client_->InitKvdbClient(KVDB_SERVER_IP_ADDR, KVDB_SERVER_PORT);
    if (!res) {
      LOG(L_ERROR) << "Failure to init keyvalue Client";
      return false;
    }
    LOG(L_INFO) << "InitHttpDataInfo";
    InitHttpDataInfo();
    // curl_services
    BOOST_ASSERT(curl_services_);
    LOG(L_INFO) << "InitCurlServices";
    res = curl_services_->InitCurlServices();
    if (!res) {
      LOG(L_ERROR) << "Failure to init CurlServices";
      return false;
    }
//#endif
    // dp client
    LOG(L_INFO) << "CreateDpClient";
    dp_client_ = vznetdp::DpClient::CreateDPClient(*io_service_,
                 DP_SERVER_IP_ADDR,
                 DP_SERVER_PORT,
                 vzconn::PerfectBufferPool::Instance());
    if (!dp_client_) {
      LOG(L_ERROR) << "Failure to create dp client";
      return false;
    }
    LOG(L_INFO) << "SignalConnectSucceed";
    dp_client_->SignalConnectSucceed.connect(
      boost::bind(&SenderServer::OnDpConnectSucceed, shared_from_this(), _1));
    dp_client_->SignalDispatcherMessage.connect(
      boost::bind(&SenderServer::OnDpMessage, shared_from_this(), _1, _2));
    dp_client_->SignalErrorEvent.connect(
      boost::bind(&SenderServer::OnDpErrorEvent, shared_from_this(), _1, _2));
    LOG(L_INFO) << "Connecting the DpServer";
    LOG(L_INFO) << "Start";
    dp_client_->Start();
    LOG(L_INFO) << "End";
  } catch (std::exception &e) {
    LOG(L_ERROR) << e.what();
    return false;
  }
  LOG(L_INFO) << "SenderServer Start";
  return true;
}

void SenderServer::ConstInit() {

  memcpy(server_settings_.hostname, DEFAULT_HOST_NAME,
         sizeof(DEFAULT_HOST_NAME));
  server_settings_.port         = DEFAULT_NET_HTTP_PORT;
  server_settings_.enable_ssl   = 0;
  server_settings_.ssl_port     = 443;
  server_settings_.http_timeout = 5;
  //
  device_reg_settings_.enable   = 1;
  memcpy(device_reg_settings_.uri, DEFAULT_REG_URI, sizeof(DEFAULT_REG_URI));
  //
  plateresult_settings_.enable  = 0;
  memcpy(plateresult_settings_.uri,
         DEFAULT_PLATE_POST_URI,
         sizeof(DEFAULT_PLATE_POST_URI));
  plateresult_settings_.plateResultLevel      = 0;
  plateresult_settings_.is_send_image         = 0;
  plateresult_settings_.is_send_small_image   = 0;
  //
  gpio_settings_.enable        = 0;
  memcpy(gpio_settings_.uri,
         DEFAULT_GPIO_POST_URL,
         sizeof(DEFAULT_GPIO_POST_URL));
  //
  serial_settings_.enable      = 0;
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

//请求处理前，先保存当前状态和一些变量
bool SenderServer::iSetLastReqMsg(const char *pMsg,
                                  void *pBuf4Reply,
                                  int nSizeBuf4Reply) {
  if (pMsg == NULL || strlen(pMsg) > 1024) {
    return(false);
  }
  //暂时只能每次做一次请求，如果上次请求还未返回，那么这次请求将不能成功；
  if (m_nReqMsgStatus != REQ_MSG_INIT) {
    return(false);
  }
  m_nReqMsgStatus = REQ_MSG_WAIT;
  strcpy(m_strLastReqMsg, pMsg);
  m_pBuf4Reply = pBuf4Reply;
  m_nSizeBuf4Reply = nSizeBuf4Reply;
  m_nSizeData4Reply = 0;
  return(true);
}

//请求处理完后的清除状态
void SenderServer::iCleanLastReqMsg() {
  m_strLastReqMsg[0] = '\0';
  m_nReqMsgStatus = REQ_MSG_INIT;
  m_pBuf4Reply = NULL;
  m_nSizeBuf4Reply = 0;
}

//轮询反馈
bool SenderServer::iCheckAndHandleReqMsg() {
  //等待
  while (1) {
    if (m_nReqMsgStatus != REQ_MSG_WAIT)
      break;
#ifdef WIN32
    Sleep(REQ_WAIT_GAP);
#else
    usleep(REQ_WAIT_GAP * 1000);
#endif
  }
#if 0
  if () {
    _DBG("Reply Timeout 4 Request(%s)\n", m_strLastReqMsg);
    m_nReqMsgStatus = REQ_MSG_TIMEOUT;
    return(false);
  }
#endif

  return(m_nReqMsgStatus == REQ_MSG_OK);
}

//消息分发线程的调用
void SenderServer::iOnReply(const DpMessage *pDmp, void *pUserData) {
  //boa_debug("iOnReply: %s\n",pDmp->method);
  if (pDmp->type == TYPE_ERROR_TIMEOUT) {

  }
  SenderServer *pInstance = (SenderServer *)pUserData;

  //必须为当前请求消息的反馈
  if (strcmp(pDmp->method, pInstance->m_strLastReqMsg) != 0) {
    return;
  }

  //当前请求消息的状态必须正确
  if (pInstance->m_nReqMsgStatus != REQ_MSG_WAIT) {
    return;
  }

  bool bOK = true;
  //复制反馈数据
  if (pInstance->m_pBuf4Reply && pInstance->m_nSizeBuf4Reply > 0
      && pDmp->data_size > 0) {
    if (pDmp->data_size > (unsigned)pInstance->m_nSizeBuf4Reply) {
      bOK = false;
    } else {
      memcpy(pInstance->m_pBuf4Reply, pDmp->data, pDmp->data_size);
      pInstance->m_nSizeData4Reply = pDmp->data_size;
    }
  }

  pInstance->m_nReqMsgStatus = bOK ? REQ_MSG_OK : REQ_MSG_ERR;
}

const char *DPMsg[] = {

  EVT_SRV_IVS_RESULT,
  EVT_SRV_TT_TCP,
  EVT_SRV_EXTERNAL_TRIGGER,
  SYS_SRV_DNS_CHANGED,
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
  DP_LOGGING_ENABLE,
  SYS_SRV_IP_CHANGED
};

#define HTTP_CMD_SIZE sizeof(DPMsg)/sizeof(char *)

void SenderServer::HttpConnSnapImageComplete(hs::HttpConn::Ptr http_conn,
    const char *data,
    std::size_t data_size,
    const boost::system::error_code& err,
    hs::CurlServices::Ptr curl_services) {
  if (http_conn.get() == NULL)
    return;
}

void SenderServer::OnGetSnapShotImage(vznetdp::DpClient::Ptr client,
                                      const DpMessage* dmp,
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
  data_parse_->GetSnapImage(imagepath.c_str(),snap_image_js,file_cached_);
  std::string postdata = snap_image_js.toStyledString();
  LOG(L_INFO) << "dmp->data : " << dmp->data;
  LOG(L_INFO) << "dmp->data_size : " << dmp->data_size;
  LOG(L_INFO) << "image_size : " << postdata.length();
  HttpConn::Ptr http_conn = curl_services_->CreateHttpConn(
                              snap_image_url,
                              postdata,
                              server_settings_.enable_ssl == 1,
                              port,
                              timeout,
                              0);
  if (http_conn.get() == NULL)
    return;
  http_conn->SignalComplete.connect(
    boost::bind(&SenderServer::HttpConnSnapImageComplete,
                this, _1, _2, _3, _4, curl_services_));
  curl_services_->PostData(http_conn);
}

bool SenderServer::OnIvsPlateResponseHandle(const char* data) {

  LOG(L_INFO) << "OnResponseHandle... ... : " << data;
  if (data == NULL)
    return false;
  LOG(L_INFO) << data;
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
    dp_client_->SendDpMessage(TCP_SRV_GPIO_IOCTL, 0,
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
      dp_client_->SendDpRequest(
        AVS_GET_SNAP, 0, NULL, 0,
        boost::bind(
          &SenderServer::OnGetSnapShotImage,
          this, _1, _2,
          url,
          trigger_port,dp_end),
        3);
    } else if(!triggerimage[JSON_SNAP_IMAGE_RELATIVE_URL].isNull()) {
      bool dp_end = false;
      GetSendUrl(
        server_settings_.hostname,
        (char *)triggerimage[JSON_SNAP_IMAGE_RELATIVE_URL].asString().c_str());
      std::string url(m_url);
      dp_client_->SendDpRequest(
        AVS_GET_SNAP, 0, NULL, 0,
        boost::bind(
          &SenderServer::OnGetSnapShotImage,
          this, _1, _2,
          url,trigger_port,dp_end), 3);
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
      dp_client_->SendDpMessage(EVT_SRV_EXTERNAL_TRIGGER, 0,
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

  dp_client_->SendDpMessage(TCP_SRV_TT_RS485, 0,
                            m_serialbuffer, sizeof(TT_Param) + datalen);
  //delete[] buffer;
}
#ifdef WIN32
void SenderServer::ResendIVSResult(hs::HttpConn::Ptr http_conn,
                                   boost::shared_ptr<boost::asio::deadline_timer> t,
                                   const boost::system::error_code& err) {
#else
void SenderServer::ResendIVSResult(hs::HttpConn::Ptr http_conn,
                                   boost::shared_ptr<boost::asio::steady_timer> t,
                                   const boost::system::error_code& err) {
#endif

  if (http_conn.get() == NULL)
    return;

  if (err) {
    LOG(L_ERROR) << err.message();
  }

  int resend_times = http_conn->resendtimes();
  if (resend_times <= 0)
    return;
  HttpPostData(http_conn->url(),
               http_conn->post_data(),
               http_conn->ssl_enable(),
               http_conn->port(),
               http_conn->timeout(),
               --resend_times,
               boost::bind(&SenderServer::HttpConnIvsResultComplete,
                           this, _1, _2, _3, _4, curl_services_));
  LOG(L_INFO) << "resend_times : " << " : " << resend_times;
}



void SenderServer::HttpConnIvsResultComplete(hs::HttpConn::Ptr http_conn,
    const char *data,
    std::size_t data_size,
    const boost::system::error_code& err,
    hs::CurlServices::Ptr curl_services) {
  LOG(L_INFO) << "HttpConnIvsResultComplete";
  if (http_conn.get() == NULL) {
    return;
    LOG(L_INFO) << "http_conn.get() == NULL";
  }
  if (OnIvsPlateResponseHandle(data)) {
    HttpWriteResponseLogging(http_conn->url(), data);
  }
  if (err) {
    if (m_nOfflineCheckStatus) {
      check_info_.bActive = 0;
      dp_client_->SendDpMessage(TCP_SRV_SET_OFFLINE_CHECK,
                                0, (char*)&check_info_,
                                sizeof(TCP_SRV_OFFLINE_CHECK_INFO));
    }
    HttpDpLogging(err.message().c_str(), err.message().length());
    if (http_conn->resendtimes() > 0) {
#ifdef WIN32
      boost::shared_ptr<boost::asio::deadline_timer> t(
        new boost::asio::deadline_timer(*io_service_));
      t->expires_from_now(boost::posix_time::seconds(http_conn->timeout()));
      //boost::asio::deadline_timer t(*io_service_);
#else
      boost::shared_ptr<boost::asio::steady_timer> t(
        new boost::asio::steady_timer(*io_service_));
      t->expires_from_now(boost::chrono::seconds(http_conn->timeout()));
#endif
      t->async_wait(boost::bind(&SenderServer::ResendIVSResult,
                                shared_from_this(), http_conn, t,
                                boost::asio::placeholders::error
                               ));
    }
    LOG(L_INFO) << http_conn->url() << " : " << err.message();
    return;
  }



  if (m_nOfflineCheckStatus) {
    dp_client_->SendDpMessage(TCP_SRV_OFFLINE_CHECK_RESP,
                              0, NULL, 0);
    check_info_.bActive = 1;
    dp_client_->SendDpMessage(TCP_SRV_SET_OFFLINE_CHECK,
                              0, (char*)&check_info_,
                              sizeof(TCP_SRV_OFFLINE_CHECK_INFO));
    //脱机检查心跳包
  }
  LOG(L_INFO).write(data, data_size);
}

void SenderServer::HttpConnGioTriggerComplete(hs::HttpConn::Ptr http_conn,
    const char *data,
    std::size_t data_size,
    const boost::system::error_code& err,
    hs::CurlServices::Ptr curl_services) {

  if (http_conn.get() == NULL)
    return;
  HttpWriteResponseLogging(http_conn->url(), data);
  if (err) {
    HttpDpLogging(err.message().c_str(), err.message().length());
    int resend_times = http_conn->resendtimes();
    if (resend_times > 0) {

      HttpPostData(http_conn->url(),
                   http_conn->post_data(),
                   http_conn->ssl_enable(),
                   http_conn->port(),
                   http_conn->timeout(),
                   --resend_times,
                   boost::bind(&SenderServer::HttpConnGioTriggerComplete,
                               this, _1, _2, _3, _4, curl_services_));
    }
    LOG(L_ERROR) << err.message();
    return;
  }
  LOG(L_INFO).write(data, data_size);
}

void SenderServer::HttpConnSerialDataComplete(hs::HttpConn::Ptr http_conn,
    const char *data,
    std::size_t data_size,
    const boost::system::error_code& err,
    hs::CurlServices::Ptr curl_services) {

  if (http_conn.get() == NULL)
    return;
  HttpWriteResponseLogging(http_conn->url(), data);
  if (err) {
    HttpDpLogging(err.message().c_str(), err.message().length());
    int resend_times = http_conn->resendtimes();
    if (resend_times > 0) {

      HttpPostData(http_conn->url(),
                   http_conn->post_data(),
                   http_conn->ssl_enable(),
                   http_conn->port(),
                   http_conn->timeout(),
                   --resend_times,
                   boost::bind(&SenderServer::HttpConnSerialDataComplete,
                               this, _1, _2, _3, _4, curl_services_));
    }
    LOG(L_ERROR) << err.message();
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
  if(!kvdb_client_->GetKey(Acount_Info, &user)) return "admin";
  reader.parse(user,user_js);
  return user_js[0]["user"].asString();
}

std::string SenderServer::GetSystemInfoAccount0Password() {
  Json::Reader reader;
  Json::Value pass_js;
  std::string pass;
  if(!kvdb_client_->GetKey(Acount_Info, &pass)) return "admin";
  reader.parse(pass,pass_js);
  return pass_js[0]["password"].asString();
}

std::string SenderServer::GetSystemInfoTitle() {
  Json::Reader reader;
  Json::Value title_js;
  std::string title;
  if(!kvdb_client_->GetKey(Sys_Title, &title,true)) {
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
  if(!kvdb_client_->GetKey(NetworkInterface_Cfg, &ip)) return "192.168.1.100";
  reader.parse(ip,ip_js);
  return ip_js["ip"].asString();
}
void SenderServer::DeviceRegFunc(const boost::system::error_code& err) {

  if (err) {
    LOG(L_ERROR) << err.message();
    return;
  }
  if (REG_CANCEL_HEARTBEAT != device_reg_settings_.enable) {
    Json::Value deviceregdata;

#ifndef WIN32
    std::string title = GetSystemInfoTitle();
    std::string user = GetSystemInfoAccount0User();
    std::string pass = GetSystemInfoAccount0Password();
    data_parse_->GetDeviceRegJson(deviceregdata,
                                  (char *)title.c_str(),
                                  (char *)user.c_str(),
                                  (char *)pass.c_str(),
                                  m_httpport);
#else
    data_parse_->GetDeviceRegJson(deviceregdata,
                                  "IVS",
                                  "admin",
                                  "admin",
                                  "80");
#endif
    GetSendUrl(server_settings_.hostname, device_reg_settings_.uri);
    HttpWriteRequestLogging(HTTP_DEVICE_REG, "");

    NewIPCDevRegConn(deviceregdata);
  }
}


void SenderServer::NewIPCFtpPostConn(char *filepath) {
  Json::Value ftpimgdata;
  UserGetImgInfo imginfo;
  std::string title = GetSystemInfoTitle();
  data_parse_->GetFtpPostJson(
    imginfo,
    ftpimgdata,
    (char*)ftp_json_[SERVIER_IP].asString().c_str(),
    (char*)ftp_json_[FOLDER_NAME].asString().c_str(),
    filepath,
    title,
    file_cached_);
  imginfo.curpose = 0;
  imginfo.posturl = ftpimgdata[JSON_FTP_URL].asString();
  //memcpy(imginfo.pdata,
  //  ftpimgdata[JSON_FTP_FILE_DATA].asString().c_str(),
  //  ftpimgdata[JSON_FTP_FILE_SIZE].asInt());
  //StringToChar(ftpimgdata[JSON_FTP_FILE_DATA].asString(),
  //             imginfo.pdata,
  //             ftpimgdata[JSON_FTP_FILE_SIZE].asInt());
  HttpConn::Ptr http_conn = curl_services_->CreateHttpConn(
                              imginfo.posturl,
                              "ftp_post",
                              false,
                              ftp_json_[PORT_STR].asInt(),
                              10,
                              0);
  if (http_conn.get() == NULL)
    return;
  http_conn->SignalComplete.connect(
    boost::bind(&SenderServer::FtpConnPostFileComplete,
                this, _1, _2, _3, _4, curl_services_));

  curl_services_->PostImageFile(
    http_conn,
    &imginfo,
    ftp_json_[USER_NAME].asString(),
    ftp_json_[PASSWORD].asString());
}

void SenderServer::NewIPCDevRegConn(Json::Value devicereg) {

  DeviceRegData regdata;
  regdata.device_name = devicereg[JSON_DEVICE_NAME].asString();
  regdata.ipaddr = devicereg[JSON_IP_ADDRESS].asString();
  regdata.port = devicereg[JSON_PORT].asString();
  regdata.user_name = devicereg[JSON_USERNAME].asString();
  regdata.pass_wd = devicereg[JSON_PASSWORD].asString();
  regdata.serialno = devicereg[JSON_SERIALNO].asString();
  regdata.channel_num = devicereg[JSON_CHANNEL_NUM].asString();

  int timeout = device_reg_settings_.enable == REG_COMET_POLLING ?
                COMET_CONNTIME : server_settings_.http_timeout;

  HttpConn::Ptr http_conn = curl_services_->CreateHttpConn(
                              m_url,
                              "devecireg",
                              server_settings_.enable_ssl == 1,
                              m_port,
                              timeout,
                              0);

  if (http_conn.get() == NULL)
    return;
  http_conn->SignalComplete.connect(
    boost::bind(&SenderServer::HttpConnDeviceRegComplete,
                this, _1, _2, _3, _4, curl_services_));

  curl_services_->PostDevRegData(http_conn, regdata);
}

void SenderServer::HttpConnDeviceRegComplete(hs::HttpConn::Ptr http_conn,
    const char *data,
    std::size_t data_size,
    const boost::system::error_code& err,
    hs::CurlServices::Ptr curl_services) {
  //LOG(L_INFO).write(data, data_size);

  if (http_conn.get() == NULL)
    return;
  if (REG_COMMON_HEARTBEAT == device_reg_settings_.enable) {
    int time_out = COMET_CONNTIME;
    if(err && m_nOfflineCheckStatus) {
      time_out = 1;
      check_info_.bActive = 0;
      dp_client_->SendDpMessage(TCP_SRV_SET_OFFLINE_CHECK,
                                0, (char*)&check_info_,
                                sizeof(TCP_SRV_OFFLINE_CHECK_INFO));
    } else if(m_nOfflineCheckStatus) {
      check_info_.bActive = 1;
      dp_client_->SendDpMessage(TCP_SRV_SET_OFFLINE_CHECK,
                                0, (char*)&check_info_,
                                sizeof(TCP_SRV_OFFLINE_CHECK_INFO));
    }
#ifdef WIN32
    timer_.expires_from_now(boost::posix_time::seconds(time_out));
#else
    timer_.expires_from_now(boost::chrono::seconds(time_out));
#endif
    timer_.async_wait(boost::bind(&SenderServer::DeviceRegFunc,
                                  shared_from_this(),
                                  boost::asio::placeholders::error));

  } else if (REG_COMET_POLLING == device_reg_settings_.enable) {

    if (err) {
#ifdef WIN32
      timer_.expires_from_now(boost::posix_time::seconds(2));
#else
      timer_.expires_from_now(boost::chrono::seconds(2));
#endif
      timer_.async_wait(boost::bind(&SenderServer::DeviceRegFunc,
                                    shared_from_this(),
                                    boost::asio::placeholders::error));
    } else {
      if (OnIvsPlateResponseHandle(data)) {

        HttpWriteResponseLogging(http_conn->url(), data);
      }
      boost::system::error_code errcode;
      DeviceRegFunc(errcode);
    }
  }
  if (err) {
    HttpDpLogging(err.message().c_str(), err.message().length());
    m_HttpServerPollLineStatus = false;//(err.value() == 7 ? false : true);
    LOG(L_ERROR) << http_conn->url() << " : " << err.message();
    return;
  }
  m_HttpServerPollLineStatus = true;
}

void SenderServer::FtpConnPostFileComplete(hs::HttpConn::Ptr http_conn,
    const char *data,
    std::size_t data_size,
    const boost::system::error_code& err,
    hs::CurlServices::Ptr curl_services) {
  if(err) {
    LOG(L_ERROR) << http_conn->url() << " : " << err.message();
  } else {
    LOG(L_INFO) << http_conn->url() << " : " << "FtpConnPostFileComplete";
  }
}

void SenderServer::OnIvsResult(vznetdp::DpClient::Ptr client,
                               const DpMessage* dmp) {
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
  data_parse_->GetPlateJson((IVS_RESULT_PARAM *)(dmp->data),
                            platedata,
                            &plateresult_settings_,
                            (char *)title.c_str(),
                            file_cached_);
#else
  data_parse_->GetPlateJson((IVS_RESULT_PARAM *)(dmp->data),
                            platedata,
                            &plateresult_settings_,
                            "IVS",
                            file_cached_);
#endif
  GetSendUrl(server_settings_.hostname, plateresult_settings_.uri);
  HttpWriteRequestLogging(HTTP_IVS_RESULT,
                          platedata[JSON_ALARMINFO_PLATE][JSON_RESULT][JSON_PLATE_RESULT][JSON_LICENSE].asString());


  if (m_HttpServerPoll == 0) {
    NewIPCConn(platedata, m_HttpResendTimes, server_settings_.http_timeout,
               boost::bind(&SenderServer::HttpConnIvsResultComplete,
                           this, _1, _2, _3, _4, curl_services_));
    NewIPCConnExt(HTTP_IVS_RESULT, platedata,
                  platedata[JSON_LICENSE].asString(), plateresult_settings_.uri,
                  boost::bind(&SenderServer::HttpConnIvsResultComplete,
                              this, _1, _2, _3, _4, curl_services_));
  } else {

    if (ip_extjs_[JSON_IP_EXT].type() == Json::nullValue ||
        m_HttpServerPollLineStatus) {
      NewIPCConn(platedata, m_HttpResendTimes, server_settings_.http_timeout,
                 boost::bind(&SenderServer::HttpConnIvsResultComplete,
                             this, _1, _2, _3, _4, curl_services_));
    } else {
      NewIPCConnExt(HTTP_IVS_RESULT, platedata,
                    platedata[JSON_LICENSE].asString(), plateresult_settings_.uri,
                    boost::bind(&SenderServer::HttpConnIvsResultComplete,
                                this, _1, _2, _3, _4, curl_services_));
    }
  }
}

void SenderServer::NewIPCConnExt(HTTP_MSGTYPE msgtype, Json::Value &value,
                                 std::string data, std::string uri,
                                 CallbackNewConnMessage call_back) {

  Json::Value ipextvalues = ip_extjs_[JSON_IP_EXT];
  for (int i = 0; i < ipextvalues.size(); i++) {
    GetSendUrl((char*)(ipextvalues[i][JSON_IP_EXT_IN].asString()).c_str(),
               (char*)uri.c_str());
    HttpWriteRequestLogging(msgtype, data);
    NewIPCConn(value, m_HttpResendTimes,
               server_settings_.http_timeout,
               call_back);
  }
}

void SenderServer::OnSerial(vznetdp::DpClient::Ptr client,
                            const DpMessage* dmp) {

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
  data_parse_->GetRs485Json(serialdata,
                            data.source,
                            m_serialdata,
                            dmp->data_size - sizeof(TT_Param));
  HttpWriteRequestLogging(HTTP_SERIAL_DATA, m_serialdata);

  if (m_HttpServerPoll == 0) {
    NewIPCConn(serialdata, m_HttpResendTimes, server_settings_.http_timeout,
               boost::bind(&SenderServer::HttpConnSerialDataComplete,
                           this, _1, _2, _3, _4, curl_services_));
    NewIPCConnExt(HTTP_SERIAL_DATA, serialdata, m_serialdata, serial_settings_.uri,
                  boost::bind(&SenderServer::HttpConnSerialDataComplete,
                              this, _1, _2, _3, _4, curl_services_));
  } else {

    if (ip_extjs_[JSON_IP_EXT].type() == Json::nullValue ||
        m_HttpServerPollLineStatus) {
      NewIPCConn(serialdata, m_HttpResendTimes, server_settings_.http_timeout,
                 boost::bind(&SenderServer::HttpConnSerialDataComplete,
                             this, _1, _2, _3, _4, curl_services_));
    } else {
      NewIPCConnExt(HTTP_SERIAL_DATA, serialdata, m_serialdata, serial_settings_.uri,
                    boost::bind(&SenderServer::HttpConnSerialDataComplete,
                                this, _1, _2, _3, _4, curl_services_));
    }
  }
}

void SenderServer::OnGioTrigger(vznetdp::DpClient::Ptr client,
                                const DpMessage* dmp) {
  if (dmp->data_size != sizeof(TRIGGER_REULST))return;
  if (!gpio_settings_.enable) {

    LOG(L_WARNING) << "gpio_settings_ is disabled !!!";
    return;
  }
  TRIGGER_REULST *trigger_result = (TRIGGER_REULST *)(dmp->data);
  Json::Value giodata;
#ifndef WIN32
  std::string title = GetSystemInfoTitle();
  data_parse_->GetGIOJson(giodata, (TRIGGER_REULST *)(dmp->data),
                          (char *)title.c_str());
#else
  data_parse_->GetGIOJson(giodata, (TRIGGER_REULST *)(dmp->data),
                          "IVS");
#endif
  GetSendUrl(server_settings_.hostname,
             gpio_settings_.uri);

  if (m_HttpServerPoll == 0) {
    NewIPCConn(giodata, m_HttpResendTimes, server_settings_.http_timeout,
               boost::bind(&SenderServer::HttpConnGioTriggerComplete,
                           this, _1, _2, _3, _4, curl_services_));
    NewIPCConnExt(HTTP_GIO_TRIGGER, giodata, "", gpio_settings_.uri,
                  boost::bind(&SenderServer::HttpConnGioTriggerComplete,
                              this, _1, _2, _3, _4, curl_services_));
  } else {

    if (ip_extjs_[JSON_IP_EXT].type() == Json::nullValue ||
        m_HttpServerPollLineStatus) {
      NewIPCConn(giodata, m_HttpResendTimes, server_settings_.http_timeout,
                 boost::bind(&SenderServer::HttpConnGioTriggerComplete,
                             this, _1, _2, _3, _4, curl_services_));
    } else {
      NewIPCConnExt(HTTP_GIO_TRIGGER, giodata, "", gpio_settings_.uri,
                    boost::bind(&SenderServer::HttpConnGioTriggerComplete,
                                this, _1, _2, _3, _4, curl_services_));
    }
  }
}

void SenderServer::OnPostEvent(hs::HttpConn::Ptr http_conn,
                               const char *data,
                               std::size_t data_size,
                               const boost::system::error_code& err) {
  if(err) {
    LOG(L_ERROR) << err.message();
    return;
  }
  // process the server result event
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
  return dp_client_->SendDpMessage("OPTIMIZE_LOGGING",
                                   0,
                                   buffer,
                                   sizeof(OptimizeLogging) + msg_size);
}

void SenderServer::HttpDpLogging(const char *logging_msg,
                                 unsigned char msg_size) {

  if (m_LoggingEnable)
    WriteDpLoggingMessage(
      ID_HTTP_SEND,
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
  if (kvdb_client_->SetKey(http_ipext_key,
                           strlen(http_ipext_key),
                           m_ipext,
                           MAX_IP_EXT_LEN)) {
    return KVDB_TRUE;
  }
  return KVDB_FALSE;
}

int SenderServer::SetCenterServerNet(const char* net) {
  memcpy(&server_settings_, net, sizeof(VZ_CenterServer_Net));
  if(kvdb_client_->SetKey(http_centerservernet_value,
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
    boost::system::error_code err;
    DeviceRegFunc(err);
  }
  if (kvdb_client_->SetKey(http_centerserverreg_value,
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
  if (kvdb_client_->SetKey(http_centerserveralarm_value,
                           strlen(http_centerserveralarm_value),
                           plateresult, sizeof(VZ_CenterServer_PlateResult))) {
    return KVDB_TRUE;
  }
  return KVDB_FALSE;
}

int SenderServer::SetCenterServerGio(const char* gio) {
  memcpy(&gpio_settings_, gio, sizeof(VZ_CenterServer_GioInAlarm));
  if (kvdb_client_->SetKey(http_centerserverio_value,
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
  if (kvdb_client_->SetKey(http_centerserverserial_value,
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
  if (kvdb_client_->SetKey(http_resendtimes_value,
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
  if (kvdb_client_->SetKey(http_serverpoll_value,
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
    dp_client_->SendDpMessage(TCP_SRV_SET_OFFLINE_CHECK,
                              0, (char*)&check_info_,
                              sizeof(TCP_SRV_OFFLINE_CHECK_INFO));
  }
  m_nOfflineCheckStatus = newstatus;

  if (kvdb_client_->SetKey(http_offlinecheckstatus_value,
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
  kvdb_client_->SetKey(http_email_config_value,
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

  kvdb_client_->SetKey(http_ftp_config_value,
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

void SenderServer::OnConfigControl(vznetdp::DpClient::Ptr client,
                                   const DpMessage* dmp) {
  LOG(L_INFO) << "Message : " << dmp->method;
  int   ret = -1;
  int   res_size = 0;
  void  *res_data = NULL;
  std::string replystr;
  LOG(L_INFO) << dmp->method;
  if (strcmp(dmp->method, HTTPSENDER_SET_HTTP_IP_EXT) == 0) {
    ret = SetIpExt(dmp->data, dmp->data_size);
    res_data = &ret;
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

  dp_client_->SendDpReply(dmp->method,
                          0,
                          dmp->id,
                          (const char *)res_data,
                          res_size);
}

void SenderServer::OnLoggingStatus(vznetdp::DpClient::Ptr client,
                                   const DpMessage* dmp) {

  memcpy(&m_LoggingEnable, dmp->data, sizeof(int));
}

void SenderServer::OnIpChange(vznetdp::DpClient::Ptr client,
                              const DpMessage* dmp) {

  unsigned int data[3];
  memcpy(data, dmp->data, 3 * sizeof(unsigned int));
  struct in_addr ip;
  ip.s_addr = data[0];
  data_parse_->SetDeviceIp(inet_ntoa(ip));
}

void SenderServer::OnGetIpMsg(vznetdp::DpClient::Ptr client,
                              const DpMessage* dmp) {
  if(dmp->type == TYPE_ERROR_TIMEOUT)
    return;
  struct in_addr ip;
  memcpy(&ip, dmp->data, sizeof(in_addr));
  data_parse_->SetDeviceIp(inet_ntoa(ip));
  boost::system::error_code err;
  DeviceRegFunc(err);	//消息注册，保证获取到 ip 后进行注册消息
}

void SenderServer::OnDpConnectSucceed(vznetdp::DpClient::Ptr client) {

  LOG(L_INFO) << "OnDpConnectSucceed";
  BOOST_ASSERT(dp_client_);
  // Register "AV_SRV_IVS_RESULT_NOTIFY"
  const char *ivs_result[1] = { EVT_SRV_IVS_RESULT };
  int res = VZNETDP_FAILURE;
  res = dp_client_->AddListenMessage(
          ivs_result, 1,
          boost::bind(&SenderServer::OnIvsResult,
                      this, _1, _2));
  if(res != VZNETDP_SUCCEED) {
    LOG(L_ERROR) << "Failure to register services";
  }

  // Register "AV_SRV_IVS_RESULT_NOTIFY"
  const char *env_serial[1] = { EVT_SRV_TT_TCP };
  res = dp_client_->AddListenMessage(
          env_serial, 1,
          boost::bind(&SenderServer::OnSerial,
                      this, _1, _2));
  if(res != VZNETDP_SUCCEED) {
    LOG(L_ERROR) << "Failure to register services";
  }

  // Register "AV_SRV_IVS_RESULT_NOTIFY"
  const char *gio_trigger[1] = { EVT_SRV_EXTERNAL_TRIGGER };
  res = dp_client_->AddListenMessage(
          gio_trigger, 1,
          boost::bind(&SenderServer::OnGioTrigger,
                      this, _1, _2));
  if(res != VZNETDP_SUCCEED) {
    LOG(L_ERROR) << "Failure to register services";
  }

  // Register "AV_SRV_IVS_RESULT_NOTIFY"
  const char *cfg_ctr[21] = {

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

    HTTPSENDER_MSG_CONFIG_CONTROL
  };
  res = dp_client_->AddListenMessage(
          cfg_ctr, 21,
          boost::bind(&SenderServer::OnConfigControl,
                      this, _1, _2));
  if(res != VZNETDP_SUCCEED) {
    LOG(L_ERROR) << "Failure to register services";
  }

  const char *logging_enable[1] = { DP_LOGGING_ENABLE };
  res = dp_client_->AddListenMessage(
          logging_enable, 1,
          boost::bind(&SenderServer::OnLoggingStatus,
                      this, _1, _2));
  if(res != VZNETDP_SUCCEED) {
    LOG(L_ERROR) << "Failure to register services";
  }

  const char *ip_change[1] = { SYS_SRV_IP_CHANGED };
  res = dp_client_->AddListenMessage(
          ip_change, 1,
          boost::bind(&SenderServer::OnIpChange,
                      this, _1, _2));
  if(res != VZNETDP_SUCCEED) {
    LOG(L_ERROR) << "Failure to register services";
  }
  std::string ip = GetSystemInfoDeviceIp();
  data_parse_->SetDeviceIp((char *)ip.c_str());
  boost::system::error_code err;
  DeviceRegFunc(err);	//消息注册，保证获取到 ip 后进行注册消息
  //dp_client_->SendDpRequest(SYS_SRV_GET_IP, 0, NULL, 0,
  //                          boost::bind(&SenderServer::OnGetIpMsg,
  //                                      this, _1, _2), 3);
  if (m_nOfflineCheckStatus) {
    dp_client_->SendDpMessage(TCP_SRV_SET_OFFLINE_CHECK,
                              0, (char*)&check_info_,
                              sizeof(TCP_SRV_OFFLINE_CHECK_INFO));
  }
}

void SenderServer::OnDpErrorEvent(vznetdp::DpClient::Ptr client,
                                  const boost::system::error_code& err) {
  LOG(L_ERROR) << err.message() ;
}

void SenderServer::OnDpMessage(vznetdp::DpClient::Ptr client,
                               const DpMessage* dmp) {

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
                      HTTP_FTP_CONFIG)) reader.parse(m_ftpbuffer,ftp_json_);
  if(iGetKeyValueFunc(http_email_config_value,
                      strlen(http_email_config_value),
                      m_emailbuffer, MAX_EMAIL_EXT_LEN,
                      HTTP_EMAIL_CONFIG)) reader.parse(m_emailbuffer,email_json_);
  //if(iGetKeyValueFunc(sys_hwinfo,
  //                    strlen(sys_hwinfo),
  //                    m_emailbuffer, MAX_EMAIL_EXT_LEN,
  //                    HTTP_EMAIL_CONFIG)) reader.parse(m_emailbuffer,email_json_);
  Json::Value hwinfo_js;
  std::string hwinfo;
  if(kvdb_client_->GetKey(sys_hwinfo, &hwinfo)) reader.parse(hwinfo,hwinfo_js);
  LOG(L_INFO) << "iInitHwiInfo";
  data_parse_->iInitHwiInfo(hwinfo_js);
  LOG(L_INFO) << "parse";
  reader.parse(m_ipext, ip_extjs_);
  //check_info_.bActive = m_nOfflineCheckStatus;
}


bool SenderServer::iGetKeyValueFunc(const char *pkey, int keylen, char *buffer,
                                    int bufferlen, int datatype) {
  LOG(L_INFO) << "GET " << pkey;

  if (kvdb_client_->GetKey(pkey, keylen, buffer, bufferlen, true)) {
    LOG(L_INFO) << "Read key form kvdb " << pkey;
    return true;
  }

  if(system_json_[JSON_DEVICE][JSON_NET][JSON_CENTER_SERVER].isNull()) {
    kvdb_client_->SetKey(pkey, keylen, buffer, bufferlen);
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
  return kvdb_client_->SetKey(pkey, keylen, buffer, bufferlen);
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
                              int resendtimes,
                              int timeout,
                              CallbackNewConnMessage call_back) {

  LOG(L_INFO) << "NewIPCConn";
  std::string postdata = value.toStyledString();

  HttpPostData(m_url,
               postdata,
               server_settings_.enable_ssl,
               m_port,
               timeout,
               resendtimes,
               call_back);
}

void SenderServer::HttpPostData(const std::string url,
                                const std::string postdata,
                                bool ssl_enable,
                                int port,
                                int timeout,
                                int resendtimes,
                                CallbackNewConnMessage call_back) {

  HttpConn::Ptr http_conn = curl_services_->CreateHttpConn(
                              url,
                              postdata,
                              server_settings_.enable_ssl == 1,
                              m_port,
                              timeout,
                              resendtimes);

  if (http_conn.get() == NULL)
    return;
  http_conn->SignalComplete.connect(boost::bind(call_back,
                                    _1, _2, _3, _4, curl_services_));
  curl_services_->PostData(http_conn);
}

void SenderServer::Stop(void) {
}
};
