/************************************************************************
*Author      : Sober.Peng 17-07-03
*Description : 封装curl multi+libevent实现异步http
************************************************************************/
#include "cmultiserver.h"

#include <map>
#include <string.h>
#include <stdlib.h>

#include "chttpasync.h"
#include "vzbase/helper/stdafx.h"

CMultiServer::CCurlEvent::CCurlEvent(curl_socket_t fd)
  : curl_sock_(fd)
  , evt_io_() {
}

CMultiServer::CCurlEvent::~CCurlEvent() {
  LOG(L_INFO) << "~CEvtCurl";
}

/* Called by libevent when we get action on a multi socket */
int CMultiServer::CCurlEvent::event_cb(SOCKET fd,
                                       short kind,
                                       const void *userp) {
  CMultiServer *p_multi = (CMultiServer *)userp;
  int action = ((kind & EVT_READ)  ? CURL_CSELECT_IN : 0) |
               ((kind & EVT_WRITE) ? CURL_CSELECT_OUT : 0);

  CURLMcode rc = curl_multi_socket_action(p_multi->curl_multi_, fd, action,
                                          &p_multi->still_running_);
  LOG_ERROR("event_cb: curl_multi_socket_action %d.", rc);

  p_multi->easy_check_info();
  return 0;
}

//////////////////////////////////////////////////////////////////////////
//static std::map<void *, int> galloc_blocks;
//static int galloc_mem_size = 0;

//int galloc_size() {
//  return galloc_blocks.size();
//}
//void *getgalloc_addr(int &n) {
//  if (galloc_blocks.size() < n) return NULL;
//
//  std::map<void *, int>::iterator iter = galloc_blocks.begin();
//  for (int i = 0; i < n; i++) {
//    iter++;
//  }
//  n = iter->second;
//  return iter->first;
//}
//
//void *Jcurl_malloc_callback(size_t size) {
//  void *ret = malloc(size);
//  galloc_blocks[ret] = size;
//  galloc_mem_size += size;
//  memset(ret, 0, size);
//  return ret;
//}
//
//void Jcurl_free_callback(void *ptr) {
//  if (ptr == 0) return;
//  std::map<void *, int>::iterator iter = galloc_blocks.find(ptr);
//  if (iter == galloc_blocks.end()) {
//    LOG(FATAL) << "free one malloc fatal!";
//  }
//  galloc_mem_size -= iter->second;
//  galloc_blocks.erase(iter);
//  free(ptr);
//}
//
//void *Jcurl_realloc_callback(void *ptr, size_t size) {
//  std::map<void *, int>::iterator iter = galloc_blocks.find(ptr);
//  if (iter == galloc_blocks.end()) {
//    LOG(FATAL) << "free one malloc fatal!";
//  }
//  galloc_mem_size -= iter->second;
//  galloc_blocks.erase(iter);
//
//  ptr = realloc(ptr, size);
//
//  galloc_blocks[ptr] = size;
//  galloc_mem_size += size;
//
//  return ptr;
//}
//
//char *Jcurl_strdup_callback(const char *str) {
//  int str_size = strlen(str) + 1;
//  char *new_str = (char *)Jcurl_malloc_callback(str_size);
//  memcpy(new_str, str, str_size);
//  return new_str;
//}
//
//void *Jcurl_calloc_callback(size_t nmemb, size_t size) {
//  return Jcurl_malloc_callback(nmemb * size);
//}

CMultiServer::CMultiServer(vzconn::EVT_LOOP *p_evt_loop)
  : evt_loop_(p_evt_loop)
  , evt_timer_()
  , curl_multi_(NULL)
  , still_running_(0) {
  // curl_global_init(CURL_GLOBAL_DEFAULT);

  //curl_global_init_mem(CURL_GLOBAL_DEFAULT, Jcurl_malloc_callback,
  //                     Jcurl_free_callback, Jcurl_realloc_callback,
  //                     Jcurl_strdup_callback, Jcurl_calloc_callback);
  //LOG(L_INFO) << "curl memsize " << galloc_mem_size;
}

CMultiServer::~CMultiServer() {
  UninitCurlServices();
  curl_global_cleanup();
}

CMultiServer *CMultiServer::Create(vzconn::EVT_LOOP *p_evt_loop) {
  if (NULL == p_evt_loop) {
    LOG(L_ERROR) << "param is null.";
    return NULL;
  }

  CMultiServer *new_p = new CMultiServer(p_evt_loop);
  if (!new_p->InitCurlServices()) {
    LOG(L_ERROR) << "CurlServices InitCurlServices failure!";
    delete new_p;
    new_p = NULL;
  }
  return new_p;
}

void CMultiServer::Remove(CMultiServer *ins) {
  if (ins) {
    delete ins;
    ins = NULL;
  }
}

bool CMultiServer::InitCurlServices() {
  curl_multi_ = curl_multi_init();
  if (NULL == curl_multi_) {
    LOG(L_ERROR) << "curl multi init failed.";
    return false;
  }

  /* setup the generic multi interface options we want */
  CURLMcode ret;
  ret = curl_multi_setopt(curl_multi_, CURLMOPT_SOCKETFUNCTION, multi_sock_cb);
  ret = curl_multi_setopt(curl_multi_, CURLMOPT_SOCKETDATA, this);
  ret = curl_multi_setopt(curl_multi_, CURLMOPT_TIMERFUNCTION, multi_timer_cb);
  ret = curl_multi_setopt(curl_multi_, CURLMOPT_TIMERDATA, this);

  evt_timer_.Init(evt_loop_, timer_cb, this);
  return true;
}

bool CMultiServer::UninitCurlServices() {
  evt_timer_.Stop();

  if (curl_multi_) {
    curl_multi_cleanup(curl_multi_);
    curl_multi_ = NULL;
  }
  return true;
}

/* CURLMOPT_SOCKETFUNCTION */
int CMultiServer::multi_sock_cb(CURL *e, curl_socket_t s, int what, void *cbp,
                                void *sockp) {
  CMultiServer *g = (CMultiServer*)cbp;
  CCurlEvent *fdp = (CCurlEvent*)sockp;
  if (what & (CURL_POLL_IN | CURL_POLL_OUT)) {
    if (sockp == NULL) {
      fdp = new CCurlEvent(s);
      if (NULL == fdp) {
        return -1;
      }
      curl_multi_assign(g->curl_multi_, s, (void *)fdp); // s与fdp关联
      fdp->evt_io_.Init(g->evt_loop_, CMultiServer::CCurlEvent::event_cb, g);
    }
  }

  LOG(L_INFO) << "multi_sock_cb " << what;
  switch (what) {
  case CURL_POLL_IN:
    if (fdp) {
      fdp->evt_io_.Stop();
      fdp->evt_io_.Start(fdp->curl_sock_, EVT_READ | EVT_PERSIST);
    }
    break;
  case CURL_POLL_OUT:
    if (fdp) {
      fdp->evt_io_.Stop();
      fdp->evt_io_.Start(fdp->curl_sock_, EVT_WRITE | EVT_PERSIST);
    }
    break;
  case CURL_POLL_INOUT:
    if (fdp) {
      fdp->evt_io_.Stop();
      fdp->evt_io_.Start(fdp->curl_sock_, EVT_READ | EVT_WRITE | EVT_PERSIST);
    }
    break;
  case CURL_POLL_REMOVE:
    if (fdp) {
      fdp->evt_io_.Stop();
      delete fdp;

      curl_multi_assign(g->curl_multi_, s, NULL);
    }
    break;

  default:
    LOG(L_ERROR) << "multi_sock_cb recv exception code " << what;
    abort();
  }
  return 0;
}

/* Update the event timer after curl_multi library calls */
int CMultiServer::multi_timer_cb(CURLM *multi, long timeout_ms, CMultiServer *g) {
  if (timeout_ms < 0) {
    LOG(L_ERROR) << "multi_timer_cb number is error " << timeout_ms;
    return -1;
  }

  LOG(L_WARNING) << "set timeout " << timeout_ms;
  g->evt_timer_.Start(timeout_ms, 0);
  return 0;
}

/* Called by libevent when our timeout expires */
int CMultiServer::timer_cb(SOCKET fd, short kind, const void *pthiz) {
  LOG(L_WARNING) << "timeout cb";
  CMultiServer *multis = (CMultiServer*)(pthiz);
  if (multis) {
    CURLMcode rc = curl_multi_socket_action(multis->curl_multi_,
                                            CURL_SOCKET_TIMEOUT,
                                            0,
                                            &multis->still_running_);
    LOG_ERROR("timer_cb: curl_multi_socket_action %d.", rc);
    multis->easy_check_info();
  }
  return 0;
}

/* Check for completed transfers, and remove their easy handles */
void CMultiServer::easy_check_info() {
  CURLMsg     *msg   = NULL;
  CURLcode     res   = CURLE_OK;

  CURL        *pcurl = NULL;
  CHttpAsync  *peasy = NULL;

  int          msgs_left = 0;
  while ((msg = curl_multi_info_read(curl_multi_, &msgs_left))) {
    if (msg->msg == CURLMSG_DONE) {
      pcurl = msg->easy_handle;
      res   = msg->data.result;
      curl_easy_getinfo(pcurl, CURLINFO_PRIVATE, &peasy);
      //curl_easy_getinfo(p_curl, CURLINFO_EFFECTIVE_URL, &eff_url);
      curl_multi_remove_handle(curl_multi_, pcurl);

      if (peasy->post_callback_) {
        peasy->post_callback_->RespCallvack(peasy, res);
      }
    } else {
      LOG(L_WARNING) << "msg->msg != CURLMSG_DONE ! It's " << msg->msg;
    }
  }
}

const char *CMultiServer::CurlEasyError(int error) {
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

  case CURLE_TOO_MANY_REDIRECTS :
    return "Number of redirects hit maximum amount";

  case CURLE_UNKNOWN_OPTION:
    return "An unknown option was passed in to libcurl";

  case CURLE_TELNET_OPTION_SYNTAX :
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

