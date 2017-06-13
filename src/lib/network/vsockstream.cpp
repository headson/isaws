/*****************************************************************************
* Copyright (c) 2011,ChengDu ThinkWatch Company
* All rights reserved.
*-----------------------------------------------------------------------------
* Filename      : vsock_stream.cpp
* Author        : Sober.Peng
* Date          : 4:1:2017
* Description   :
*-----------------------------------------------------------------------------
* Modify        : 
*-----------------------------------------------------------------------------
******************************************************************************/
#include "stdafx.h"
#include "vsock_stream.h"

VSockStream::VSockStream()
    : VSocket()
{
}

VSockStream::~VSockStream()
{
}

int32_t VSockStream::open(const Inet_Addr& cLocalAddr, bool nonblocking, bool resue, bool client)
{
    _handler = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(_handler == INVALID_HANDLER)
    {
        return RET_SOCK_FAILED;
    }

    if(nonblocking) { //�����첽SOCKET
        set_socket_nonblocking(_handler);
    }
    set_client_option();
    Inet_Addr cLAddr = cLocalAddr;

    //����һ����������
    if(!cLAddr.is_null())
    {
        //���ö˿ڸ���
        if(resue)
        {
            int32_t val = 1;
            set_option(SOL_SOCKET, SO_REUSEADDR, &val, sizeof(int32_t));
        }

        if(client)
        {
            uint16_t port = cLAddr.get_port();
            uint32_t count = 0;
            while(this->bind(cLAddr) != 0) //һֱ�󶨣�ֱ���ɹ�
            {
                count ++;
                if(count > 20000) //�������1000��
                {
                    VSocket::close();
                    return RET_SCTRL_FAILED;
                }

                port ++;
                cLAddr.set_port(port);
            }
        }
        else
        {
            if(this->bind(cLAddr) != 0)
            {
                VSocket::close();
                return RET_SCTRL_FAILED;
            }
        }
    }

    return RET_SUCCESS;
}

int32_t VSockStream::get_remote_addr(Inet_Addr &remote_addr) const
{
    if(!isopen())
        return -1;
#ifdef WIN32
    int32_t len = sizeof(sockaddr_in);
#else
    uint32_t len = sizeof(sockaddr_in);
#endif

    sockaddr* addr = reinterpret_cast<sockaddr *>(remote_addr.get_addr());
    if(::getpeername(get_handler(), addr, &len) == -1)
    {
        return RET_SCTRL_FAILED;
    }

    return RET_SUCCESS; 
}

//////////////////////////////////////////////////////////////////////////
int32_t VSockStream::connect_to_server(VHANDLER& vHdl, const Inet_Addr& cRemoteAddr, int32_t nTimeout)
{
    vHdl = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(vHdl == INVALID_HANDLER) 
    {
        return RET_SOCK_FAILED;
    }
    int32_t val = 1;
    setsockopt(vHdl, SOL_SOCKET, SO_REUSEADDR, (char *)&val, sizeof(int32_t));

    set_socket_nonblocking(vHdl);
    int32_t nRet = connect(vHdl, (const sockaddr*)cRemoteAddr.get_addr(), sizeof(sockaddr_in));
    if (nRet == 0)
    {
        return RET_SUCCESS;
    }
    else 
    {
        if (error_no() == XEINPROGRESS)
        {
            fd_set fdw, fdr, efds;
            FD_ZERO(&fdw); FD_SET(vHdl, &fdw);

            fdr = efds = fdw;
            struct timeval tv;
            tv.tv_sec  = nTimeout / 1000;
            tv.tv_usec = (nTimeout % 1000)*1000;
            nRet = select(vHdl+1, &fdr, &fdw, &efds, &tv);
            if (nRet > 0)
            {
#ifndef WIN32
                int nError = 0;
                socklen_t nLen = sizeof(nError);
                getsockopt(vHdl, SOL_SOCKET, SO_ERROR, (char*)&nError, &nLen);
                if (nError == 0) {
                    return RET_SUCCESS;
                }
#else
                if (FD_ISSET(vHdl, &fdw)) {
                    return RET_SUCCESS;
                }
#endif
            }
        }
    }
    VSocket::close(vHdl);
    return RET_SCONN_FAILED;
}

void VSockStream::set_client_option()
{
    int32_t err = 0;
    int32_t opt = 0;

    // keep live
    opt = 1;
    err = set_option(SOL_SOCKET, SO_KEEPALIVE, &opt, sizeof(int32_t));
    if(err < 0)
    {
        printf("setsockopt SO_KEEPALIVE error: 0x%x", errno);
        return;
    }

#ifndef WIN32
    opt = 30;// ���������30�������κ����������������̽��
    set_option(IPPROTO_TCP, TCP_KEEPIDLE, &opt, sizeof(opt));

    opt = 5; // ̽��ʱ����ʱ����Ϊ5��
    set_option(IPPROTO_TCP, TCP_KEEPINTVL, &opt, sizeof(opt));

    opt = 3; // ̽�Ⳣ�Դ����������һ��̽������յ���Ӧ�ˣ�������εĲ��ٷ�
    set_option(IPPROTO_TCP, TCP_KEEPCNT, &opt, sizeof(opt));
#endif

    struct linger ling = {1, 1};
    err = set_option(SOL_SOCKET, SO_LINGER, &ling, sizeof(struct linger));
    if(err < 0)
    {
        printf("setsockopt SO_LINGER error: 0x%x", errno);
        return;
    }

#if defined(SOL_TCP) && defined(TCP_NODELAY)
    opt = 1;
    err = set_option(SOL_TCP, TCP_NODELAY, &opt, sizeof(int32_t));
    if(err < 0)
    {
        printf("setsockopt TCP_NODELAY error: 0x%x", errno);
        return;
    }
#else
    opt = 1;
    err = set_option(IPPROTO_TCP, TCP_NODELAY, &opt, sizeof(int32_t));
    if(err < 0)
    {
        printf("setsockopt TCP_NODELAY error: 0x%x", errno);
        return;
    }
#endif

    opt = 64 * 1024;
    err = set_option(SOL_SOCKET, SO_SNDBUF, &opt, sizeof(int32_t));
    if(err < 0)
    {
        printf("setsockopt SO_SNDBUF error: 0x%x", errno);
        return;
    }

    opt = 64 * 1024;
    err = set_option(SOL_SOCKET, SO_RCVBUF, &opt, sizeof(int32_t));
    if(err < 0)
    {
        printf("setsockopt SO_RCVBUF error: 0x%x", errno);
        return;
    }

    struct timeval nTmOut = {1, 0};
    err = set_option(SOL_SOCKET, SO_RCVTIMEO, &nTmOut, sizeof(struct timeval));
    if(err < 0)
    {
        printf("setsockopt SO_RCVTIMEO error: 0x%x", errno);
        return;
    }

    nTmOut.tv_sec = 0;
    nTmOut.tv_usec = 40*1000;
    err = set_option(SOL_SOCKET, SO_SNDTIMEO, &nTmOut, sizeof(struct timeval));
    if(err < 0)
    {
        printf("setsockopt SO_SNDTIMEO error: 0x%x", errno);
        return;
    }
}
