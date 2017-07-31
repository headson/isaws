// AVPlayer.cpp
// CAVPlayer.cpp : 实现文件
//
#include "avplayer.h"
#include "vzbase/helper/stdafx.h"

extern "C"
{
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libswscale/swscale.h"
#include "libavutil/imgutils.h"
};

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define  MBLOCKMAX_X    3
#define  MBLOCKMAX_Y    4

static void   WINAPI stopPlayBack(void *ff_av_ctx);
static int    WINAPI UpdateScreen(void *ff_av_ctx, void* pframe);
static int    WINAPI connect(void *ff_av_ctx);
static DWORD  WINAPI AvMain(void* pData);
static DWORD  WINAPI FFAudioThread(void *pData);
static DWORD  WINAPI FFVideoThread(void* pData);


VzPlayerManager::VzPlayerManager() {}

VzPlayerManager::~VzPlayerManager() {}

bool VzPlayerManager::VzPlayerManagerSetup() {
  av_register_all();
  avformat_network_init();
  return true;
}

bool VzPlayerManager::VzPlayerManagerCleanup() {
  avformat_network_deinit();
  return true;
}

VzPlayer *VzPlayerManager::CreateVzPlayer(PlayerCallback *call_back,
    void *user_data,
    int auto_relink) {
  return NULL;
}

void VzPlayerManager::DeleteVzPlayer(VzPlayer *player) {
  if (player == NULL) {
    LOG(L_WARNING) << "The player == NULL";
    return;
  }
}

//////////////////////////////////////////////////////////////////////////
VzPlayer::VzPlayer(PlayerCallback *call_back,
                   void *user_data,
                   int auto_relink)
  : m_hBitmap(NULL)
  , m_hMemHdc(NULL)
  , m_pBmpData(NULL)
  , m_iWidth(0)
  , m_iHeight(0)
  , m_hWnd(NULL)
  , m_pctx(NULL)
  , m_hAVMainThread(NULL)
  , m_hAVShutdownEvent(NULL)
  , m_dwAVMainThreadId(NULL)
  , m_TimCount(0) {
  memset(&m_cs, 0, sizeof(m_cs));
  InitializeCriticalSection(&m_cs);
}

// destractor
VzPlayer::~VzPlayer() {
  DestroyBmp();
  DeleteCriticalSection(&m_cs);
  memset(&m_cs, 0, sizeof(m_cs));
}

int WINAPI connect(void *ff_av_ctx) {

  if (audioComponent==0) {
    ctx->hShutDownVThread = CreateEvent(NULL, TRUE, FALSE, NULL);
    ctx->hAudioThread = CreateThread(0, 0, FFAudioThread, ctx, 0, 0);  //create the audio thread 'GFFAudioThread'
  }
  if (videoComponent==0) {
    ctx->hShutDownAThread = CreateEvent(NULL, TRUE, FALSE, NULL);
    ctx->hVideoThread = CreateThread(0, 0, FFVideoThread, ctx, 0, 0);  //create the video thread 'GFFVideoThread'
  }
  return 1;
}
//<AvMain start>
DWORD WINAPI AvMain(void* pData) {
  AVPacket      packet;
  AVInputFormat *fmt            = NULL;
  int           frameFinished   = 0;
  int           pktNotAvail     = 0;
  ff_av_param   *ctx = (ff_av_param *)pData;
  ctx->pFormatCtx     = NULL;
  ctx->timeout        = 300;
  ctx->retVal         = -1;
  if (pData == 0) {
    return -1;
  }

  avcodec_register_all();  // init ffmpeg
  av_register_all();
  while(!ctx->stop) {
    if(ctx->timeout-- > 0) {
      if(connect(ctx)<=0) { //open the connection
        Sleep(1000);
        continue;
      }
    }
    while(!ctx->stop) {
      if (ctx->pause == 1)  // stop getting the next packet when pause enabled
        continue;

      if(av_read_frame(ctx->pFormatCtx, &packet)<0) { //read the packet
        if (strncmp(ctx->aUrl, "rtsp://", 7) == NULL)
          Sleep(10000);       // wait for 10 seconds before closing the connection
        else
          stopPlayBack(ctx);  // to stop the playback mode
        break;
      }

      if(packet.data) {  // for discarding the invalid packet data..(this would help playing streams with low fps)
        if(packet.stream_index == ctx->videoStream)       // if its a video stream then queue the video packets
          queue_av_pkt(ctx->vq, &packet, (__int64)(1000*av_q2d(ctx->pFormatCtx->streams[ctx->videoStream]->time_base)*packet.dts));
        else if(packet.stream_index == ctx->audioStream)  // if its a audio stream then queue the audio packets
          queue_av_pkt(ctx->aq, &packet, (__int64)(1000*av_q2d(ctx->pFormatCtx->streams[ctx->audioStream]->time_base)*packet.dts));
      }
      av_free_packet(&packet);
    }
    // Signal the thread to exit
    SetEvent(ctx->hShutDownVThread);
    // Signal the thread to exit
    SetEvent(ctx->hShutDownAThread);
    //jun 2010
    cleanup(ctx);
  }
  return 0;
}

DWORD WINAPI FFAudioThread(void *pData) {
  int device_opened=0;
  ff_av_param *ctx = (ff_av_param *)pData;

  av_pkt  *pkt;
  int     pkt_size, idx, len=-1, data_size, delay;
  int16_t *audio_buf;
  AVCodecContext *aCodecCtx=0;

  if (ctx->audioStream!=-1) {
    aCodecCtx=ctx->pFormatCtx->streams[ctx->audioStream]->codec;
  }

  if(aCodecCtx!=0 && gvp_audio_device_open_win32(ctx, aCodecCtx->channels, aCodecCtx->sample_rate) == 0)
    device_opened = 1;

  audio_buf = (int16_t*)malloc(10*1024);

  for(; ctx->stop==0;) {
    if (ctx->pause == 1) continue; //stop decoding the next packet when pause enabled
    if(WAIT_OBJECT_0 == WaitForSingleObject(ctx->hShutDownAThread, 0)) {
      break;
    }
    data_size = AVCODEC_MAX_AUDIO_FRAME_SIZE;
    if(queue_dequeue(ctx->aq, (void**)&pkt, &pkt_size, 1)==0) {
      ctx->noPackets = 0;
      break;
    }
    if(pkt==0 || pkt_size<=0)break;
    if(ctx->stop) {
      free(pkt->data);
      free(pkt);
      break;
    }
    if((device_opened) && (pkt_size>0)) {
      delay = (int)(pkt->pts - ctx->elapsed_ms);
      if (strncmp(ctx->aUrl,"rtsp://",7) != NULL) {
        Sleep(10);
      }
      if(ctx->pause_audio == 0 && audio_buf != 0) {
        if(WAIT_OBJECT_0 == WaitForSingleObject(ctx->hShutDownAThread, 0)) {
          free(pkt->data);
          pkt->size = 0;
          free(pkt);
          break;
        }
        for(idx=0; ctx->stop==0 && idx<pkt->size; idx += len) { //single packet might have more than one audio frame
          //len = avcodec_decode_audio2(aCodecCtx, audio_buf, &data_size, pkt->data+idx, pkt->size-idx);
          len = avcodec_decode_audio3(aCodecCtx, audio_buf, &data_size, pkt);
          if(len<=0)
            break;
        }
        FFQueueAudioPacket(ctx, (uint8_t*)audio_buf, data_size, aCodecCtx->sample_rate);
      }
    }
    free(pkt->data);
    pkt->size = 0;
    free(pkt);
  }
  if(audio_buf) {
    free(audio_buf);
    audio_buf = NULL;
  }
  if(device_opened) {
    gvp_audio_device_close_win32(ctx);
    if (aCodecCtx)
      avcodec_flush_buffers(aCodecCtx);
  }
  if(aCodecCtx)
    avcodec_close(aCodecCtx);
  return 0;
}

DWORD WINAPI FFVideoThread(void* pData) {
  av_pkt  *pkt;
  int     pkt_size;
  int     len = 0, got_picture, delay;
  __int64 sync;
  ff_av_param *ctx    = (ff_av_param *)pData;
  AVFrame     *frame  = avcodec_alloc_frame();

  if(ctx->videoStream >= 0) {
    ctx->pVideoCodecCtx = ctx->pFormatCtx->streams[ctx->videoStream]->codec;
  }

  sync = av_gettime() / 100;
  for(; ctx->stop==0;) {
    if(WAIT_OBJECT_0 == WaitForSingleObject(ctx->hShutDownVThread, 0)) {
      break;
    }
    if (ctx->pause == 1) continue; //stop decoding the next packet when pause enabled

    if(queue_dequeue(ctx->vq, (void**)&pkt, &pkt_size, 1)==0) {
      ctx->noPackets = 0;
      break;
    }

    if(pkt==0 || pkt_size<=0)
      break;
    if ((ctx->pVideoCodecCtx) && (pkt_size>0)) {
      if(WAIT_OBJECT_0 == WaitForSingleObject(ctx->hShutDownVThread, 0)) {
        if(pkt->data)free(pkt->data);
        pkt->size = 0;
        free(pkt);
        break;
      }
      len = avcodec_decode_video(ctx->pVideoCodecCtx, frame, &got_picture, pkt->data, pkt->size);
      //len = avcodec_decode_video2(ctx->pVideoCodecCtx, frame, &got_picture, pkt);
    }
    ctx->elapsed_ms = (av_gettime()/100)-sync;
    if(len > 0) {
      delay = (int)(pkt->pts - ctx->elapsed_ms);

      if (strncmp(ctx->aUrl,"rtsp://",7) != NULL) {
        Sleep(10);//INFINITE
      }

      if(got_picture) {
        UpdateScreen(ctx, frame);
      }
    }
    if(pkt->data)free(pkt->data);
    pkt->size = 0;
    free(pkt);
  }
  if(frame) {
    av_free(frame);
  }
  if (ctx->pVideoCodecCtx) {
    avcodec_flush_buffers(ctx->pVideoCodecCtx);  // Flush out the buffers for video & audio context before close
  }
  if (ctx->pVideoCodecCtx) {
    avcodec_close(ctx->pVideoCodecCtx);          // Close the codec
  }
  return 0;
}
//<AvMain end>
void WINAPI stopPlayBack(void *ff_av_ctx) {
}

int WINAPI  UpdateScreen(void *ff_av_ctx, void* pframe) {
  if(!ff_av_ctx) {
    return -1;
  }

  AVFrame     *frame   = (AVFrame*)pframe;
  ff_av_param *pctx    = (ff_av_param*)ff_av_ctx;
  VzPlayer   *pPlayer = (VzPlayer*)pctx->prv_data;
  EnterCriticalSection(&pctx->ct);
  if(pctx->state != 1 || pctx->stop!=0) {
    LeaveCriticalSection(&pctx->ct);
    return -1;
  }
  pPlayer->CreateBmp(pctx->pVideoCodecCtx->width,
                     pctx->pVideoCodecCtx->height);
  LeaveCriticalSection(&pctx->ct);

  if(pPlayer == NULL ||
      pPlayer->m_pBmpData == NULL) {
    return -1;
  }
  pPlayer->ScaleFrame(frame);
  pPlayer->ShowFrame();
  return 0;
}

int VzPlayer::CreateBmp(int w, int h) { ///创建位图
  EnterCriticalSection(&m_cs);
  if(w == m_iWidth && h == m_iHeight) {
    if(m_hMemHdc != NULL || m_pBmpData != NULL) {
      LeaveCriticalSection(&m_cs);
      return 0;  // already setup
    }
  }
  if(!m_hWnd) { //
    LeaveCriticalSection(&m_cs);
    return -1;  // failed
  }

  DestroyBmp();

  m_iWidth  = w;
  m_iHeight = h;

  HDC         hdc;
  BYTE        buffer[sizeof(BITMAPINFOHEADER) + 3 * sizeof(RGBQUAD)];
  BITMAPINFO        *pBMI    = (BITMAPINFO*)buffer;
  BITMAPINFOHEADER  *pHeader = &pBMI->bmiHeader;
  DWORD             *pColors = (DWORD*)&pBMI->bmiColors;
  int               bpp = 24;

  pHeader->biSize            = sizeof(BITMAPINFOHEADER);
  pHeader->biWidth           = m_iWidth;
  pHeader->biHeight          = -1*m_iHeight;
  pHeader->biPlanes          = 1;
  pHeader->biBitCount        = bpp;
  pHeader->biCompression     = BI_RGB;//BI_BITFIELDS;
  pHeader->biSizeImage       = (m_iWidth * m_iHeight * bpp) / 8;
  pHeader->biXPelsPerMeter   = 0;
  pHeader->biYPelsPerMeter   = 0;
  pHeader->biClrUsed         = 0;
  pHeader->biClrImportant    = 0;

  hdc=::GetDC(m_hWnd);
  if(!hdc) {
    LeaveCriticalSection(&m_cs);
    return -1;
  }
  m_hMemHdc = CreateCompatibleDC(hdc);
  if(m_hMemHdc != NULL) {
    m_hBitmap = CreateDIBSection(0, pBMI, DIB_RGB_COLORS, (void**)&(m_pBmpData), 0, 0);
    if(m_hBitmap) {
      ::SelectObject(m_hMemHdc, m_hBitmap);
    } else {
      LeaveCriticalSection(&m_cs);
      return -1;
    }
  } else {
    LeaveCriticalSection(&m_cs);
    return -1;
  }
  ::ReleaseDC(m_hWnd,hdc);
  LeaveCriticalSection(&m_cs);
  return 0;//OK
}

void VzPlayer::DestroyBmp() { //销毁位图
  if(m_hBitmap) {
    ::DeleteObject(m_hBitmap);
    m_hBitmap = NULL;
  }
  if(m_hMemHdc) {
    ::DeleteDC(m_hMemHdc);
    m_hMemHdc = NULL;
  }
  m_pBmpData = NULL;
  if(m_pctx) {
    ff_av_param *pctx = (ff_av_param*)m_pctx;

    if(pctx->pimg_resample_ctx) {
      sws_freeContext(pctx->pimg_resample_ctx);
      pctx->pimg_resample_ctx = NULL;
    }
  }
}

//设置显示窗口
int VzPlayer::SetupDisplayWnd(HWND hwnd) {
  if(!hwnd) {
    return -1;
  }

  EnterCriticalSection(&m_cs);
  m_hWnd = hwnd;
  LeaveCriticalSection(&m_cs);
  return 0;
}

// 打开视频通道
int VzPlayer::OpenChannel(const char *purl) {
  if(!m_pctx || !purl)
    return -1;

  ff_av_param * pctx = (ff_av_param*)m_pctx;
  EnterCriticalSection(&pctx->ct);
  if(pctx->state != 0) {
    LeaveCriticalSection(&pctx->ct);
    return 0;  // already open
  }
  pctx->state = 1;
  pctx->stop  = 0;

  size_t size=0;
  wcstombs_s(&size, pctx->aUrl, MAX_PATH, purl, MAX_PATH);
  LeaveCriticalSection(&pctx->ct);

  m_TimCount++;
  timeBeginPeriod(1);

  FFInit();
  m_hAVShutdownEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
  if (m_hAVMainThread == NULL) {
    m_hAVMainThread = CreateThread(0, 0, AvMain, m_pctx, 0, &m_dwAVMainThreadId);
  }
  return 0;
}

// 关闭通道
int VzPlayer::CloseChannel() {
  if(!m_pctx) {
    return -1;
  }

  memset(m_pBmpData, 0, (m_iWidth * m_iHeight * 24) / 8);
  ShowFrame();

  ff_av_param * pctx = (ff_av_param *)m_pctx;
  EnterCriticalSection(&pctx->ct);
  if(pctx->state == 0) {
    LeaveCriticalSection(&pctx->ct);
    return 0;//already stoped
  }
  pctx->stop = 1;
  LeaveCriticalSection(&pctx->ct);

  if (m_hAVMainThread != NULL) {
    // Signal the thread to exit
    SetEvent(m_hAVShutdownEvent);
    //ResumeThread(m_hAVMainThread);

    DWORD ExitCode;
    GetExitCodeThread(m_hAVMainThread, &ExitCode);
    if (ExitCode == STILL_ACTIVE) {
      WaitForSingleObject(m_hAVMainThread, INFINITE);
    }
    CloseHandle(m_hAVMainThread);
    m_hAVMainThread = NULL;
    DestroyBmp();
  }

  // Reset the shutdown event
  ResetEvent(m_hAVShutdownEvent);
  CloseHandle(m_hAVShutdownEvent);
  m_hAVShutdownEvent = NULL;

  while(m_TimCount>0) {  //timeEndPeriod needs to be called as many times as timeBeginPeriod
    // don;t know the side-effect if not done
    m_TimCount --;
    timeEndPeriod(1);
  }
  pctx->state = 0;

  return 0;
}

// 缩放视频帧
int VzPlayer::ScaleFrame(void * pframe) {
  AVFrame *frame = (AVFrame*)pframe;

  EnterCriticalSection(&m_cs);

  ff_av_param *pctx       = (ff_av_param*)m_pctx;
  uint8_t     *data[4]    = {(uint8_t*)m_pBmpData, 0, 0, 0};
  int         linesize[4] = {m_iWidth*3, 0, 0, 0};

  if(pctx->pimg_resample_ctx == NULL) {
    pctx->pimg_resample_ctx = sws_getContext(
                                pctx->pVideoCodecCtx->width,
                                pctx->pVideoCodecCtx->height,
                                pctx->pVideoCodecCtx->pix_fmt,
                                pctx->pVideoCodecCtx->width,/*m_iWidth,*/
                                pctx->pVideoCodecCtx->height,/*m_iHeight,*/
                                PIX_FMT_BGR24,
                                SWS_BILINEAR,
                                NULL,
                                NULL,
                                NULL);
  }
  if(pctx->pimg_resample_ctx) {
    if (pctx->pVideoCodecCtx->width > 0
        && pctx->pVideoCodecCtx->height > 0) {
      sws_scale(pctx->pimg_resample_ctx,
                frame->data,
                frame->linesize,
                0,
                pctx->pVideoCodecCtx->height,
                data,
                linesize);
    }
  }

  LeaveCriticalSection(&m_cs);
  return 0;
}

// 显示视频帧
void VzPlayer::ShowFrame() {
  HDC hdc=::GetDC(m_hWnd);

  CRect rc;
  ::GetClientRect(m_hWnd, &rc);

  ff_av_param *pctx       = (ff_av_param*)m_pctx;
  StretchBlt(hdc, 0, 0, rc.Width(), rc.Height(), m_hMemHdc, 0, 0,
             pctx->pVideoCodecCtx->width, pctx->pVideoCodecCtx->height, SRCCOPY);
}
