#include "chttpsync.h"
#include "curl/curl.h"

#include <string>

CHttpSync::CHttpSync(void) 
  : is_debug_(0) {
}

CHttpSync::~CHttpSync(void) {
}

static int OnDebug(CURL *, curl_infotype itype, char * pData, size_t size, void *) {
  if(itype == CURLINFO_TEXT) {
    //printf("[TEXT]%s\n", pData);
  } else if(itype == CURLINFO_HEADER_IN) {
    printf("[HEADER_IN]%s\n", pData);
  } else if(itype == CURLINFO_HEADER_OUT) {
    printf("[HEADER_OUT]%s\n", pData);
  } else if(itype == CURLINFO_DATA_IN) {
    printf("[DATA_IN]%s\n", pData);
  } else if(itype == CURLINFO_DATA_OUT) {
    printf("[DATA_OUT]%s\n", pData);
  }
  return 0;
}

static size_t OnRespData(void* buffer, size_t size, size_t nmemb, void* lpVoid) {
  std::string *str = dynamic_cast<std::string*>((std::string *)lpVoid);
  if(NULL == str || NULL == buffer) {
    return -1;
  }
  str->append((char*)buffer, size * nmemb);
  return nmemb;
}

int CHttpSync::Post(const std::string &surl, unsigned int nport,
                      const std::string &spost, std::string &sresp) {
  CURLcode res;
  CURL* curl = curl_easy_init();
  if(NULL == curl) {
    return CURLE_FAILED_INIT;
  }
  if(is_debug_) {
    curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);
    curl_easy_setopt(curl, CURLOPT_DEBUGFUNCTION, OnDebug);
  }
  curl_easy_setopt(curl, CURLOPT_URL, surl.c_str());
  curl_easy_setopt(curl, CURLOPT_PORT, nport);
  curl_easy_setopt(curl, CURLOPT_POST, 1);
  curl_easy_setopt(curl, CURLOPT_POSTFIELDS, spost.c_str());
  curl_easy_setopt(curl, CURLOPT_READFUNCTION, NULL);
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, OnRespData);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&sresp);
  curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);
  curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT_MS, HTTP_TIMEOUT_CONN);
  curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, HTTP_TIMEOUT_RESP);
  res = curl_easy_perform(curl);
  curl_easy_cleanup(curl);
  return res;
}

int CHttpSync::Get(const std::string &surl, unsigned int nport,
                     std::string &sresp) {
  CURLcode res;
  CURL *curl = curl_easy_init();
  if(NULL == curl) {
    return CURLE_FAILED_INIT;
  }
  if(is_debug_) {
    curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);
    curl_easy_setopt(curl, CURLOPT_DEBUGFUNCTION, OnDebug);
  }
  curl_easy_setopt(curl, CURLOPT_URL, surl.c_str());
  curl_easy_setopt(curl, CURLOPT_PORT, nport);
  curl_easy_setopt(curl, CURLOPT_READFUNCTION, NULL);
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, OnRespData);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&sresp);
  /**
  * 当多个线程都使用超时处理的时候，同时主线程中有sleep或是wait等操作。
  * 如果不设置这个选项，libcurl将会发信号打断这个wait从而导致程序退出。
  */
  curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);
  curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT_MS, HTTP_TIMEOUT_CONN);
  curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, HTTP_TIMEOUT_RESP);
  res = curl_easy_perform(curl);
  curl_easy_cleanup(curl);
  return res;
}

int CHttpSync::Posts(const std::string &surl, unsigned int nport,
                       const std::string &spost, std::string &sresp,
                       const char *pCaPath) {
  CURLcode res;
  CURL *curl = curl_easy_init();
  if(NULL == curl) {
    return CURLE_FAILED_INIT;
  }
  if(is_debug_) {
    curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);
    curl_easy_setopt(curl, CURLOPT_DEBUGFUNCTION, OnDebug);
  }
  curl_easy_setopt(curl, CURLOPT_URL, surl.c_str());
  curl_easy_setopt(curl, CURLOPT_PORT, nport);
  curl_easy_setopt(curl, CURLOPT_POST, 1);
  curl_easy_setopt(curl, CURLOPT_POSTFIELDS, spost.c_str());
  curl_easy_setopt(curl, CURLOPT_READFUNCTION, NULL);
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, OnRespData);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&sresp);
  curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);
  if(NULL == pCaPath) {
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, false);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, false);
  } else {
    //缺省情况就是PEM，所以无需设置，另外支持DER
    //curl_easy_setopt(curl,CURLOPT_SSLCERTTYPE,"PEM");
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, true);
    curl_easy_setopt(curl, CURLOPT_CAINFO, pCaPath);
  }
  curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT_MS, HTTP_TIMEOUT_CONN);
  curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, HTTP_TIMEOUT_RESP);
  res = curl_easy_perform(curl);
  curl_easy_cleanup(curl);
  return res;
}

int CHttpSync::Gets(const std::string &surl, unsigned int nport, 
                      std::string &sresp,
                      const char *pCaPath) {
  CURLcode res;
  CURL *curl = curl_easy_init();
  if(NULL == curl) {
    return CURLE_FAILED_INIT;
  }
  if(is_debug_) {
    curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);
    curl_easy_setopt(curl, CURLOPT_DEBUGFUNCTION, OnDebug);
  }
  curl_easy_setopt(curl, CURLOPT_URL, surl.c_str());
  curl_easy_setopt(curl, CURLOPT_PORT, nport);
  curl_easy_setopt(curl, CURLOPT_READFUNCTION, NULL);
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, OnRespData);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&sresp);
  curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);
  if(NULL == pCaPath) {
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, false);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, false);
  } else {
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, true);
    curl_easy_setopt(curl, CURLOPT_CAINFO, pCaPath);
  }
  curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT_MS, HTTP_TIMEOUT_CONN);
  curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, HTTP_TIMEOUT_RESP);
  res = curl_easy_perform(curl);
  curl_easy_cleanup(curl);
  return res;
}

void CHttpSync::SetDebug(bool bDebug) {
  is_debug_ = bDebug ? 1 : 0;
}

const char *CHttpSync::GetErr(int error) {
  switch (error) {
  case CURLE_OK:
    return "No error";

  case CURLE_UNSUPPORTED_PROTOCOL:
    return "Unsupported protocol";

  case CURLE_FAILED_INIT:
    return "Failed initialization";

  case CURLE_URL_MALFORMAT:
    return "URL using bad/illegal format or missing URL";

  case CURLE_NOT_BUILT_IN:
    return "A requested feature, protocol or option was not found built-in in"
      " this libcurl due to a build-time decision.";

  case CURLE_COULDNT_RESOLVE_PROXY:
    return "Couldn't resolve proxy name";

  case CURLE_COULDNT_RESOLVE_HOST:
    return "Couldn't resolve host name";

  case CURLE_COULDNT_CONNECT:
    return "Couldn't connect to server";

  case CURLE_FTP_WEIRD_SERVER_REPLY:
    return "FTP: weird server reply";

  case CURLE_REMOTE_ACCESS_DENIED:
    return "Access denied to remote resource";

  case CURLE_FTP_ACCEPT_FAILED:
    return "FTP: The server failed to connect to data port";

  case CURLE_FTP_ACCEPT_TIMEOUT:
    return "FTP: Accepting server connect has timed out";

  case CURLE_FTP_PRET_FAILED:
    return "FTP: The server did not accept the PRET command.";

  case CURLE_FTP_WEIRD_PASS_REPLY:
    return "FTP: unknown PASS reply";

  case CURLE_FTP_WEIRD_PASV_REPLY:
    return "FTP: unknown PASV reply";

  case CURLE_FTP_WEIRD_227_FORMAT:
    return "FTP: unknown 227 response format";

  case CURLE_FTP_CANT_GET_HOST:
    return "FTP: can't figure out the host in the PASV response";

  case CURLE_HTTP2:
    return "Error in the HTTP2 framing layer";

  case CURLE_FTP_COULDNT_SET_TYPE:
    return "FTP: couldn't set file type";

  case CURLE_PARTIAL_FILE:
    return "Transferred a partial file";

  case CURLE_FTP_COULDNT_RETR_FILE:
    return "FTP: couldn't retrieve (RETR failed) the specified file";

  case CURLE_QUOTE_ERROR:
    return "Quote command returned error";

  case CURLE_HTTP_RETURNED_ERROR:
    return "HTTP response code said error";

  case CURLE_WRITE_ERROR:
    return "Failed writing received data to disk/application";

  case CURLE_UPLOAD_FAILED:
    return "Upload failed (at start/before it took off)";

  case CURLE_READ_ERROR:
    return "Failed to open/read local data from file/application";

  case CURLE_OUT_OF_MEMORY:
    return "Out of memory";

  case CURLE_OPERATION_TIMEDOUT:
    return "Timeout was reached";

  case CURLE_FTP_PORT_FAILED:
    return "FTP: command PORT failed";

  case CURLE_FTP_COULDNT_USE_REST:
    return "FTP: command REST failed";

  case CURLE_RANGE_ERROR:
    return "Requested range was not delivered by the server";

  case CURLE_HTTP_POST_ERROR:
    return "Internal problem setting up the POST";

  case CURLE_SSL_CONNECT_ERROR:
    return "SSL connect error";

  case CURLE_BAD_DOWNLOAD_RESUME:
    return "Couldn't resume download";

  case CURLE_FILE_COULDNT_READ_FILE:
    return "Couldn't read a file:// file";

  case CURLE_LDAP_CANNOT_BIND:
    return "LDAP: cannot bind";

  case CURLE_LDAP_SEARCH_FAILED:
    return "LDAP: search failed";

  case CURLE_FUNCTION_NOT_FOUND:
    return "A required function in the library was not found";

  case CURLE_ABORTED_BY_CALLBACK:
    return "Operation was aborted by an application callback";

  case CURLE_BAD_FUNCTION_ARGUMENT:
    return "A libcurl function was given a bad argument";

  case CURLE_INTERFACE_FAILED:
    return "Failed binding local connection end";

  case CURLE_TOO_MANY_REDIRECTS:
    return "Number of redirects hit maximum amount";

  case CURLE_UNKNOWN_OPTION:
    return "An unknown option was passed in to libcurl";

  case CURLE_TELNET_OPTION_SYNTAX:
    return "Malformed telnet option";

  case CURLE_PEER_FAILED_VERIFICATION:
    return "SSL peer certificate or SSH remote key was not OK";

  case CURLE_GOT_NOTHING:
    return "Server returned nothing (no headers, no data)";

  case CURLE_SSL_ENGINE_NOTFOUND:
    return "SSL crypto engine not found";

  case CURLE_SSL_ENGINE_SETFAILED:
    return "Can not set SSL crypto engine as default";

  case CURLE_SSL_ENGINE_INITFAILED:
    return "Failed to initialise SSL crypto engine";

  case CURLE_SEND_ERROR:
    return "Failed sending data to the peer";

  case CURLE_RECV_ERROR:
    return "Failure when receiving data from the peer";

  case CURLE_SSL_CERTPROBLEM:
    return "Problem with the local SSL certificate";

  case CURLE_SSL_CIPHER:
    return "Couldn't use specified SSL cipher";

  case CURLE_SSL_CACERT:
    return "Peer certificate cannot be authenticated with given CA "
      "certificates";

  case CURLE_SSL_CACERT_BADFILE:
    return "Problem with the SSL CA cert (path? access rights?)";

  case CURLE_BAD_CONTENT_ENCODING:
    return "Unrecognized or bad HTTP Content or Transfer-Encoding";

  case CURLE_LDAP_INVALID_URL:
    return "Invalid LDAP URL";

  case CURLE_FILESIZE_EXCEEDED:
    return "Maximum file size exceeded";

  case CURLE_USE_SSL_FAILED:
    return "Requested SSL level failed";

  case CURLE_SSL_SHUTDOWN_FAILED:
    return "Failed to shut down the SSL connection";

  case CURLE_SSL_CRL_BADFILE:
    return "Failed to load CRL file (path? access rights?, format?)";

  case CURLE_SSL_ISSUER_ERROR:
    return "Issuer check against peer certificate failed";

  case CURLE_SEND_FAIL_REWIND:
    return "Send failed since rewinding of the data stream failed";

  case CURLE_LOGIN_DENIED:
    return "Login denied";

  case CURLE_TFTP_NOTFOUND:
    return "TFTP: File Not Found";

  case CURLE_TFTP_PERM:
    return "TFTP: Access Violation";

  case CURLE_REMOTE_DISK_FULL:
    return "Disk full or allocation exceeded";

  case CURLE_TFTP_ILLEGAL:
    return "TFTP: Illegal operation";

  case CURLE_TFTP_UNKNOWNID:
    return "TFTP: Unknown transfer ID";

  case CURLE_REMOTE_FILE_EXISTS:
    return "Remote file already exists";

  case CURLE_TFTP_NOSUCHUSER:
    return "TFTP: No such user";

  case CURLE_CONV_FAILED:
    return "Conversion failed";

  case CURLE_CONV_REQD:
    return "Caller must register CURLOPT_CONV_ callback options";

  case CURLE_REMOTE_FILE_NOT_FOUND:
    return "Remote file not found";

  case CURLE_SSH:
    return "Error in the SSH layer";

  case CURLE_AGAIN:
    return "Socket not ready for send/recv";

  case CURLE_RTSP_CSEQ_ERROR:
    return "RTSP CSeq mismatch or invalid CSeq";

  case CURLE_RTSP_SESSION_ERROR:
    return "RTSP session error";

  case CURLE_FTP_BAD_FILE_LIST:
    return "Unable to parse FTP file list";

  case CURLE_CHUNK_FAILED:
    return "Chunk callback failed";

  case CURLE_NO_CONNECTION_AVAILABLE:
    return "The max connection limit is reached";

  case CURLE_SSL_PINNEDPUBKEYNOTMATCH:
    return "SSL public key does not match pinned public key";

  case CURLE_SSL_INVALIDCERTSTATUS:
    return "SSL server certificate status verification FAILED";

    /* error codes not used by current libcurl */
  case CURLE_OBSOLETE20:
  case CURLE_OBSOLETE24:
  case CURLE_OBSOLETE29:
  case CURLE_OBSOLETE32:
  case CURLE_OBSOLETE40:
  case CURLE_OBSOLETE44:
  case CURLE_OBSOLETE46:
  case CURLE_OBSOLETE50:
  case CURLE_OBSOLETE57:
  case CURL_LAST:
    break;
  }
  return "Unknown error";
}
