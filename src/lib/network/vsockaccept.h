/*****************************************************************************
* Copyright (c) 2011,ChengDu ThinkWatch Company
* All rights reserved.
*-----------------------------------------------------------------------------
* Filename      : vsock_stream.h
* Author        : Sober.Peng
* Date          : 21:12:2016
* Description   :
*-----------------------------------------------------------------------------
* Modify        : 
*-----------------------------------------------------------------------------
******************************************************************************/
#pragma once
#include "vosinc.h"
#include "vsock_stream.h"

#define MAX_LISTEN  10

class VSockAccept : public VSockStream
{
public:
    VSockAccept();
    virtual ~VSockAccept();

    int32_t open(const Inet_Addr& cLocalAddr, bool nonblocking = false, bool resue = true, bool client = false);

    int32_t accept(VHANDLER& vHdl, Inet_Addr& cRemoteAddr, bool nonblocking=false);
};
