/*****************************************************************************
* Copyright (c) 2011,ChengDu ThinkWatch Company
* All rights reserved.
*-----------------------------------------------------------------------------
* Filename      : vbin_stream.h
* Author        : Sober.Peng
* Date          : 6:2:2017
* Description   : ʵ��Э�����Ķ����ƴ��������,�����ֽ���ģʽ��
*-----------------------------------------------------------------------------
* Modify        : 
*-----------------------------------------------------------------------------
******************************************************************************/
#pragma once
#include "vosinc.h"

#define DEF_PKT_SIZE        2048        // 2K��Ĭ�ϴ�С
#define MAX_STREAM_SIZE     10485760    // 10M

template<class Elem, uint32_t CAPACITY>
class VBinStream_T
{
public:
    typedef typename::VBinStream_T<Elem, CAPACITY> _MyBint;

    VBinStream_T()
    {
        //�ж��ֽ���
        union 
        {
            uint16_t    s16;
            uint8_t     s8[2];
        }un;
        un.s16      = 0x010a;
        big_endian  = (un.s8[0] == 0x01);

        //����Binstream���󻺳���
        size_   = sizeof(Elem) * CAPACITY;
        data_   = (Elem *)malloc(size_);

        rptr_   = (uint8_t *)data_;
        wptr_   = (uint8_t *)data_;

        used_   = 0;
        rsize_  = 0;
    }

    virtual ~VBinStream_T()
    {
        if(data_ != NULL)
        {
            free(data_);
            data_   = NULL;

            rptr_   = NULL;
            wptr_   = NULL;

            size_   = 0;

            used_   = 0;
            rsize_  = 0;
        }
    }

    void        rewind(bool reset = false)     //��λ    
    {
        if(reset)
        {
            wptr_ = (uint8_t *)data_;
            used_ = 0;
        }

        rptr_ = (uint8_t *)data_;
        rsize_ = 0;
    }

    void        resize(uint32_t new_size)      //���������ڴ��С    
    {
        if(new_size <= size_)
            return ;

        uint32_t alloc_size  = size_;
        while(new_size >= alloc_size)
            alloc_size *= 2;

        data_ = (Elem *)realloc(data_, alloc_size);
        size_ = alloc_size;
        wptr_ = (uint8_t *)data_ + used_;
        rptr_ = (uint8_t *)data_ + 	rsize_;
    }

    void        reduce()                       //����ֻ��������״̬�µ���
    {
        if(size_ > 4096) //4KB
        {
            free(data_);

            size_ = sizeof(Elem) * CAPACITY;
            data_ = (Elem *)malloc(size_);

            rptr_ = (uint8_t *)data_;
            wptr_ = (uint8_t *)data_;

            used_ = 0;
            rsize_ = 0;
        }
    }

    //����������Ĵ�С
    void        set_used_size(uint32_t used)
    {
        used_ = used;
    };
    uint32_t    get_used_size() const
    {
        return used_;
    };

    const Elem* get_data_ptr() const
    {
        return data_;
    };

    //��ȡ�������Ĵ�С
    uint32_t    size() const
    {
        return size_;
    };

    uint8_t*    get_wptr()
    {
        return wptr_;
    };

    const uint8_t* get_rptr() const
    {
        return rptr_;
    };

    _MyBint& operator=(const _MyBint& strm)
    {
        resize(strm.size_);
        ::memcpy(data_, strm.data_, strm.size_);
        used_   = strm.used_;
        rsize_  = strm.rsize_;
        rptr_   = data_ + rsize_;
        wptr_   = data_ + used_;

        return *this;
    }

    _MyBint& operator=(const string& sStr)
    {
        rewind(true);
        resize(sStr.size());
        set_used_size(sStr.size());

        ::memcpy(get_wptr(), sStr.data(), sStr.size());
        wptr_ = data_ + used_;

        return *this;
    }

    void push_data(const uint8_t* pData, uint32_t nData)
    {
        resize(used_ + nData);
        ::memcpy((void *)wptr_, (const void *)pData, (size_t)nData);
        wptr_ += nData;
        used_ += nData;
    }

protected:
    Elem*       data_;      //���ݻ�����
    uint8_t*    rptr_;      //��ǰ��λ��ָ��
    uint8_t*    wptr_;      //��ǰдλ��ָ��

    size_t      size_;      //��󻺳����ߴ�
    size_t      used_;      //�Ѿ�ʹ�õĻ������ߴ�
    size_t      rsize_;     //��ȡ���ֽ���

    bool        big_endian; //ϵͳ�ֽ����־
};

typedef VBinStream_T<uint8_t, DEF_PKT_SIZE> VBinStream;


