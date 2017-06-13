/*****************************************************************************
* Copyright (c) 2011,ChengDu ThinkWatch Company
* All rights reserved.
*-----------------------------------------------------------------------------
* Filename      : vsock_accept.cpp
* Author        : Sober.Peng
* Date          : 28:12:2016
* Description   :
*-----------------------------------------------------------------------------
* Modify        : 
*-----------------------------------------------------------------------------
******************************************************************************/
#include "stdafx.h"
#include "vsock_accept.h"

VSockAccept::VSockAccept()
    : VSockStream()
{
}

VSockAccept::~VSockAccept()
{
}

int32_t VSockAccept::open(const Inet_Addr& cLocalAddr, bool nonblocking, bool resue, bool client)
{
    int32_t nRet = 0;
    nRet = VSockStream::open(cLocalAddr, nonblocking, resue, client);
    if (nRet != 0) {
        return nRet;
    }

    nRet = ::listen(get_handler(), MAX_LISTEN);
    if(nRet != 0) {
        VSocket::close();
        return RET_SCTRL_FAILED;
    }

    return RET_SUCCESS;
}

int32_t VSockAccept::accept(VHANDLER& vHdl, Inet_Addr& cRemoteAddr, bool nonblocking)
{
    sockaddr_in addr;
#ifdef WIN32
    int32_t addr_len = sizeof(addr);
#else
    uint32_t addr_len = sizeof(addr);
#endif

    VHANDLER new_socket = ::accept(get_handler(),(sockaddr *)&addr, &addr_len);
    if(new_socket == INVALID_HANDLER) {
        return -1;
    }

    if(nonblocking) {
        set_socket_nonblocking(new_socket);
    }

    vHdl = new_socket;
    cRemoteAddr = addr;
    return 0;
}
