/************************************************************************/
/* Author      : SoberPeng 2017-06-13
/* Description :
/************************************************************************/
#ifndef VSOCKET_H
#define VSOCKET_H

#include "base/vtypes.h"

#ifdef WIN32
#include <windows.h>
#include <winsock2.h>
#endif

#include <string>

class CInetAddr
{
public:
    CInetAddr();
    CInetAddr(const CInetAddr& addr);
    CInetAddr(const sockaddr_in& addr);
    CInetAddr(uint32_t ip, uint16_t port);
    CInetAddr(const std::string& hostname, uint16_t port);

    virtual ~CInetAddr();

    static bool IsIPV4(const char *s);

    bool                IsNull() const;
    CInetAddr&          operator=(const CInetAddr& addr);
    CInetAddr&          operator=(const sockaddr_in& addr);
    CInetAddr&          operator=(const std::string& addr_str);

    void                SetIP(uint32_t ip);
    void                SetIP(const std::string& hostname);
    void                SetPort(uint16_t port);

    uint32_t            GetIP() const;
    uint16_t            GetPort() const;
    const sockaddr_in&  IPAddr() const;
    sockaddr_in*        GetAddr();
    const sockaddr_in*  GetAddr() const;
    std::string         ToString() const;
    std::string         IP2String() const;

    bool                operator==(const CInetAddr& addr) const;
    bool                operator!=(const CInetAddr& addr) const;
    bool                operator>(const CInetAddr& addr) const;
    bool                operator<(const CInetAddr& addr) const;

private:
    sockaddr_in         c_addr_;
};

class VSocket
{
public:
    VSocket();
    virtual ~VSocket();

    SOCKET          GetSocket() const;
    virtual void    SetSocket(SOCKET handle);

    virtual int32_t Open(const CInetAddr& cLocalAddr, bool nonblocking=false , bool resue=true, bool client=false) { return 0; };
    virtual void    Close();

    //�󶨵�ַ
    int32_t         Bind(const CInetAddr &cLocalAddr);

    bool            IsOpen() const;

    //��������
    int32_t         SetOption(int level, int option, void *optval, int optlen) const;
    int32_t         GetOption(int level, int option, void *optval, int *optlen) const;

    //��ȡSOCKET�󶨵ĵ�ַ
    int32_t         GetLocalAddr(CInetAddr &cLocalAddr) const;

    /*****************************************************************************
    * Author        : Sober.Peng 28:12:2016
    * Description   : �������ݽ���
    * Param         : pData[OUT] �������ݣ�nData[IN] �����С
    * Return        : >0 ���ݳ��ȣ�0 û�յ����ݣ�-1 ����
    ******************************************************************************/
    virtual int32_t Recv(void* pData, uint32_t nData);
    virtual int32_t Recv(void* pData, uint32_t nData, CInetAddr& cRemoteAddr);

    /*****************************************************************************
    * Author        : Sober.Peng 28:12:2016
    * Description   : ���緢�ͽ���
    * Param         : pData[IN] �������ݣ�nData[IN] �����С
    * Return        : >0 ���ݳ��ȣ�0 û�������ݣ�-1 ����
    ******************************************************************************/
    virtual int32_t Send(const void* pData, uint32_t nData);
    virtual int32_t Send(const void* pData, uint32_t nData, const CInetAddr& cRemoteAddr);

protected:
    SOCKET              handler_;
    int32_t             rw_flag_;
};

#endif  // VSOCKET_H
