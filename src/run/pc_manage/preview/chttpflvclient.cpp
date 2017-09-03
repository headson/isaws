
/************************************************************************
* Author      : SoberPeng 2017-08-26
* Description :
************************************************************************/
#include "chttpflvclient.h"

#include "cvideowidget.h"
#include "vzbase/helper/stdafx.h"

#pragma comment(lib, "avcodec.lib")
#pragma comment(lib, "avformat.lib")
#pragma comment(lib, "swscale.lib")
#pragma comment(lib, "avutil.lib")

CHttpFlvClient::CHttpFlvClient(OnPlayerStatusResponse call_back,
                               void *user_data,
                               int auto_relink)
  : QThread()
  , user_data_(user_data)
  , status_call_back_(call_back)
  , auto_relink_(auto_relink) {
  image_rorate_ = false;
  thread_running_ = false;

  av_register_all();
  avformat_network_init();
}

CHttpFlvClient::~CHttpFlvClient() {
  Stop();
  avformat_network_deinit();
}

void CHttpFlvClient::SetPlayUrl(const char *rtsp_url) {
  rtsp_url_ = rtsp_url;
}

int CHttpFlvClient::StartPlayer(AVMediaType media_type, CPlayWidget *wnd) {
  QMutexLocker al(&vmutex_);
  if (QThread::isRunning()) {
    thread_running_ = false;
    QThread::wait();
  }

  vdo_widget_ = wnd;
  media_type_ = media_type;

  thread_running_ = true;
  QThread::start(QThread::IdlePriority);
  return VZPLAYER_RES_SUCCEED;
}

void CHttpFlvClient::run() {
  AVFormatContext *avf_contex   = NULL;
  AVCodecContext  *avc_contex   = NULL;
  AVCodec         *av_codec     = NULL;
  int              video_index  = -1;

  unsigned char   *yuv_buffer   = NULL;
  AVFrame         *av_yuv_frame = NULL;

  AVPacket         av_dec_packet;

REOPEN:
  // 分配一个audio and video 解码器
  avf_contex = avformat_alloc_context();

  // 打开一个流，并且与解码器相互之间绑定
  if (avformat_open_input(&avf_contex,
                          rtsp_url_.c_str(),
                          NULL,
                          NULL) != 0) {
    LOG_ERROR("Couldn't open input stream %s.\n", rtsp_url_.c_str());
    goto EXIT;
  }
  // 查找打开的流中是否有可以播放的数据(根据SDP的描述来判断的)
  if (avformat_find_stream_info(avf_contex, NULL) < 0) {
    LOG_ERROR("Couldn't find stream information.\n");
    goto EXIT;
  }
  // 查找是否存在用户想要播放的数据流
  for (int i = 0; i < avf_contex->nb_streams; i++) {
    if (avf_contex->streams[i]->codec->codec_type == media_type_) {
      video_index = i;
      break;
    }
  }
  if (video_index == VZPLAYER_INDEX_NONE) {
    LOG_ERROR("Didn't find a video stream.\n");
    goto EXIT;
  }
  // 将编码器提取出来
  avc_contex = avf_contex->streams[video_index]->codec;
  av_codec = avcodec_find_decoder(avc_contex->codec_id);
  if (av_codec == NULL) {
    LOG_ERROR("Codec not found.\n");
    goto EXIT;
  }
  // 重新初始化编码器状态
  if (avcodec_open2(avc_contex, av_codec, NULL) < 0) {
    LOG_ERROR("Could not open codec.\n");
    goto EXIT;
  }

  // 分配帧数据
  av_yuv_frame = av_frame_alloc();
  // 分配一个帧缓存buffer
  yuv_buffer = (unsigned char *)av_malloc(
                 av_image_get_buffer_size(AV_PIX_FMT_YUV420P,
                                          avc_contex->width,
                                          avc_contex->height,
                                          1));
  // 将帧缓存的内存与帧bind
  av_image_fill_arrays(av_yuv_frame->data,
                       av_yuv_frame->linesize,
                       yuv_buffer,
                       AV_PIX_FMT_YUV420P,
                       avc_contex->width,
                       avc_contex->height,
                       1);

  av_init_packet(&av_dec_packet);
  while (thread_running_) {
    if(av_read_frame(avf_contex, &av_dec_packet) < 0) {
      break;
    }
    int got_picture = 0;
    int ret = avcodec_decode_video2(avc_contex,
                                    av_yuv_frame,
                                    &got_picture,
                                    &av_dec_packet);
    if(ret < 0) {
      LOG_ERROR("Decode Error.\n");
      break;
    }
    if(got_picture) {
      int picSize = avc_contex->height * avc_contex->width;
      int newSize = picSize * 1.5;

      //申请内存
      char *buf = new char[newSize];

      int height = avc_contex->height;
      int width = avc_contex->width;

      //写入数据
      int a = 0, i;
      for (i = 0; i < height; i++) {
        memcpy(buf + a, av_yuv_frame->data[0] + i * av_yuv_frame->linesize[0], width);
        a += width;
      }
      for (i = 0; i < height / 2; i++) {
        memcpy(buf + a, av_yuv_frame->data[1] + i * av_yuv_frame->linesize[1], width / 2);
        a += width / 2;
      }
      for (i = 0; i < height / 2; i++) {
        memcpy(buf + a, av_yuv_frame->data[2] + i * av_yuv_frame->linesize[2], width / 2);
        a += width / 2;
      }

      vdo_widget_->PlayOneFrame(buf, avc_contex->width, avc_contex->height);

      //===============
      //到这里，buf里面已经是yuv420p的数据了，可以对它做任何的处理拉！
      //===============
      delete[] buf;
    }
    av_free_packet(&av_dec_packet);
    // _sleep(5*1000);
  }

EXIT:
  if (av_yuv_frame) {
    memset(yuv_buffer, 0x55, avc_contex->width*avc_contex->height*1.5);
    vdo_widget_->PlayOneFrame((char*)yuv_buffer, avc_contex->width, avc_contex->height);

    av_frame_free(&av_yuv_frame);
    av_yuv_frame = NULL;
  }

  if (avc_contex) {
    avcodec_close(avc_contex);
    avc_contex = NULL;
  }

  if (avf_contex) {
    avformat_close_input(&avf_contex);
    avf_contex = NULL;
  }

  //if (yuv_buffer) {
  //  delete[] yuv_buffer;
  //  yuv_buffer = NULL;
  //}
}

void CHttpFlvClient::Stop() {
  auto_relink_ = 0;
  if (QThread::isRunning()) {
    thread_running_ = false;
    QThread::wait();
  }
}
