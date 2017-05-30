/*****************************************************************************
* Copyright (c) 2011,ChengDu ThinkWatch Company
* All rights reserved.
*-----------------------------------------------------------------------------
* Filename      : vsockbuffer.h
* Author        : Sober.Peng
* Date          : 2016-3-18
* Description   :
*-----------------------------------------------------------------------------
* Modify        : 
*-----------------------------------------------------------------------------
******************************************************************************/
#pragma once
#include "verror.h"
#include "vheader.h"
#include "vmessage.h"

#include "base/vlock.h"
#include "base/vblockbuffer.h"

//SOCKET接受队列
template<class T, class R, uint32_t CAPACITY>
class CReadBBuffer_T : public VBlockBuffer_T<CAPACITY>
{
public:
    CReadBBuffer_T()
        : VBlockBuffer_T<CAPACITY>()
    {
    };

    virtual ~CReadBBuffer_T()
    {
    };

    //将发送数据递交到发送队列中
    int32_t put(const int8_t* pData, int32_t nData)
    {
        VLockGuard al(vMutex_);
        if(nData <= 0 || nData > MAX_BUFFER_SIZE / 2)
            return -1;

        if((uint32_t)nData > this->free_size())
        {
            if(!this->realloc_buffer(nData))
                return -1;
        }

        memcpy(this->get_wptr(), pData, nData);
        this->move_wptr(nData);
        this->recycle();

        return nData;
    };

    //从接受对象中收取报文
    int32_t recv(R& reciver)
    {
        VLockGuard al(vMutex_);
        if(this->free_size() <= 0) //无空间，尝试开辟更大的空间
        {
            this->realloc_buffer(this->buffer_size_ / 8);
        }

        int32_t nRecv = reciver.Read(this->get_wptr(), this->free_size());
        if(nRecv > 0)
        {
            this->move_wptr(nRecv);
            this->recycle();
        }

        return nRecv;
    };

    //判断一个报文是否收取完毕
    int32_t split(T& trans)
    {
        VLockGuard al(vMutex_);
        while (true)
        {
            TAG_PkgHead* pHead = NULL;
            while (true)
            {
                if (this->used_size() < (uint32_t)HEAD_LENGTH) {
                    break;
                }

                pHead = (TAG_PkgHead*)this->get_rptr();
                if (pHead->nMark == HEAD_MARK)
                {
                    this->recycle();
                    break;
                }
                this->move_rptr(1);
            }

            uint32_t nPkgLen = 0;
            pHead = (TAG_PkgHead*)this->get_rptr();
            if (pHead->nMark == HEAD_MARK)
            {
                if (pHead->eCmdMain == PKG_ENC_STREAM ||
                    pHead->eCmdMain == PKG_TALK_STREAM)
                {
                    nPkgLen = pHead->nPkgLen+HEAD_LEN_STR;
                }
                else if (pHead->eCmdMain%2 == 0)
                {
                    nPkgLen = pHead->nPkgLen+HEAD_LEN_REQ;
                }
                else
                {
                    nPkgLen = pHead->nPkgLen+HEAD_LEN_RET;
                }
            }

            if (nPkgLen > 0 && nPkgLen <= this->used_size())
            {
                int32_t nSend = trans.ReadPacket(this->get_rptr(), nPkgLen);  // 调用处理，但是结尾不是'\0',当字符串处理时注意

                this->move_rptr(nSend);
                this->recycle();
            } 
            else if (nPkgLen > 0 && 
                (this->free_size() <= (nPkgLen - this->used_size()))) 
            {
                uint32_t alloc_size = nPkgLen - this->used_size();
                if(alloc_size > MAX_BUFFER_SIZE)
                    alloc_size = MAX_BUFFER_SIZE;

                this->realloc_buffer(alloc_size);
                break;
            } 
            else 
            {
                break;
            }
        }
        return 0;
    };

private:
    VMutex  vMutex_;
};

//SOCKET发送队列
template<class T, class S, uint32_t CAPACITY>
class CWriteBBuffer_T : public VBlockBuffer_T<CAPACITY>
{
public:
    CWriteBBuffer_T()
        : VBlockBuffer_T<CAPACITY>()
    {
    };
    virtual ~CWriteBBuffer_T()
    {
    };

    //将数据递交到发送对象进行发送
    int32_t send(S& sender)
    {
        VLockGuard al(vMutex_);
        if (this->used_size() <= 0)
            return 0;

        int32_t sender_size = sender.Send(this->get_rptr(), this->used_size());
        if(sender_size > 0)
        {
            this->move_rptr(sender_size);
            this->recycle();
        }

        return sender_size;
    };

    //判断一个报文是否收取完毕
    int32_t split(S& sender)
    {
        VLockGuard al(vMutex_);
        while (true)
        {
            TAG_PkgHead* pHead = NULL;
            while (true)
            {
                if (this->used_size() < (uint32_t)HEAD_LENGTH) {
                    break;
                }

                pHead = (TAG_PkgHead*)this->get_rptr();
                if (pHead->nMark == HEAD_MARK)
                {
                    this->recycle();
                    break;
                }
                this->move_rptr(1);
            }

            uint32_t nPkgLen = 0;
            pHead = (TAG_PkgHead*)this->get_rptr();
            if (pHead->nMark == HEAD_MARK)
            {
                if (pHead->eCmdMain == PKG_ENC_STREAM ||
                    pHead->eCmdMain == PKG_TALK_STREAM)
                {
                    nPkgLen = pHead->nPkgLen+HEAD_LEN_STR;
                }
                else if (pHead->eCmdMain%2 == 0)
                {
                    nPkgLen = pHead->nPkgLen+HEAD_LEN_REQ;
                }
                else
                {
                    nPkgLen = pHead->nPkgLen+HEAD_LEN_RET;
                }
            }

            if (nPkgLen > 0 && nPkgLen <= this->used_size())
            {
                int32_t nSend = sender.Send(this->get_rptr(), nPkgLen);  // 调用处理，但是结尾不是'\0',当字符串处理时注意

                this->move_rptr(nSend);
                this->recycle();
            } 
            else if (nPkgLen > 0 && 
                (this->free_size() <= (nPkgLen - this->used_size()))) 
            {
                uint32_t alloc_size = nPkgLen - this->used_size();
                if(alloc_size > MAX_BUFFER_SIZE)
                    alloc_size = MAX_BUFFER_SIZE;

                this->realloc_buffer(alloc_size);
                break;
            } 
            else 
            {
                break;
            }
        }
        return 0;
    };

    //将发送数据递交到发送队列中
    int32_t put(const int8_t* pData, int32_t nData)
    {
        VLockGuard al(vMutex_);
        if(nData <= 0 || nData > MAX_BUFFER_SIZE / 2)
            return -1;

        if((uint32_t)nData > this->free_size())
        {
            if(!this->realloc_buffer(nData))
                return -1;
        }

        memcpy(this->get_wptr(), pData, nData);
        this->move_wptr(nData);
        this->recycle();

        return nData;
    };

    //直接发送二进制串，std::string类型
    int32_t put(int8_t* pHead, int32_t nHead, int8_t* pBody, int32_t nBody)
    {
        VLockGuard al(vMutex_);
        uint32_t packet_size = nHead + nBody;
        if(packet_size <= 0 || packet_size > MAX_BUFFER_SIZE / 2)
            return -1;

        if(packet_size > this->free_size())
        {
            if(!this->realloc_buffer(packet_size))
                return -1;
        }

        if (put(pHead, nHead) > 0)
        {
            return -1;
        }
        if (put(pBody, nBody) > 0)
        {
            return -1;
        }
        return 0;
    }

public:
    VMutex      vMutex_;
};
