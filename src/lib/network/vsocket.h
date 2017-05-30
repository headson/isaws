/*****************************************************************************
* Copyright (c) 2011,ChengDu ThinkWatch Company
* All rights reserved.
*-----------------------------------------------------------------------------
* Filename      : vsocket.h
* Author        : Sober.Peng
* Date          : 21:12:2016
* Description   :
*-----------------------------------------------------------------------------
* Modify        : 
*-----------------------------------------------------------------------------
******************************************************************************/
#pragma once
#include "vosinc.h"

class Inet_Addr
{
public:
    Inet_Addr();
    Inet_Addr(const Inet_Addr& addr);
    Inet_Addr(const sockaddr_in& addr);
    Inet_Addr(uint32_t ip, uint16_t port);
    Inet_Addr(const string& hostname, uint16_t port);

    virtual ~Inet_Addr();

    static bool is_ipv4(const char *s);

    bool                is_null() const;
    Inet_Addr&          operator=(const Inet_Addr& addr);
    Inet_Addr&          operator=(const sockaddr_in& addr);
    Inet_Addr&          operator=(const string& addr_str);

    void                set_ip(uint32_t ip);
    void                set_ip(const string& hostname);
    void                set_port(uint16_t port);

    uint32_t            get_ip() const;
    uint16_t            get_port() const;
    const sockaddr_in&  ipaddr() const;
    sockaddr_in*        get_addr();
    const sockaddr_in*  get_addr() const;
    string              to_string() const;
    string              ip_to_string() const;

    bool                operator==(const Inet_Addr& addr) const;
    bool                operator!=(const Inet_Addr& addr) const;
    bool                operator>(const Inet_Addr& addr) const;
    bool                operator<(const Inet_Addr& addr) const;

private:
    sockaddr_in     cAddr_;
};

// 
class VSocketCallback : public VObject
{
public:
    VSocketCallback() : VObject() { }
    virtual ~VSocketCallback()    { }

    virtual int32_t  on_message(const void* pData, uint32_t nData, 
        const void* pCtx, const Inet_Addr& cRemoteAddr) { return 0; };
};

class VSocket
{
public:
    VSocket();
    virtual ~VSocket();

    VHANDLER        get_handler() const;
    virtual void    set_handler(VHANDLER handle);

    // 设置回调类
    VSocketCallback* get_callback() const { return _pCallback; }
    virtual void     set_callback(VSocketCallback* vSockProc);

    virtual int32_t open(const Inet_Addr& cLocalAddr, bool nonblocking=false , bool resue=true, bool client=false) { return 0; };
    virtual int32_t close();
    static  int32_t close(VHANDLER& vHdl);

    //绑定地址
    int32_t         bind(const Inet_Addr &cLocalAddr);

    bool            isopen() const;

    //属性设置
    int32_t         set_option(int level, int option, void *optval, int optlen) const;
    int32_t         get_option(int level, int option, void *optval, int *optlen) const;

    //获取SOCKET绑定的地址
    int32_t         get_local_addr(Inet_Addr &cLocalAddr) const;

    /*****************************************************************************
    * Author        : Sober.Peng 28:12:2016
    * Description   : 网络数据接收
    * Param         : pData[OUT] 接收数据，nData[IN] 缓存大小
    * Return        : >0 数据长度，0 没收到数据，-1 断网
    ******************************************************************************/
    virtual int32_t recv(void* pData, uint32_t nData);
    virtual int32_t recv(void* pData, uint32_t nData, Inet_Addr& cRemoteAddr);

    /*****************************************************************************
    * Author        : Sober.Peng 28:12:2016
    * Description   : 网络发送接收
    * Param         : pData[IN] 发送数据，nData[IN] 缓存大小
    * Return        : >0 数据长度，0 没发送数据，-1 断网
    ******************************************************************************/
    virtual int32_t send(const void* pData, uint32_t nData);
    virtual int32_t send(const void* pData, uint32_t nData, const Inet_Addr& cRemoteAddr);

protected:
    VHANDLER            _handler;
    int32_t             _rw_flag;

    VSocketCallback*    _pCallback;     // 包处理
};
