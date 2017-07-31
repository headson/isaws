#pragma once
//  CAVPlayer

#include <stdio.h>
#include <windows.h>

#include <string>

#ifdef _AVPLAYER_DEF_
#define AVPLAYER_API _declspec(dllexport)
#else
#define AVPLAYER_API _declspec(dllimport)
#endif

class VzPlayer;

typedef void(__stdcall *PlayerCallback)(const void *data,
                                        unsigned int data_size,
                                        VzPlayer *player,
                                        void *user_data);

class VzPlayerManager {
 public:
  VzPlayerManager();
  virtual ~VzPlayerManager();
  bool VzPlayerManagerSetup();
  bool VzPlayerManagerCleanup();
  VzPlayer *CreateVzPlayer(PlayerCallback *call_back,
                           void *user_data,
                           int auto_relink = 1);
  void DeleteVzPlayer(VzPlayer *player);
};

class AVPLAYER_API VzPlayer {
 private:
  std::string        url_;

 private:
  CRITICAL_SECTION   m_cs;
  HWND               m_hWnd;              // 显示窗口句柄
  HBITMAP            m_hBitmap;           // 位图句柄
  HDC                m_hMemHdc;           // 内存DC句柄
  int                m_iWidth;            // 当前位图宽度
  int                m_iHeight;           // 当前位图的高度

  void              *m_pctx;

  HANDLE             m_hAVMainThread;     // av main thread handle
  HANDLE             m_hAVShutdownEvent;
  DWORD              m_dwAVMainThreadId;  // av main thread id

  int                m_TimCount;

 public:
  BYTE              *m_pBmpData; //指向位图的数据区域

 public:
  VzPlayer(PlayerCallback *call_back,
           void *user_data,
           int auto_relink);
  ~VzPlayer();

  inline int  SetupDisplayWnd(HWND hwnd);       // 设置显示窗口
  int         OpenChannel(const char *purl);    // 打开视频通道
  int         CloseChannel();                   // 关闭通道

 public:
  int         CreateBmp(int w,int h);           ///创建位图
  void        DestroyBmp();                     // 销毁位图
  int         ScaleFrame(void* frame);          // 缩放视频帧
  void        ShowFrame();                      // 显示视频帧
};
