/************************************************************************/
/* Author      : Sober.Peng 17-06-22
/* Description :
/************************************************************************/
#include "cmcastsocket.h"

#include "stdafx.h"
#include "vzconn/base/connhead.h"

CMCastSocket::CMCastSocket(vzconn::EVT_LOOP* p_loop,
                           vzconn::CClientInterface *c_cli_proc)
  : vzconn::VSocket(c_cli_proc)
  , p_evt_loop_(p_loop)
  , c_evt_recv_()
  , c_recv_data_()
  , c_send_data_() {
}

CMCastSocket * CMCastSocket::Create(vzconn::EVT_LOOP* p_loop,
                                    vzconn::CClientInterface *c_cli_proc) {
  if (!p_loop || !p_loop->get_event()) {
    LOG(L_ERROR) << "evt loop error.";
    return NULL;
  }

  if (c_cli_proc == NULL) {
    LOG(L_ERROR) << "param is error.";
    return NULL;
  }

  return (new CMCastSocket(p_loop, c_cli_proc));
}

CMCastSocket::~CMCastSocket() {
  c_evt_recv_.Stop();

  Close();
}

int32 CMCastSocket::Open(vzconn::CInetAddr *p_loca_addr, 
                         bool b_block, 
                         bool b_reuse) {
  return 0;
}

int32 CMCastSocket::AsyncWrite(const void *p_data, uint32 n_data, uint16 e_flag) {
  return 0;
}

int32 CMCastSocket::AsyncWrite(struct iovec iov[], uint32 n_iov, uint16 e_flag) {
  return 0;
}

int32 CMCastSocket::EvtRecv(SOCKET fd,
                            short n_events,
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
  int32 n_ret = 0;
  n_ret = Recv(c_recv_data_.GetWritePtr(), c_recv_data_.FreeSize());
  if (n_ret > 0 && cli_hdl_ptr_) {
    c_recv_data_.MoveWritePtr(n_ret);

    uint16 n_flag = 0;
    int32 n_pkg_len = cli_hdl_ptr_->NetHeadParse(c_recv_data_.GetReadPtr(),
                      c_recv_data_.UsedSize(),
                      &n_flag);
    if (n_pkg_len > 0) {
      n_ret = cli_hdl_ptr_->HandleRecvPacket(
                this,
                c_recv_data_.GetReadPtr() + cli_hdl_ptr_->NetHeadSize(),
                c_recv_data_.UsedSize() - cli_hdl_ptr_->NetHeadSize(),
                n_flag);
    } else {
      n_ret = -1;
    }
    c_recv_data_.Clear();
  }

  if (n_ret < 0) {
    if (cli_hdl_ptr_) {
      cli_hdl_ptr_->HandleClose(this);
    }
  }
  return n_ret;
}

#if 0
int BroadcastInfo::SendUdpData(char* center_ip, int port,  char* data,
      int len, unsigned int our_ip, int our_port, int is_multicast) {
  int socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
  if (socket_fd <0) {return -1;}

  struct  sockaddr_in address;

  address.sin_family = AF_INET;
  address.sin_addr.s_addr = inet_addr(center_ip);
  address.sin_port = htons(port);

  struct  sockaddr_in our_addr;
  our_addr.sin_family = AF_INET;
  our_addr.sin_addr.s_addr = our_ip;
  our_addr.sin_port = htons(our_port);
  if (bind(socket_fd,  (struct sockaddr *) &our_addr,
           sizeof(struct sockaddr_in)) == -1) {
    LOG(L_ERROR) << "Bind error";
    close(socket_fd);
    return -1;
  }

  if (is_multicast) {
    u_char ttl = 1;
    int ret = setsockopt(socket_fd,  IPPROTO_IP,  IP_MULTICAST_TTL,  &ttl,
                         sizeof(ttl));
    if (ret < 0) {
      LOG(L_ERROR) << "Setsockopt IP_MULTICAST_TTL fall:" << strerror(errno);
    }

    u_char loop = 0;
    ret = setsockopt(socket_fd, IPPROTO_IP, IP_MULTICAST_LOOP, &loop,
      sizeof(loop));
    if (ret < 0) {
      LOG(L_ERROR) << "Setsockopt IP_MULTICAST_LOOP fall:" << strerror(errno);
    }
  }

  int ret = sendto(socket_fd,  data,  len,  0, (struct sockaddr*)&address,
                   sizeof(struct sockaddr));
  close(socket_fd);
  return ret;
}
#endif

