
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
  // ����һ��audio and video ������
  avf_contex = avformat_alloc_context();

  // ��һ������������������໥֮���
  if (avformat_open_input(&avf_contex,
                          rtsp_url_.c_str(),
                          NULL,
                          NULL) != 0) {
    LOG_ERROR("Couldn't open input stream %s.\n", rtsp_url_.c_str());
    goto EXIT;
  }
  // ���Ҵ򿪵������Ƿ��п��Բ��ŵ�����(����SDP���������жϵ�)
  if (avformat_find_stream_info(avf_contex, NULL) < 0) {
    LOG_ERROR("Couldn't find stream information.\n");
    goto EXIT;
  }
  // �����Ƿ�����û���Ҫ���ŵ�������
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
  // ����������ȡ����
  avc_contex = avf_contex->streams[video_index]->codec;
  av_codec = avcodec_find_decoder(avc_contex->codec_id);
  if (av_codec == NULL) {
    LOG_ERROR("Codec not found.\n");
    goto EXIT;
  }
  // ���³�ʼ��������״̬
  if (avcodec_open2(avc_contex, av_codec, NULL) < 0) {
    LOG_ERROR("Could not open codec.\n");
    goto EXIT;
  }

  // ����֡����
  av_yuv_frame = av_frame_alloc();
  // ����һ��֡����buffer
  yuv_buffer = (unsigned char *)av_malloc(
                 av_image_get_buffer_size(AV_PIX_FMT_YUV420P,
                                          avc_contex->width,
                                          avc_contex->height,
                                          1));
  // ��֡������ڴ���֡bind
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

      //�����ڴ�
      char *buf = new char[newSize];

      int height = avc_contex->height;
      int width = avc_contex->width;

      //д������
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
      //�����buf�����Ѿ���yuv420p�������ˣ����Զ������κεĴ�������
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
