/*****************************************************************************
* Copyright (c) 2011,ChengDu ThinkWatch Company
* All rights reserved.
*-----------------------------------------------------------------------------
* Filename      : vsock_dgram.h
* Author        : Sober.Peng
* Date          : 28:12:2016
* Description   : ∂®“ÂUDP SOCKET¿‡
*-----------------------------------------------------------------------------
* Modify        : 
*-----------------------------------------------------------------------------
******************************************************************************/
#pragma once
#include "vosinc.h"
#include "vsocket.h"

class VSockDgram : public VSocket
{
public:
    VSockDgram();
    virtual ~VSockDgram();

    int32_t open(const Inet_Addr& local_addr, bool nonblocking = false,  bool resue = true, bool client = false);
    
    int32_t set_boardcast(Inet_Addr& cBoardcastAddr, int32_t nPort);

protected:
    void    set_client_option();

private:
    Inet_Addr local_addr_;
};
