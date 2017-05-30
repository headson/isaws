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
#include "vsocket.h"

typedef void (*Callback)(int32_t eType, int8_t *pData, int32_t nData, const void* pArg);

class VSockStream : public VSocket
{
public:
    VSockStream();
    virtual ~VSockStream();

    int32_t open(const Inet_Addr& cLocalAddr, bool nonblocking = false, bool resue = true, bool client = false);
    
    //获得远端的对应IP地址,一般TCP有效
    int32_t get_remote_addr(Inet_Addr &cRemoteAddr) const;

    void    set_client_option();

public:
    /*****************************************************************************
    * Author        : Sober.Peng 21:12:2016
    * Description   : 连接服务器
    * Param         : 
    * Return        : 
    ******************************************************************************/
    static int32_t  connect_to_server(VHANDLER& vHdl, const Inet_Addr& cRemoteAddr, int32_t nTimeout=5000);

};
