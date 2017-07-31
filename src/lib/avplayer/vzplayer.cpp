#include "vzplayer.h"
#include "vzbase/helper/stdafx.h"

namespace vzplayer {

VzPlayerManager::VzPlayerManager() {
}

VzPlayerManager::~VzPlayerManager() {
}

bool VzPlayerManager::VzPlayerManagerSetup() {
  av_register_all();
  avformat_network_init();
  return true;
}

bool VzPlayerManager::VzPlayerManagerCleanup() {
  avformat_network_deinit();
  return true;
}

VzPlayer *VzPlayerManager::CreateVzPlayer(
  OnPlayerStatusResponse *call_back,
  void *user_data,
  int auto_relink) {
  return NULL;
}

void VzPlayerManager::DeleteVzPlayer(VzPlayer *player) {
  if(player == NULL) {
    return ;
  }
}

////////////////////////////////////////////////////////////////////////////////
VzPlayer::VzPlayer(OnPlayerStatusResponse *call_back,
                   void *user_data,
                   int auto_relink)
  : user_data_(user_data),
    status_call_back_(call_back),
    auto_relink_(auto_relink),
    avc_contex_(NULL),
    avf_contex_(NULL),
    avcodec_(NULL),
    video_index_(VZPLAYER_INDEX_NONE),
    av_frame_(NULL),
    av_frame_yuv_(NULL),
    buffer_(NULL),
    img_convert_ctx_(NULL),
    packet_(NULL) {
  memset(&m_cs, 0, sizeof(m_cs));
  InitializeCriticalSection(&m_cs);
}

VzPlayer::~VzPlayer() {
  DestroyBmp();
  DeleteCriticalSection(&m_cs);
  memset(&m_cs, 0, sizeof(m_cs));
}

int VzPlayer::SetRtspUrl(const char *rtsp_url) {
  rtsp_url_ = rtsp_url;
  return VZPLAYER_RES_SUCCEED;
}

int OnIoOpen(struct AVFormatContext *s,
             AVIOContext **pb,
             const char *url,
             int flags,
             AVDictionary **options) {
  LOG(L_INFO) << "OnIoOpen";
  return 0;
}

void OnIoClose(struct AVFormatContext *s, AVIOContext *pb) {
  LOG(L_INFO) << "OnIoClose";
}

int VzPlayer::StartPlayer(AVMediaType media_type) {
  media_type_ = media_type;
  // 分配一个audio and video 解码器
  avf_contex_ = avformat_alloc_context();
  //avf_contex_->io_open  = &OnIoOpen;
  //avf_contex_->io_close = &OnIoClose;
  // 打开一个流，并且与解码器相互之间绑定
  if (avformat_open_input(&avf_contex_,
                          rtsp_url_.c_str(),
                          NULL,
                          NULL) != 0) {
    LOG(L_WARNING) << "Couldn't open input stream " << rtsp_url_;
    return VZPLAYER_RES_FAILURE;
  }
  // 查找打开的流中是否有可以播放的数据(根据SDP的描述来判断的)
  if (avformat_find_stream_info(avf_contex_, NULL) < 0) {
    LOG(L_WARNING) << "Couldn't find stream information.";
    return VZPLAYER_RES_FAILURE;
  }
  // 查找是否存在用户想要播放的数据流
  for(int i = 0; i < avf_contex_->nb_streams; i++) {
    if(avf_contex_->streams[i]->codec->codec_type == media_type_) {
      video_index_ = i;
      break;
    }
  }
  if(video_index_ == VZPLAYER_INDEX_NONE) {
    LOG(L_WARNING) << "Didn't find a video stream.";
    return VZPLAYER_RES_FAILURE;
  }
  // 将编码器提取出来
  avc_contex_ = avf_contex_->streams[video_index_]->codec;
  avcodec_    = avcodec_find_decoder(avc_contex_->codec_id);
  if(avcodec_ == NULL) {
    LOG(L_WARNING) << "Codec not found.";
    return VZPLAYER_RES_FAILURE;
  }
  // 重新初始化编码器状态
  if (avcodec_open2(avc_contex_, avcodec_, NULL) < 0) {
    LOG(L_WARNING) << "Could not open codec.";
    return VZPLAYER_RES_FAILURE;
  }
  // 分配帧数据
  av_frame_       = av_frame_alloc();
  av_frame_yuv_   = av_frame_alloc();
  // 分配一个帧缓存buffer
  buffer_ = (unsigned char *)av_malloc(
              av_image_get_buffer_size(AV_PIX_FMT_YUV420P,
                                       avc_contex_->width,
                                       avc_contex_->height,
                                       1));
  // 将帧缓存的内存与帧bind
  av_image_fill_arrays(av_frame_yuv_->data,
                       av_frame_yuv_->linesize,
                       buffer_,
                       AV_PIX_FMT_YUV420P,
                       avc_contex_->width,
                       avc_contex_->height,
                       1);

  img_convert_ctx_ = sws_getContext(avc_contex_->width,
                                    avc_contex_->height,
                                    avc_contex_->pix_fmt,
                                    avc_contex_->width,
                                    avc_contex_->height,
                                    AV_PIX_FMT_YUV420P,
                                    SWS_BICUBIC,
                                    NULL,
                                    NULL,
                                    NULL);
  return InitSDLAndRunning();
}

int VzPlayer::InitSDLAndRunning() {
  packet_ = (AVPacket *)av_malloc(sizeof(AVPacket));

  Running();
  return CleanUp();
}

int VzPlayer::Running() {
  for (;;) {
    if(av_read_frame(avf_contex_, packet_) < 0) {
      return VZPLAYER_RES_FAILURE;
    }
    int got_picture = 0;
    int ret = avcodec_decode_video2(avc_contex_,
                                    av_frame_,
                                    &got_picture,
                                    packet_);
    if(ret < 0) {
      LOG(L_WARNING) << "Decode Error.";
      return VZPLAYER_RES_FAILURE;
    }
    if(got_picture) {
      UpdateScreen(frame);
    }
    av_free_packet(packet_);
  }
  return VZPLAYER_RES_SUCCEED;
}

int VzPlayer::CleanUp() {
  sws_freeContext(img_convert_ctx_);
  SDL_Quit();
  //--------------
  av_frame_free(&av_frame_yuv_);
  av_frame_free(&av_frame_);
  avcodec_close(avc_contex_);
  avformat_close_input(&avf_contex_);
  return VZPLAYER_RES_SUCCEED;
}


int WINAPI  UpdateScreen(void *ff_av_ctx, void* pframe) {
  if (!ff_av_ctx) {
    return -1;
  }

  AVFrame     *frame = (AVFrame*)pframe;
  ff_av_param *pctx = (ff_av_param*)ff_av_ctx;
  VzPlayer   *pPlayer = (VzPlayer*)pctx->prv_data;
  EnterCriticalSection(&pctx->ct);
  if (pctx->state != 1 || pctx->stop != 0) {
    LeaveCriticalSection(&pctx->ct);
    return -1;
  }
  pPlayer->CreateBmp(pctx->pVideoCodecCtx->width,
                     pctx->pVideoCodecCtx->height);
  LeaveCriticalSection(&pctx->ct);

  if (pPlayer == NULL ||
      pPlayer->m_pBmpData == NULL) {
    return -1;
  }
  pPlayer->ScaleFrame(frame);
  pPlayer->ShowFrame();
  return 0;
}

int VzPlayer::CreateBmp(int w, int h) { ///创建位图
  EnterCriticalSection(&m_cs);
  if (w == m_iWidth && h == m_iHeight) {
    if (m_hMemHdc != NULL || m_pBmpData != NULL) {
      LeaveCriticalSection(&m_cs);
      return 0;  // already setup
    }
  }
  if (!m_hWnd) { //
    LeaveCriticalSection(&m_cs);
    return -1;  // failed
  }

  DestroyBmp();

  m_iWidth = w;
  m_iHeight = h;

  HDC         hdc;
  BYTE        buffer[sizeof(BITMAPINFOHEADER)+ 3 * sizeof(RGBQUAD)];
  BITMAPINFO        *pBMI = (BITMAPINFO*)buffer;
  BITMAPINFOHEADER  *pHeader = &pBMI->bmiHeader;
  DWORD             *pColors = (DWORD*)&pBMI->bmiColors;
  int               bpp = 24;

  pHeader->biSize = sizeof(BITMAPINFOHEADER);
  pHeader->biWidth = m_iWidth;
  pHeader->biHeight = -1 * m_iHeight;
  pHeader->biPlanes = 1;
  pHeader->biBitCount = bpp;
  pHeader->biCompression = BI_RGB;//BI_BITFIELDS;
  pHeader->biSizeImage = (m_iWidth * m_iHeight * bpp) / 8;
  pHeader->biXPelsPerMeter = 0;
  pHeader->biYPelsPerMeter = 0;
  pHeader->biClrUsed = 0;
  pHeader->biClrImportant = 0;

  hdc = ::GetDC(m_hWnd);
  if (!hdc) {
    LeaveCriticalSection(&m_cs);
    return -1;
  }
  m_hMemHdc = CreateCompatibleDC(hdc);
  if (m_hMemHdc != NULL) {
    m_hBitmap = CreateDIBSection(0, pBMI, DIB_RGB_COLORS, (void**)&(m_pBmpData), 0, 0);
    if (m_hBitmap) {
      ::SelectObject(m_hMemHdc, m_hBitmap);
    }
    else {
      LeaveCriticalSection(&m_cs);
      return -1;
    }
  }
  else {
    LeaveCriticalSection(&m_cs);
    return -1;
  }
  ::ReleaseDC(m_hWnd, hdc);
  LeaveCriticalSection(&m_cs);
  return 0;//OK
}

void VzPlayer::DestroyBmp() { //销毁位图
  if (m_hBitmap) {
    ::DeleteObject(m_hBitmap);
    m_hBitmap = NULL;
  }
  if (m_hMemHdc) {
    ::DeleteDC(m_hMemHdc);
    m_hMemHdc = NULL;
  }
  m_pBmpData = NULL;
  if (m_pctx) {
    ff_av_param *pctx = (ff_av_param*)m_pctx;

    if (pctx->pimg_resample_ctx) {
      sws_freeContext(pctx->pimg_resample_ctx);
      pctx->pimg_resample_ctx = NULL;
    }
  }
}

//设置显示窗口
int VzPlayer::SetupDisplayWnd(HWND hwnd) {
  if (!hwnd) {
    return -1;
  }

  EnterCriticalSection(&m_cs);
  m_hWnd = hwnd;
  LeaveCriticalSection(&m_cs);
  return 0;
}

}  // namespace vzplayer
