#ifndef SRC_LIB_VZPLAYER2_PLAYER_VZPLAYER_H_
#define SRC_LIB_VZPLAYER2_PLAYER_VZPLAYER_H_

#include <windows.h>

#include <string>

// #define __STDC_CONSTANT_MACROS
extern "C"
{
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libswscale/swscale.h"
#include "libavutil/imgutils.h"
};

namespace vzplayer {

#define VZPLAYER_RES_FAILURE -1
#define VZPLAYER_RES_SUCCEED 0
#define VZPLAYER_INDEX_NONE  -1

typedef enum {
  STREAM_TYPE_NULL = -1,
  STREAM_TYPE_VIDEO = 0,
  STREAM_TYPE_AUDIO,
  STREAM_TYPE_INFO,
  STREAM_TYPE_VIDEO2,
  STREAM_TYPE_COUNT
} VzPlayerStreamType;

class VzPlayer;

typedef void (__stdcall *OnPlayerStatusResponse)(
  int state_code,
  unsigned int param,
  VzPlayer *player,
  void *user_data);

class VzPlayerManager {
 public:
  VzPlayerManager();
  virtual ~VzPlayerManager();
  bool VzPlayerManagerSetup();
  bool VzPlayerManagerCleanup();
  VzPlayer *CreateVzPlayer(OnPlayerStatusResponse *call_back,
                           void *user_data,
                           int auto_relink = 1);
  void DeleteVzPlayer(VzPlayer *player);
};

class VzPlayer {
 public:
  VzPlayer(OnPlayerStatusResponse *call_back,
           void *user_data,
           int auto_relink);
  virtual ~VzPlayer();

  int SetRtspUrl(const char *rtsp_url);
  int StartPlayer(AVMediaType media_type);

 private:
  int InitSDLAndRunning();
  int Running();
  int CleanUp();

 public:
  inline int  SetupDisplayWnd(HWND hwnd);       // 设置显示窗口
  int         CreateBmp(int w, int h);          // 创建位图
  void        DestroyBmp();                     // 销毁位图
  int         ScaleFrame(void* frame);          // 缩放视频帧
  void        ShowFrame();                      // 显示视频帧

 private:
  static const unsigned int SCREEN_W   = 352;
  static const unsigned int SCREEN_H   = 288;

  void                     *user_data_;
  OnPlayerStatusResponse   *status_call_back_;

  int                       auto_relink_;

  std::string               rtsp_url_;
  AVMediaType               media_type_;

  AVFormatContext          *avf_contex_;
  AVCodecContext           *avc_contex_;
  AVCodec                  *avcodec_;
  int                       video_index_;
  AVFrame	                 *av_frame_;
  AVFrame                  *av_frame_yuv_;
  unsigned char            *buffer_;
  SwsContext               *img_convert_ctx_;
  AVPacket                 *packet_;

  CRITICAL_SECTION   m_cs;
  HWND               m_hWnd;              // 显示窗口句柄
  HBITMAP            m_hBitmap;           // 位图句柄
  HDC                m_hMemHdc;           // 内存DC句柄
  int                m_iWidth;            // 当前位图宽度
  int                m_iHeight;           // 当前位图的高度

  void              *m_pctx;

  HANDLE             m_hAVMainThread;     // av main thread handle
  HANDLE             m_hAVShutdownEvent;
};

}  // namespace vzplayer

#endif  // SRC_LIB_VZPLAYER2_PLAYER_VZPLAYER_H_
