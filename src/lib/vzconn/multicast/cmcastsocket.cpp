/************************************************************************
*Author      : Sober.Peng 17-07-10
*Description :
************************************************************************/
#include "cmcastsocket.h"
#include "vzconn/base/basedefines.h"

#ifndef WIN32
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#else
#include <WinSock2.h>
#include <Ws2tcpip.h>
#include <iphlpapi.h>
#include <vector>
#endif

#include "vzbase/helper/stdafx.h"

namespace vzconn {
  
CMCastSocket::CMCastSocket(vzconn::EVT_LOOP* p_loop,
                           vzconn::CClientInterface *cli_hdl)
  : vzconn::VSocket(cli_hdl)
  , evt_loop_(p_loop)
  , evt_recv_() {
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
  evt_recv_.Stop();

  Close();
}

#ifdef WIN32

#define WORKING_BUFFER_SIZE 15000
#define MAX_TRIES 3

#define MALLOC(x) HeapAlloc(GetProcessHeap(), 0, (x))
#define FREE(x) HeapFree(GetProcessHeap(), 0, (x))

void bzero(void* data, int size) {
  memset(data, 0, size);
}
void bcopy(void* des, const void* src, int size) {
  memcpy(des, src, size);
}
#endif

#ifdef WIN32

enum InterfaceState {
  INTERFACE_STATE_NONE,
  INTERFACE_STATE_ADD,
  INTERFACE_STATE_ERROR
};

struct NetworkInterfaceState {
  std::string adapter_name;
  IN_ADDR ip_addr;
  InterfaceState state;
};

std::vector<NetworkInterfaceState> network_interfaces_;

int UpdateNetworkInterface(std::vector<NetworkInterfaceState>& net_inter) {
  /* Declare and initialize variables */
  DWORD dwSize = 0;
  DWORD dwRetVal = 0;

  unsigned int i = 0;

  // Set the flags to pass to GetAdaptersAddresses
  ULONG flags = GAA_FLAG_INCLUDE_PREFIX;

  // default to unspecified address family (both)
  ULONG family = AF_INET;

  LPVOID lpMsgBuf = NULL;

  PIP_ADAPTER_ADDRESSES pAddresses = NULL;
  ULONG outBufLen = WORKING_BUFFER_SIZE;
  ULONG Iterations = 0;

  PIP_ADAPTER_ADDRESSES pCurrAddresses = NULL;
  PIP_ADAPTER_UNICAST_ADDRESS pUnicast = NULL;
  PIP_ADAPTER_ANYCAST_ADDRESS pAnycast = NULL;
  PIP_ADAPTER_MULTICAST_ADDRESS pMulticast = NULL;
  IP_ADAPTER_DNS_SERVER_ADDRESS *pDnServer = NULL;
  IP_ADAPTER_PREFIX *pPrefix = NULL;
  // char temp_ip[128];

  do {
    pAddresses = (IP_ADAPTER_ADDRESSES *)MALLOC(outBufLen);
    if (pAddresses == NULL) {
      printf
      ("Memory allocation failed for IP_ADAPTER_ADDRESSES struct\n");
      return 1;
    }

    dwRetVal =
      GetAdaptersAddresses(family, flags, NULL, pAddresses, &outBufLen);

    if (dwRetVal == ERROR_BUFFER_OVERFLOW) {
      FREE(pAddresses);
      pAddresses = NULL;
    } else {
      break;
    }

    Iterations++;

  } while ((dwRetVal == ERROR_BUFFER_OVERFLOW) && (Iterations < MAX_TRIES));

  // -------------------------------------------------------------------------
  if (dwRetVal == NO_ERROR) {
    // If successful, output some information from the data we received
    pCurrAddresses = pAddresses;
    while (pCurrAddresses) {
      //printf("\tAdapter name: %s\n", pCurrAddresses->AdapterName);
      pUnicast = pCurrAddresses->FirstUnicastAddress;
      if (pUnicast != NULL) {
        for (i = 0; pUnicast != NULL; i++) {
          if (pUnicast->Address.lpSockaddr->sa_family == AF_INET) {
            NetworkInterfaceState nif;
            sockaddr_in* v4_addr =
              reinterpret_cast<sockaddr_in*>(pUnicast->Address.lpSockaddr);
            //printf("\tIpv4 pUnicast : %s\n", hexToCharIP(temp_ip, v4_addr->sin_addr));
            nif.ip_addr = v4_addr->sin_addr;
            nif.adapter_name = pCurrAddresses->AdapterName;
            nif.state = InterfaceState::INTERFACE_STATE_NONE;
            net_inter.push_back(nif);
          }
          pUnicast = pUnicast->Next;
        }
      }
      pCurrAddresses = pCurrAddresses->Next;
    }
  } else {
    if (dwRetVal != ERROR_NO_DATA) {
      if (FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER
                        |FORMAT_MESSAGE_FROM_SYSTEM
                        | FORMAT_MESSAGE_IGNORE_INSERTS,
                        NULL,
                        dwRetVal,
                        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                        // Default language
                        (LPTSTR)& lpMsgBuf, 0, NULL)) {
        printf("\tError: %s", lpMsgBuf);
        LocalFree(lpMsgBuf);
        if (pAddresses)
          FREE(pAddresses);
        return 1;
      }
    }
  }

  if (pAddresses) {
    FREE(pAddresses);
  }
  return 0;
}

int ChangeMulticastMembership(int sockfd, const char* multicast_addr) {
  struct ip_mreq      mreq;
  bzero(&mreq, sizeof(struct ip_mreq));
  std::vector<NetworkInterfaceState> net_inter;
  UpdateNetworkInterface(net_inter);
  for (std::size_t i = 0; i < net_inter.size(); i++) {
    // Find network interface
    bool is_found = false;
    for (std::size_t j = 0; j < network_interfaces_.size(); j++) {
      if (network_interfaces_[j].adapter_name == net_inter[i].adapter_name
          && network_interfaces_[j].ip_addr.S_un.S_addr ==
          net_inter[i].ip_addr.S_un.S_addr
          && network_interfaces_[j].state == INTERFACE_STATE_ADD) {
        is_found = true;
        break;
      }
    }
    if (!is_found) {
      //LOG(L_INFO) << network_interfaces_[i].adapter_name << " : ";
      mreq.imr_interface.s_addr = net_inter[i].ip_addr.S_un.S_addr;
      mreq.imr_multiaddr.s_addr = inet_addr(multicast_addr);
      if (setsockopt(sockfd, IPPROTO_IP, IP_ADD_MEMBERSHIP,
                     (const char *)&mreq, sizeof(struct ip_mreq)) == -1) {
        net_inter[i].state = INTERFACE_STATE_ERROR;
        //LOG(L_ERROR) << "IP_ADD_MEMBERSHIP " << inet_ntoa(net_inter[i].ip_addr);
      } else {
        LOG(L_INFO) << "IP_ADD_MEMBERSHIP " << inet_ntoa(net_inter[i].ip_addr);
        net_inter[i].state = INTERFACE_STATE_ADD;
      }

      unsigned char loop = 0;
      setsockopt(sockfd, IPPROTO_IP, IP_MULTICAST_LOOP,
                 (const char*)&loop, sizeof(loop));
    }
    if (net_inter[i].state != INTERFACE_STATE_NONE) {
      continue;
    }
  }
  network_interfaces_ = net_inter;
  return 0;
}

#endif // def WIN32

int32 CMCastSocket::Open(const char* center_ip, int center_port) {
  const char *multicast_addr = (const char *)center_ip;
  uint16      multicast_port = center_port;

  LOG(L_WARNING) << multicast_addr << " : " << multicast_port;
  sockaddr_in         peeraddr;
#ifdef WIN32
  int                 socklen;
#else
  size_t              socklen;
#endif
  // Init send sock
  sockaddr_in  send_addr;
  send_addr.sin_family = AF_INET;
  send_addr.sin_port = htons(multicast_port);
  send_addr.sin_addr.s_addr = inet_addr(multicast_addr);

  // Create Udp socket that used to recv data
  SOCKET s = socket(AF_INET, SOCK_DGRAM, 0);
  if (s < 0) {
    LOG(L_ERROR) << "socket creating err in udptalk";
    return 1;
  }
  int enable = 1;
  if (setsockopt(s, SOL_SOCKET, SO_REUSEADDR,
                 (const char*)&enable, sizeof(int)) < 0) {
    LOG(L_ERROR) << "SO_REUSEADDR error";
    return 1;
  }

  // Init the multicast address
  socklen = sizeof(struct sockaddr_in);
  memset(&peeraddr, 0, socklen);
  peeraddr.sin_family = AF_INET;
  peeraddr.sin_port = htons(multicast_port);
  peeraddr.sin_addr.s_addr = INADDR_ANY;
  // peeraddr.sin_addr.s_addr = inet_addr("0.0.0.0");
  /* 绑定自己的端口和IP信息到socket上 */
  if (bind(s, (struct sockaddr *) &peeraddr,
           sizeof(struct sockaddr_in)) == -1) {
    LOG(L_ERROR) << "Bind error\n";
    return 1;
  }

  LOG(L_INFO) << "------------START FOUND DEVICES---------------";
#if 0
  ChangeMulticastMembership(s, multicast_addr);
#else
  struct ip_mreq      mreq;
  bzero(&mreq, sizeof(struct ip_mreq));
  mreq.imr_interface.s_addr = INADDR_ANY;
  mreq.imr_multiaddr.s_addr = inet_addr(multicast_addr);
  if (setsockopt(s, IPPROTO_IP, IP_ADD_MEMBERSHIP,
                 (const char *)&mreq, sizeof(struct ip_mreq)) == -1) {
    perror("Add membership error\n");
    // LOG(L_ERROR) << "IP_ADD_MEMBERSHIP " << inet_ntoa(net_inter[i].ip_addr);
  }

  unsigned char loop = 0;
  setsockopt(s, IPPROTO_IP, IP_MULTICAST_LOOP,
             (const char*)&loop, sizeof(loop));
#endif

  /* 循环接收网络上来的组播消息 */
  SetSocket(s);
  evt_recv_.Init(evt_loop_, EvtRecv, this);
  if (evt_recv_.Start(GetSocket(), EVT_READ | EVT_PERSIST) != 0) {
    return -1;
  }
  return 0;
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
  uint8 s_data[2048+1] = {0};
  int32 n_data = vzconn::VSocket::Recv(s_data, 2048, c_remote_addr);
  LOG(L_INFO) << "recv frame "<<n_data;
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

int CMCastSocket::SendUdpData(const char* center_ip, int center_port,
                              const char* pdata, unsigned int ndata) {
  LOG(L_INFO) << (const char *)center_ip << "\t" << center_port;
  struct sockaddr_in scenter;
  scenter.sin_family = AF_INET;
  scenter.sin_port = htons(center_port);
  scenter.sin_addr.s_addr = inet_addr((char*)center_ip);
  int ret = sendto(GetSocket(),
                   (const char*)pdata,
                   ndata,
                   0,
                   (struct sockaddr*)&scenter,
                   sizeof(struct sockaddr));
  return ret;
}
}  // namespace vzconn
