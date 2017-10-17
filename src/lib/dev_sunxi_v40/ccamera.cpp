/************************************************************************
*Author      : Sober.Peng 17-10-17
*Description :
************************************************************************/
#include "ccamera.h"
#include "videodev2.h"

#include <sys/mman.h>
#include <sys/ioctl.h>

#include "vzbase/helper/stdafx.h"

CCamera::CCamera(const char *dev_name, int width, int height) {
  vdo_fd_   = 0;
  dev_name_ = dev_name;
  cap_w_    = width;
  cap_h_    = height;
  cap_fps_  = 30;
  buf_cnt_  = 10;
  cap_fmt_  = V4L2_PIX_FMT_NV12;
}

CCamera::~CCamera() {
}

CCamera *CCamera::Create(const char *dev_name, int width, int height) {
  if (dev_name == NULL || width <= 0 || height <= 0) {
    LOG(L_ERROR) << "param is failed.";
    return NULL;
  }

  CCamera *cam = new CCamera(dev_name, width, height);
  if (cam) {
    return cam;
  }
  return NULL;
}

int CCamera::capture_init() {
  struct v4l2_input inp;
  struct v4l2_streamparm parms;
  struct v4l2_format fmt;
  struct v4l2_requestbuffers req;
  struct v4l2_buffer buf;

  if ((vdo_fd_ = open(dev_name_.c_str(), O_RDWR, S_IRWXU)) < 0) {
    LOG_ERROR("can't open %s(%s)\n", dev_name_.c_str(), strerror(errno));
    return -1;
  }
  fcntl(vdo_fd_, F_SETFD, FD_CLOEXEC);

  /* set capture input */
  inp.index = 0;
  inp.type = V4L2_INPUT_TYPE_CAMERA;
  if (ioctl(vdo_fd_, VIDIOC_S_INPUT, &inp) == -1) {
    LOG_ERROR("VIDIOC_S_INPUT failed! s_input: %d\n", inp.index);
    return -1;
  }

  /* set capture parms */
  parms.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  parms.parm.capture.timeperframe.numerator = 1;
  parms.parm.capture.timeperframe.denominator = cap_fps_;
  if (ioctl(vdo_fd_, VIDIOC_S_PARM, &parms) == -1) {
    LOG_ERROR("VIDIOC_S_PARM failed!\n");
    return -1;
  }

  /* get and print capture parms */
  memset(&parms, 0, sizeof(struct v4l2_streamparm));
  parms.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  if (ioctl(vdo_fd_, VIDIOC_G_PARM, &parms) == 0) {
    LOG_INFO(" Camera capture framerate is %u/%u\n",
             parms.parm.capture.timeperframe.denominator, \
             parms.parm.capture.timeperframe.numerator);
  } else {
    LOG_ERROR("VIDIOC_G_PARM failed!\n");
  }

  /* set image format */
  memset(&fmt, 0, sizeof(struct v4l2_format));
  fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  fmt.fmt.pix.width = cap_w_;
  fmt.fmt.pix.height = cap_h_;
  fmt.fmt.pix.pixelformat = cap_fmt_;
  fmt.fmt.pix.field = V4L2_FIELD_NONE;
  if (ioctl(vdo_fd_, VIDIOC_S_FMT, &fmt) < 0) {
    LOG_ERROR("VIDIOC_S_FMT failed!\n");
    return -1;
  }

  /* reqbufs */
  memset(&req, 0, sizeof(struct v4l2_requestbuffers));
  req.count = buf_cnt_;
  req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  req.memory = V4L2_MEMORY_MMAP;
  if (ioctl(vdo_fd_, VIDIOC_REQBUFS, &req) < 0) {
    LOG_ERROR("VIDIOC_REQBUFS failed\n");
    return -1;
  }

  /* mmap buffers */
  buffers_ = (TAG_CAP_DATA*)calloc(req.count, sizeof(TAG_CAP_DATA));
  for (int n_buffers = 0; n_buffers < req.count; ++n_buffers) {
    memset(&buf, 0, sizeof(struct v4l2_buffer));
    buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buf.memory = V4L2_MEMORY_MMAP;
    buf.index = n_buffers;

    if (ioctl(vdo_fd_, VIDIOC_QUERYBUF, &buf) == -1) {
      LOG_ERROR("VIDIOC_QUERYBUF error\n");
      return -1;
    }

    buffers_[n_buffers].length = buf.length;
    buffers_[n_buffers].start = (void*)mmap(NULL, buf.length,
                                            PROT_READ | PROT_WRITE,
                                            MAP_SHARED, vdo_fd_,
                                            buf.m.offset);
    LOG_INFO("map buffer index: %d, mem: 0x%x, len: %x, offset: %x\n",
             n_buffers, (int)buffers_[n_buffers].start, buf.length, buf.m.offset);
  }

  /* qbuf */
  for (int n_buffers = 0; n_buffers < req.count; n_buffers++) {
    memset(&buf, 0, sizeof(struct v4l2_buffer));
    buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buf.memory = V4L2_MEMORY_MMAP;
    buf.index = n_buffers;
    if (ioctl(vdo_fd_, VIDIOC_QBUF, &buf) == -1) {
      LOG_ERROR("VIDIOC_QBUF error\n");
      return -1;
    }
  }
  return 0;
}

int CCamera::capture_quit() {
  LOG_INFO("capture quit!\n");

  /* streamoff */
  if (vdo_fd_ > 0) {
    enum v4l2_buf_type type;
    type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if (ioctl(vdo_fd_, VIDIOC_STREAMOFF, &type) == -1) {
      LOG_ERROR("VIDIOC_STREAMOFF error! %s\n", strerror(errno));
      return -1;
    }
  }

  if (buffers_) {
    /* munmap buffers */
    for (int i = 0; i < buf_cnt_; i++) {
      LOG_INFO("ummap index: %d, mem: %x, len: %x\n",
               i, (unsigned int)buffers_[i].start, (unsigned int)buffers_[i].length);
      munmap(buffers_[i].start, buffers_[i].length);
    }

    /* free buffers and close videofd */
    free(buffers_);
    buffers_ = NULL;
  }
  if (vdo_fd_ > 0) {
    close(vdo_fd_);
    vdo_fd_ = 0;
  }
  return 0;
}

int CCamera::GetFrame(unsigned int *is_exit, CameraCallback callback, void *usr_arg) {
  int ret;
  ret = capture_init();
  if (0 != ret) {
    return -1;
  }

  /* streamon */
  enum v4l2_buf_type type;
  type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  if (ioctl(vdo_fd_, VIDIOC_STREAMON, &type) == -1) {
    LOG_ERROR("VIDIOC_STREAMON error! %s\n", strerror(errno));
    return -1;
  }

  fd_set fds;
  struct timeval tv;
  struct v4l2_buffer buf;
  while (0 == (*is_exit)) {
    /* wait for sensor capture data */
    tv.tv_sec = 2;
    tv.tv_usec = 0;
    FD_ZERO(&fds);
    FD_SET(vdo_fd_, &fds);
    ret = select(vdo_fd_ + 1, &fds, NULL, NULL, &tv);
    if (ret == -1) {
      if (errno == EINTR) {
        continue;
      }
      LOG_ERROR("select error\n");
    } else if (ret == 0) {
      LOG_ERROR("select timeout\n");
      break;
    }

    memset(&buf, 0, sizeof(struct v4l2_buffer));
    buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buf.memory = V4L2_MEMORY_MMAP;
    ret = ioctl(vdo_fd_, VIDIOC_DQBUF, &buf);
    if (ret == 0) {
      unsigned int phyaddr = buf.m.offset;
      void *vir_addr = buffers_[buf.index].start + phyaddr - buf.m.offset;

      int frameszie = cap_w_ * cap_h_ * 1.5;
      //LOG_INFO("buf.length %d, vaddr: 0x%x, phy_addr 0x%x\n",
      //         buf.length, (unsigned int)vir_addr, phyaddr);
      //LOG_INFO("buf.length %d, vaddr: 0x%x, phy_addr 0x%x\n",
      //         frameszie,  (unsigned int)vir_addr, buffers_[buf.index].phy_addr);

      if (callback) {
        callback(vir_addr, phyaddr, cap_w_, cap_h_, usr_arg);
      }

      ioctl(vdo_fd_, VIDIOC_QBUF, &buf);
    }
  }
  capture_quit();
  LOG_ERROR("capture exit !\n");
  return 0;
}
