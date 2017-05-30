#include "vsocket.h"

Inet_Addr::Inet_Addr()
{
    memset(&cAddr_, 0, sizeof(struct sockaddr_in));
    cAddr_.sin_family = AF_INET;
    cAddr_.sin_addr.s_addr = htonl(INADDR_ANY);
    cAddr_.sin_port = htons(0);
}

Inet_Addr::Inet_Addr(const sockaddr_in& addr)
{
    memcpy(&cAddr_, &addr, sizeof(struct sockaddr_in));
}

Inet_Addr::Inet_Addr(uint32_t ip, uint16_t port)
{
    memset(&cAddr_, 0, sizeof(struct sockaddr_in));

    set_ip(ip);
    set_port(port);

    cAddr_.sin_family = AF_INET;
}

Inet_Addr::Inet_Addr(const Inet_Addr &addr)
{
    cAddr_ = addr.cAddr_;
}

Inet_Addr::Inet_Addr(const string& hostname, uint16_t port)
{
    memset(&cAddr_, 0, sizeof(struct sockaddr_in));

    set_ip(hostname);
    set_port(port);

    cAddr_.sin_family = AF_INET;
}

Inet_Addr::~Inet_Addr()
{
}

bool Inet_Addr::is_ipv4( const char *s )
{
    bool rv = true;
    int32_t tmp1, tmp2, tmp3, tmp4, i;

    while(true)
    {
        i = sscanf(s, "%d.%d.%d.%d", &tmp1, &tmp2, &tmp3, &tmp4);

        if(i != 4)
        {
            rv = false;
            break;
        }

        if((tmp1 > 255) || (tmp2 > 255) || (tmp3 > 255) || (tmp4 > 255))
        {
            rv = false;
            break;
        }

        const char *pChar;
        for(pChar = s; *pChar != 0; pChar++)
        {
            if((*pChar != '.') && ((*pChar < '0') || (*pChar > '9')))
            {
                rv = false;
                break;
            }
        }
        break;
    }

    return rv;
}

bool Inet_Addr::is_null() const
{
    return (ntohs(cAddr_.sin_port) == 0);
}

Inet_Addr& Inet_Addr::operator = (const Inet_Addr& addr)
{
    this->cAddr_ = addr.cAddr_;
    return *this;
}

Inet_Addr& Inet_Addr::operator =(const sockaddr_in &addr)
{
    this->cAddr_ = addr;
    return *this;
}

Inet_Addr& Inet_Addr::operator =(const std::string &addr_str)
{
    std::string::size_type pos = addr_str.find(':');
    if(std::string::npos == pos)
    {
        return *this;
    }

    set_port(static_cast<uint16_t>(strtoul(addr_str.substr(pos + 1).c_str(), NULL, 10)));
    set_ip(addr_str.substr(0, pos));

    return *this;
}

void Inet_Addr::set_ip(uint32_t ip)
{
    cAddr_.sin_addr.s_addr = htonl(ip);
}

void Inet_Addr::set_ip(const std::string &hostname)
{
    cAddr_.sin_addr.s_addr = inet_addr(hostname.c_str());
    if (cAddr_.sin_addr.s_addr == (uint32_t)-1) //地址为0.0.0.0，无效地址 
    {
        if (hostent * pHost = gethostbyname(hostname.c_str())) 
        {
            cAddr_.sin_addr.s_addr = (*reinterpret_cast<uint32_t *>(pHost->h_addr_list[0]));
        } 
    }
}

void Inet_Addr::set_port(uint16_t port)
{
    cAddr_.sin_port = htons(port);
}

uint32_t Inet_Addr::get_ip() const
{
    return ntohl(cAddr_.sin_addr.s_addr);
}

uint16_t Inet_Addr::get_port() const
{
    return ntohs(cAddr_.sin_port);
}

const sockaddr_in& Inet_Addr::ipaddr() const
{
    return cAddr_;
}

sockaddr_in* Inet_Addr::get_addr()
{
    return &cAddr_;
}

const sockaddr_in* Inet_Addr::get_addr() const
{
    return &cAddr_;
}

std::string Inet_Addr::to_string() const
{
    char ip_str[32] = {0};
#ifdef WIN32
    sprintf(ip_str, "%d.%d.%d.%d", cAddr_.sin_addr.S_un.S_un_b.s_b1,
        cAddr_.sin_addr.S_un.S_un_b.s_b2,
        cAddr_.sin_addr.S_un.S_un_b.s_b3,
        cAddr_.sin_addr.S_un.S_un_b.s_b4);
#else
    ::inet_ntop(cAddr_.sin_family, &(cAddr_.sin_addr), ip_str, sizeof(ip_str));
#endif

    sprintf(ip_str, "%s:%d", ip_str, get_port());
    return ip_str;
}

std::string Inet_Addr::ip_to_string() const
{
    char ip_str[32] = {0};
    
#ifdef WIN32
    sprintf(ip_str, "%d.%d.%d.%d", cAddr_.sin_addr.S_un.S_un_b.s_b1,
        cAddr_.sin_addr.S_un.S_un_b.s_b2,
        cAddr_.sin_addr.S_un.S_un_b.s_b3,
        cAddr_.sin_addr.S_un.S_un_b.s_b4);
#else
    ::inet_ntop(cAddr_.sin_family, &(cAddr_.sin_addr), ip_str, sizeof(ip_str));
#endif

    return ip_str;
}

bool Inet_Addr::operator ==(const Inet_Addr& addr) const
{
    return (ntohl(cAddr_.sin_addr.s_addr) == ntohl(addr.cAddr_.sin_addr.s_addr)
        && ntohs(cAddr_.sin_port) == ntohs(addr.cAddr_.sin_port));
}

bool Inet_Addr::operator !=(const Inet_Addr &addr) const
{
    return !((*this) == addr);
}

bool Inet_Addr::operator <(const Inet_Addr &addr) const
{
    if(ntohl(cAddr_.sin_addr.s_addr) < ntohl(addr.cAddr_.sin_addr.s_addr))
        return true;
    else if(ntohl(cAddr_.sin_addr.s_addr) > ntohl(addr.cAddr_.sin_addr.s_addr))
        return false;
    else
    {
        return (ntohs(cAddr_.sin_port) < ntohs(addr.cAddr_.sin_port) ? true : false);
    }
}

bool Inet_Addr::operator >(const Inet_Addr& addr) const
{
    if(ntohl(cAddr_.sin_addr.s_addr) > ntohl(addr.cAddr_.sin_addr.s_addr))
        return true;
    else if(ntohl(cAddr_.sin_addr.s_addr) < ntohl(addr.cAddr_.sin_addr.s_addr))
        return false;
    else
    {
        return (ntohs(cAddr_.sin_port) > ntohs(addr.cAddr_.sin_port) ? true : false);
    }
}

std::ostream& operator<<(std::ostream& os, const Inet_Addr& addr)
{
    os << addr.to_string();
    return os;
}

//////////////////////////////////////////////////////////////////////////
VSocket::VSocket() 
    : _handler(INVALID_HANDLER)
    , _rw_flag(0)
    , _pCallback(NULL)
{
#ifndef WIN32
    _rw_flag = MSG_NOSIGNAL | MSG_DONTWAIT;
#endif
}

VSocket::~VSocket()
{
    close();
}

VHANDLER VSocket::get_handler() const
{
    return _handler;
}

void VSocket::set_handler(VHANDLER handle)
{
    _handler = handle;
}

void VSocket::set_callback(VSocketCallback* vSockProc)
{
    _pCallback = vSockProc;
}

int32_t VSocket::close()
{
    return close(_handler);
}

int32_t VSocket::close(VHANDLER& vHdl)
{
    if(vHdl != INVALID_HANDLER)
    {
        closesocket(vHdl);

        vHdl = INVALID_HANDLER;
    }
    return 0;
}

bool VSocket::isopen() const
{
    return (_handler != INVALID_HANDLER);
}

int32_t VSocket::set_option(int level, int option, void *optval, int optlen) const
{
    return setsockopt(get_handler(), level, option, (char *)optval, optlen);
}

int32_t VSocket::get_option(int level, int option, void *optval, int *optlen) const
{
#ifdef WIN32
    return getsockopt(get_handler(), level, option, (char *)optval, optlen);
#else
    return getsockopt(get_handler(), level, option, (char *)optval, (socklen_t *)optlen);
#endif
}

int32_t VSocket::get_local_addr(Inet_Addr &cLocalAddr) const
{
#ifdef WIN32
    int32_t len = sizeof(sockaddr_in);
#else
    uint32_t len = sizeof(sockaddr_in);
#endif

    sockaddr *addr = reinterpret_cast<sockaddr *> (cLocalAddr.get_addr());
    if(::getsockname(get_handler(), addr, &len) == -1)
        return -1;

    return 0;
}

int32_t VSocket::bind(const Inet_Addr &cLocalAddr)
{
    int32_t ret = ::bind(_handler, 
        (struct sockaddr *)cLocalAddr.get_addr(), sizeof(sockaddr_in));
    if(ret != 0)
    {
        VSocket::close();
        return -1;
    }

    return 0;
}

/*****************************************************************************
* Author        : Sober.Peng 28:12:2016
* Description   : 网络数据接收
* Param         : pData[OUT] 接收数据，nData[IN] 缓存大小
* Return        : >0 数据长度，0 没收到数据，-1 断网
******************************************************************************/
int32_t VSocket::recv(void *pData, uint32_t nData)
{
    int32_t nRet = -1;
    if(!isopen()) {
        return nRet;
    }

    nRet = ::recv(get_handler(), (char *)pData, nData, _rw_flag);
    if (nRet > 0) {
        return nRet;
    }

    if (nRet < 0 && error_no() == XEAGAIN) 
    {
        return 0;
    }

    return -1;
}

/*****************************************************************************
* Author        : Sober.Peng 28:12:2016
* Description   : 网络数据接收
* Param         : pData[OUT] 接收数据，nData[IN] 缓存大小
* Return        : >0 数据长度，0 没收到数据，-1 断网
******************************************************************************/
int32_t VSocket::recv(void* pData, uint32_t nData, Inet_Addr& cRemoteAddr)
{
    int32_t nRet = -1;
    if(!isopen()) {
        return nRet;
    }

#ifdef WIN32
    int32_t from_size = sizeof(struct sockaddr_in);
#else
    uint32_t from_size = sizeof(struct sockaddr_in);
#endif

    nRet = ::recvfrom(get_handler(), (char *)pData, nData, 
        _rw_flag, (struct sockaddr *)cRemoteAddr.get_addr(), &from_size);
    if (nRet > 0) {
        return nRet;
    }

    if (nRet < 0 && error_no() == XEAGAIN) 
    {
        return 0;
    }

    return -1;
}

/*****************************************************************************
* Author        : Sober.Peng 28:12:2016
* Description   : 网络发送接收
* Param         : pData[IN] 发送数据，nData[IN] 缓存大小
* Return        : >0 数据长度，0 没发送数据，-1 断网
******************************************************************************/
int32_t VSocket::send(const void *buf, uint32_t buf_size)
{
    int32_t nRet = -1;
    if(!isopen()) {
        return nRet;
    }

    nRet = ::send(get_handler(), (const char *)buf, buf_size, _rw_flag);
    if (nRet > 0) {
        return nRet;
    }

    if (nRet < 0 && error_no() == XEAGAIN) 
    {
        return 0;
    }

    return -1;
}

/*****************************************************************************
* Author        : Sober.Peng 28:12:2016
* Description   : 网络发送接收
* Param         : pData[IN] 发送数据，nData[IN] 缓存大小
* Return        : >0 数据长度，0 没发送数据，-1 断网
******************************************************************************/
int32_t VSocket::send(const void* buf, uint32_t buf_size, const Inet_Addr& remote_addr)
{
    int32_t nRet = -1;
    if(!isopen()) {
        return nRet;
    }

    nRet = ::sendto(get_handler(), (const char *)buf, buf_size, 
        _rw_flag, (struct sockaddr *)remote_addr.get_addr(), sizeof(sockaddr_in));
    if (nRet > 0) {
        return nRet;
    }

    if (nRet < 0 && error_no() == XEAGAIN) 
    {
        return 0;
    }

    return -1;
}

