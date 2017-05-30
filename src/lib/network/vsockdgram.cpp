/*****************************************************************************
* Copyright (c) 2011,ChengDu ThinkWatch Company
* All rights reserved.
*-----------------------------------------------------------------------------
* Filename      : vsock_dgram.cpp
* Author        : Sober.Peng
* Date          : 4:1:2017
* Description   :
*-----------------------------------------------------------------------------
* Modify        : 
*-----------------------------------------------------------------------------
******************************************************************************/
#include "stdafx.h"
#include "vsock_dgram.h"

VSockDgram::VSockDgram()
    : VSocket()
{
}

VSockDgram::~VSockDgram()
{
}

int32_t VSockDgram::open(const Inet_Addr& local_addr, bool nonblocking, bool resue, bool client)
{
    _handler = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if(INVALID_HANDLER == _handler)
    {
        return -1;
    }
    
#ifdef WIN32 //解决WINSOCK2 的UDP端口ICMP的问题
#if defined(SIO_UDP_CONNRESET)
    int32_t byte_retruned = 0;
    bool new_be = false;
    int32_t status = WSAIoctl(_handler, SIO_UDP_CONNRESET,
        &new_be, sizeof(new_be), NULL, 0, (LPDWORD)&byte_retruned, NULL, NULL);
#endif
#endif

    //设置异步模式
    if(nonblocking) {
        set_socket_nonblocking(_handler);
    }
    set_client_option();

    if(resue)
    {
        //设置端口复用
        int32_t val = 1;
        set_option(SOL_SOCKET, SO_REUSEADDR, &val, sizeof(int32_t));
    }

    int32_t ret = bind(local_addr);
    if(ret == -1)
    {
        return ret;
    }
    return ret;
}

int32_t VSockDgram::set_boardcast(Inet_Addr& cBoardcastAddr, int32_t nPort)
{
    struct sockaddr_in sin;
    memset(&sin, 0, sizeof(sin));

    sin.sin_family = AF_INET;
    sin.sin_port   = htons((u_short)nPort);
    if(sin.sin_port == 0)
    {
        return RET_INVALID_ARG;
    }
    sin.sin_addr.s_addr = INADDR_BROADCAST;
    cBoardcastAddr = sin;

    int32_t opt = 1;
    set_option(SOL_SOCKET, SO_BROADCAST, &opt, sizeof(opt));
    return RET_SUCCESS;
}

void VSockDgram::set_client_option()
{
    int32_t err = 0;
    int32_t opt = 0;

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





