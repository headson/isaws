/************************************************************************/
/* 作者: SoberPeng 17-05-23
/* 描述: 
/************************************************************************/
#ifndef LIBPLATFORM_V4L2_H
#define LIBPLATFORM_V4L2_H

#ifdef __cplusplus
extern "C" {
#endif
#include "videodev2.h"
#ifdef __cplusplus
}
#endif

#include <sys/mman.h>
#include <sys/ioctl.h>

class CV4l2
{
public:
    #define REQBUFF_NUM 3

public:
    CV4l2()
    {
        vdo_fd = 0;

        sVideo = "";
        nInput = 0;
        
        nWidth = 720;
        nHeight= 576;
        
        nFrmFreq = 30;
        nFrmLost = 1;

        memset(sBuffer, 0, REQBUFF_NUM*sizeof(struct TAG_V4L2_BUFFER));
    }

    // V4L打开
    int32_t v4l_open()
    {
        int i, mode = 0;
        struct v4l2_format          fmt;
        struct v4l2_streamparm      parm;
        struct v4l2_requestbuffers  req;
        struct v4l2_control         ctl;
        struct v4l2_crop            crop;
        struct v4l2_dbg_chip_ident  chip;
        struct v4l2_frmsizeenum     fsize;

        if ((vdo_fd = open(sVideo.c_str(), O_RDWR, 0)) < 0)
        {
            loge("Unable to open %s.", sVideo.c_str());
            return RET_FOPEN_FAILED;
        }

        if (ioctl(vdo_fd, VIDIOC_DBG_G_CHIP_IDENT, &chip))
        {
            loge("VIDIOC_DBG_G_CHIP_IDENT failed %s.", sVideo.c_str());
            close(vdo_fd); vdo_fd = 0;
            return RET_EIOCTL_FAILED;
        }

#if 1
        for (i = 0; i < 10; i++)
        {
            fsize.index = i;
            if (ioctl(vdo_fd, VIDIOC_ENUM_FRAMESIZES, &fsize))
            {
                break;
            }
            else
            {
                if ((nWidth == fsize.discrete.width) 
                    && (nHeight == fsize.discrete.height))
                    mode = i;
            }
        }
        if (mode == -1)
        {
            loge("Not support the resolution in camera %s.", sVideo.c_str());
            close(vdo_fd); vdo_fd = 0;
            return RET_INVALID_ARG;
        }

        parm.type                                   = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        parm.parm.capture.timeperframe.numerator    = 1;
        parm.parm.capture.timeperframe.denominator  = nFrmFreq;// 帧率
        parm.parm.capture.capturemode               = mode;
        if (ioctl(vdo_fd, VIDIOC_S_PARM, &parm) < 0)
        {
            loge("set frame rate failed");
            close(vdo_fd); vdo_fd = 0;
            return RET_EIOCTL_FAILED;
        }
#endif

        ctl.value = 0;
        ctl.id = V4L2_CID_PRIVATE_BASE;
        if (ioctl(vdo_fd, VIDIOC_S_CTRL, &ctl) < 0)
        {
            loge("set control failed");
            close(vdo_fd); vdo_fd = 0;
            return RET_EIOCTL_FAILED;
        }

        if (ioctl(vdo_fd, VIDIOC_S_INPUT, &nInput) < 0)
        {
            loge("VIDIOC_S_INPUT failed");
            close(vdo_fd); vdo_fd = 0;
            return RET_EIOCTL_FAILED;
        }

        crop.type       = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        crop.c.width    = nWidth;
        crop.c.height   = nHeight;
        crop.c.top      = 0;
        crop.c.left     = 0;
        if (ioctl(vdo_fd, VIDIOC_S_CROP, &crop) < 0)
        {
            loge("VIDIOC_S_CROP failed");
            close(vdo_fd); vdo_fd = 0;
            return RET_EIOCTL_FAILED;
        }

        fmt.type                    = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        fmt.fmt.pix.width           = nWidth;
        fmt.fmt.pix.height          = nHeight;
        fmt.fmt.pix.bytesperline    = nWidth;
        fmt.fmt.pix.priv            = 0;
        fmt.fmt.pix.sizeimage       = 0;
        fmt.fmt.pix.pixelformat     = V4L2_PIX_FMT_YUV420;
        if (ioctl(vdo_fd, VIDIOC_S_FMT, &fmt) < 0)
        {
            loge("set format failed");
            close(vdo_fd); vdo_fd = 0;
            return RET_EIOCTL_FAILED;
        }

        memset(&req, 0, sizeof(req));
        req.count   = REQBUFF_NUM;
        req.memory  = V4L2_MEMORY_MMAP;
        req.type    = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        if (ioctl(vdo_fd, VIDIOC_REQBUFS, &req) < 0)
        {
            loge("v4l_capture_setup: VIDIOC_REQBUFS failed");
            close(vdo_fd); vdo_fd = 0;
            return RET_EIOCTL_FAILED;
        }
        logd("Video=%s, mode=%d, w=%d, h=%d, input=%d.", 
            sVideo.c_str(), mode, nWidth, nHeight, nInput);
        return RET_SUCCESS;
    }

    // V4L关闭
    void v4l_close()
    {
        if (vdo_fd > 0)
        {
            close(vdo_fd); 
            vdo_fd = 0;
        }
    }

    // V4l2开始抓拍
    int32_t v4l_start_capturing()
    {
        int i = 0;
        struct v4l2_buffer buf;
        enum v4l2_buf_type type;

        if (vdo_fd <= 0) 
            return RET_INVALID_HDL;

        for (i = 0; i < REQBUFF_NUM; i++)
        {
            memset(&buf, 0, sizeof(buf));
            buf.index   = i;
            buf.memory  = V4L2_MEMORY_MMAP;
            buf.type    = V4L2_BUF_TYPE_VIDEO_CAPTURE;
            if (ioctl(vdo_fd, VIDIOC_QUERYBUF, &buf) < 0)
            {
                loge("VIDIOC_QUERYBUF error %s", sVideo.c_str());

                v4l_stop_capturing();
                close(vdo_fd); vdo_fd = 0;
                return RET_EIOCTL_FAILED;
            }

            sBuffer[i].length = buf.length;
            sBuffer[i].offset = (size_t) buf.m.offset;
            sBuffer[i].start  = mmap(NULL, sBuffer[i].length, PROT_READ | PROT_WRITE, MAP_SHARED, vdo_fd, sBuffer[i].offset);
            if (sBuffer[i].start == NULL)
            {
                loge("%s mmap start is failed.", sVideo.c_str());

                v4l_stop_capturing();
                close(vdo_fd); vdo_fd = 0;
                return RET_MEMORY_LEAK;
            }
        }

        for (i = 0; i < REQBUFF_NUM; i++)
        {
            memset(&buf, 0, sizeof(buf));
            buf.index       = i;
            buf.memory      = V4L2_MEMORY_MMAP;
            buf.m.offset    = sBuffer[i].offset;
            buf.type        = V4L2_BUF_TYPE_VIDEO_CAPTURE;
            if (ioctl(vdo_fd, VIDIOC_QBUF, &buf) < 0)
            {
                loge("%s VIDIOC_QBUF error", sVideo.c_str());

                v4l_stop_capturing();
                close(vdo_fd); vdo_fd = 0;
                return RET_EIOCTL_FAILED;
            }
        }

        type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        if (ioctl(vdo_fd, VIDIOC_STREAMON, &type) < 0)
        {
            loge("%s VIDIOC_STREAMON error", sVideo.c_str());

            v4l_stop_capturing();
            close(vdo_fd); vdo_fd = 0;
            return RET_EIOCTL_FAILED;
        }
        return RET_SUCCESS;
    }

    // V4l2停止抓拍
    void v4l_stop_capturing()
    {
        int32_t i = 0;

        enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

        if (vdo_fd > 0)
            ioctl(vdo_fd, VIDIOC_STREAMOFF, &type);
        else
            return;

        for (i = 0; i < REQBUFF_NUM; i++)
        {
            if (sBuffer[i].start)
            {
                munmap(sBuffer[i].start, sBuffer[i].length);
                sBuffer[i].length = 0;
                sBuffer[i].offset = 0;
                sBuffer[i].start = NULL;
            }
        }
    }

    // V4l2从输出队列取得一帧数据
    int32_t v4l_get_capture_data(struct v4l2_buffer *buf)
    {
        if (vdo_fd > 0 && buf != NULL)
        {
            memset(buf, 0, sizeof(struct v4l2_buffer));
            buf->type   = V4L2_BUF_TYPE_VIDEO_CAPTURE;
            buf->memory = V4L2_MEMORY_MMAP;
            if (ioctl(vdo_fd, VIDIOC_DQBUF, buf) < 0)
            {
                loge("%s VIDIOC_DQBUF error", sVideo.c_str());

                v4l_stop_capturing();
                close(vdo_fd); vdo_fd = 0;
                return RET_EIOCTL_FAILED;
            }

            return RET_SUCCESS;
        }
        loge("v4l_get_capture_data: %d.", vdo_fd);
        return RET_INVALID_HDL;
    }

    // V4l2投放空缓存到视频输入队列
    void v4l_put_capture_data(struct v4l2_buffer *buf)
    {
        if (vdo_fd > 0) ioctl(vdo_fd, VIDIOC_QBUF, buf);
    }

public:
    int         vdo_fd;
    std::string sVideo;         // 视频地址

    uint16_t    nWidth, nHeight;    // 宽，高
    int32_t     nInput;             // MEM,IC_MEM(IPU预处理)

    int32_t     nFrmFreq;           // 帧率
    int32_t     nFrmLost;           // 丢帧

    struct TAG_V4L2_BUFFER 
    {
        void*    start;  // 虚拟地址
        size_t   offset; // 
        uint32_t  length; // 长度
    } sBuffer[REQBUFF_NUM];
};

#endif  // LIBPLATFORM_V4L2_H
