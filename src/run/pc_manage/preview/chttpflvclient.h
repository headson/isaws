#ifndef SRC_LIB_VZPLAYER2_PLAYER_VZPLAYER_H_
#define SRC_LIB_VZPLAYER2_PLAYER_VZPLAYER_H_

// #define __STDC_CONSTANT_MACROS

#include <QtGui>
#include <QMutex>
#include <QImage>
#include <QThread>

#include <string>

#ifdef __cplusplus
extern "C"
{
#endif

#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libswscale/swscale.h"
#include "libavutil/imgutils.h"
#include "libavutil/pixfmt.h"

#ifdef __cplusplus
};
#endif

#define VZPLAYER_RES_FAILURE -1
#define VZPLAYER_RES_SUCCEED  0

#define VZPLAYER_INDEX_NONE  -1
typedef enum {
  STREAM_TYPE_NULL = -1,
  STREAM_TYPE_VIDEO = 0,
  STREAM_TYPE_AUDIO,
  STREAM_TYPE_INFO,
  STREAM_TYPE_VIDEO2,
  STREAM_TYPE_COUNT
} VzPlayerStreamType;

class CPlayWidget;
class CHttpFlvClient;
typedef void (__stdcall *OnPlayerStatusResponse)(
  int state_code,
  unsigned int param,
  CHttpFlvClient *player,
  void *user_data);

class CHttpFlvClient : public QThread {
  Q_OBJECT

 public:
  explicit CHttpFlvClient(OnPlayerStatusResponse call_back,
                          void *user_data,
                          int auto_relink);
  virtual ~CHttpFlvClient();

  void SetPlayUrl(const char *rtsp_url);

  int  StartPlayer(AVMediaType media_type, CPlayWidget *wnd);
  void Stop();

 private:
  void run();

 private:
  CPlayWidget              *vdo_widget_;
  void                     *user_data_;
  OnPlayerStatusResponse    status_call_back_;

 private:
  QMutex                    vmutex_;
  bool                      image_rorate_;
  volatile bool             thread_running_;

 private:
  int                       auto_relink_;
  std::string               rtsp_url_;
  AVMediaType               media_type_;
};

#endif  // SRC_LIB_VZPLAYER2_PLAYER_VZPLAYER_H_
