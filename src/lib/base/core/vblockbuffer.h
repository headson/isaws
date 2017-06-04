/*****************************************************************************
* Copyright (c) 2011,ChengDu ThinkWatch Company
* All rights reserved.
*-----------------------------------------------------------------------------
* Filename      : vblock_buffer.h
* Author        : Sober.Peng
* Date          : 26:12:2016
* Description   : ʵ��SOCKET�Ľ��ܺͷ���BUFFER���ƣ���Ҫ���TCP������շ�������,���������30M
*-----------------------------------------------------------------------------
* Modify        : 
*-----------------------------------------------------------------------------
******************************************************************************/
#pragma once
#include "vosinc.h"

//��󻺳��С:30M
#define MAX_BUFFER_SIZE 30*SZ_1M

template<uint32_t CAPACITY>
class VBlockBuffer_T
{
public:
    VBlockBuffer_T()
    {
        read_pos_       = 0;
        write_pos_      = 0;
        buffer_         = (uint8_t *)malloc(CAPACITY * sizeof(uint8_t));
        buffer_size_    = CAPACITY;
    };

    virtual ~VBlockBuffer_T()
    {
        read_pos_ = 0;
        write_pos_ = 0;

        if (buffer_) { free(buffer_); buffer_ = NULL; }
        buffer_size_  = 0;
    };

    bool realloc_buffer(uint32_t size) //����һ��2����BUFFER
    {
        //ʣ��ռ乻��
        if(size < free_size() || buffer_size_ > MAX_BUFFER_SIZE)
            return false;

        uint32_t buffer_size = buffer_size_ * 2;
        while(buffer_size -  used_size() < size) //�����µ�ʣ��ռ��Ƿ���
        {
            buffer_size = buffer_size * 2;
        }

        buffer_ = (uint8_t *)realloc(buffer_, buffer_size);
        buffer_size_ = buffer_size;
        return true;
    }

    void reset()
    {
        if(buffer_size_ > CAPACITY)
        {
            free(buffer_);

            buffer_ = (uint8_t *)malloc(CAPACITY * sizeof(uint8_t));
            buffer_size_ = CAPACITY;
        }

        memset(buffer_, 0x00, buffer_size_);

        read_pos_ = 0;
        write_pos_ = 0;
    }

    //��ȡ��λ��
    uint8_t* get_rptr()
    {
        return buffer_ + read_pos_;
    };
    //�ƶ���λ��
    void move_rptr(uint32_t n)
    {
        read_pos_ += n;
    };
    //��ȡдλ��
    uint8_t* get_wptr()
    {
        return buffer_ + write_pos_;
    };
    //�ƶ�дλ��
    void move_wptr(uint32_t n)
    {
        write_pos_ += n;
    };

    //BUFFER��������С
    uint32_t length() const
    {
        return buffer_size_;
    };
    //���ݳ���
    uint32_t used_size()
    {
        if(write_pos_ > read_pos_) {
            return write_pos_ - read_pos_;
        } else {
            recycle();
            return 0;
        }
    };
    //ʣ�໺��������
    uint32_t free_size() const
    {
        return buffer_size_ - write_pos_;
    };

    bool is_full() const
    {
        return (free_size() == 0);
    };

    void recycle()
    {
        if(write_pos_ <= read_pos_)
        {
            write_pos_ = 0;
            read_pos_ = 0;
        }
        else if(read_pos_ > 0)
        {
            memmove(buffer_, buffer_ + read_pos_, write_pos_ - read_pos_);
            write_pos_ = write_pos_ - read_pos_;
            read_pos_ = 0;
        }
    };

protected:
    uint8_t*    buffer_;
    uint32_t    buffer_size_;

    uint32_t    read_pos_;
    uint32_t    write_pos_;
};

