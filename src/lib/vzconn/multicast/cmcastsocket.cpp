/************************************************************************/
/* Author      : Sober.Peng 17-06-22
/* Description :
/************************************************************************/
#include "cmcastsocket.h"

#include "vzbase/helper/stdafx.h"
#include "vzconn/base/connhead.h"
namespace vzconn {

SOCKET CMCastSocket::send_socket_ = INVALID_SOCKET;

CMCastSocket::CMCastSocket(vzconn::EVT_LOOP* p_loop,
                           vzconn::CClientInterface *cli_hdl)
  : vzconn::VSocket(cli_hdl)
  , p_evt_loop_(p_loop)
  , c_evt_recv_() {
}

CMCastSocket * CMCastSocket::Create(vzconn::EVT_LOOP* p_loop,
                                    vzconn::CClientInterface *cli_hdl) {
  if (!p_loop || !p_loop->get_event()) {
    LOG(L_ERROR) << "evt loop error.";
    return NULL;
  }

  if (cli_hdl == NULL) {
    LOG(L_ERROR) << "param is error.";
    return NULL;
  }

  return (new CMCastSocket(p_loop, cli_hdl));
}

CMCastSocket::~CMCastSocket() {
  c_evt_recv_.Stop();

  Close();
}

bool CMCastSocket::Open(const unsigned char* s_center_ip, unsigned short n_center_port) {
  SOCKET fd = socket(AF_INET, SOCK_DGRAM, 0);
  if (fd <0) {
    perror("socket failed");
    return false;
  }
  int yes = 1;
  int ret = setsockopt(fd, SOL_SOCKET,SO_REUSEADDR, (char*)&yes, sizeof(yes));
  if (ret < 0) {
    perror("Reusing ADDR failed");
    return false;
  }

  struct sockaddr_in s_local;
  s_local.sin_family      = AF_INET;
  s_local.sin_addr.s_addr = INADDR_ANY;
  s_local.sin_port        = htons(n_center_port);
  ret = bind(fd, (struct sockaddr *)&s_local, sizeof(struct sockaddr_in));
  if (ret < 0) {
    perror("Bind error");
    close(fd);
    return false;
  }

  struct ip_mreq mreq;
  mreq.imr_multiaddr.s_addr = inet_addr((char*)s_center_ip);
  mreq.imr_interface.s_addr = htonl(INADDR_ANY);
  ret = setsockopt(fd,IPPROTO_IP,IP_ADD_MEMBERSHIP, (char*)&mreq, sizeof(mreq));
  if (ret < 0) {
    perror("setsockopt");
    return false;
  }
  SetSocket(fd);

  c_evt_recv_.Init(p_evt_loop_, EvtRecv, this);
  ret = c_evt_recv_.Start(GetSocket(), EVT_READ | EVT_PERSIST);
  if (ret != 0) {
    return false;
  }
  return true;
}

int32 CMCastSocket::EvtRecv(SOCKET      fd,
                            short       n_events,
                            const void *p_usr_arg) {
  int32 n_ret = 0;
  if (p_usr_arg) {
    n_ret = ((CMCastSocket*)p_usr_arg)->OnRecv();
    if (n_ret < 0) {
      delete ((CMCastSocket*)p_usr_arg);
    }
  }
  return n_ret;
}

int32 CMCastSocket::OnRecv() {
  vzconn::CInetAddr c_remote_addr;
  char s_data[2048] = {0};
  int32 n_data = vzconn::VSocket::Recv(s_data, 2047, c_remote_addr);
  if (cli_hdl_ptr_) {
    cli_hdl_ptr_->HandleRecvPacket(this, s_data, n_data, 0);
  }

  if (n_data < 0) {
    if (cli_hdl_ptr_) {
      cli_hdl_ptr_->HandleClose(this);
    }
  }
  return n_data;
}

int32 CMCastSocket::SendUdpData(const char* s_center_ip, unsigned short n_center_port,
                                const char* p_data, unsigned int n_data) {
  if (send_socket_ == INVALID_SOCKET) {
    send_socket_ = socket(AF_INET, SOCK_DGRAM, 0);
    if (send_socket_ <0) {
      perror("socket failed");
      return -1;
    }

    int yes = 1;
    int ret = setsockopt(send_socket_, SOL_SOCKET,SO_REUSEADDR, (char*)&yes, sizeof(yes));
    if (ret < 0) {
      perror("Reusing ADDR failed");
      return -1;
    }

    char ttl = 1;
    ret = setsockopt(send_socket_,  IPPROTO_IP, IP_MULTICAST_TTL, (char*)&ttl, sizeof(ttl));
    if (ret < 0) {
      perror("Setsockopt IP_MULTICAST_TTL fall.");
      return -1;
    }

    char loop = 0;
    ret = setsockopt(send_socket_, IPPROTO_IP, IP_MULTICAST_LOOP, (char*)&loop, sizeof(loop));
    if (ret < 0) {
      perror("Setsockopt IP_MULTICAST_LOOP fall.");
      return -1;
    }
  }

  if (send_socket_ < 0) {
    LOG_ERROR("socket failed");
    return -1;
  }

  struct sockaddr_in s_center;
  s_center.sin_family = AF_INET;
  s_center.sin_addr.s_addr = inet_addr((char*)s_center_ip);
  s_center.sin_port = htons(n_center_port);
  int n_ret = sendto(send_socket_, p_data, n_data, 0,
                     (struct sockaddr*)&s_center, sizeof(struct sockaddr));
  return n_ret;
}

}  // namespace vzconn
