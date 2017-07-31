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
  HWND               m_hWnd;              // ��ʾ���ھ��
  HBITMAP            m_hBitmap;           // λͼ���
  HDC                m_hMemHdc;           // �ڴ�DC���
  int                m_iWidth;            // ��ǰλͼ���
  int                m_iHeight;           // ��ǰλͼ�ĸ߶�

  void              *m_pctx;

  HANDLE             m_hAVMainThread;     // av main thread handle
  HANDLE             m_hAVShutdownEvent;
  DWORD              m_dwAVMainThreadId;  // av main thread id

  int                m_TimCount;

 public:
  BYTE              *m_pBmpData; //ָ��λͼ����������

 public:
  VzPlayer(PlayerCallback *call_back,
           void *user_data,
           int auto_relink);
  ~VzPlayer();

  inline int  SetupDisplayWnd(HWND hwnd);       // ������ʾ����
  int         OpenChannel(const char *purl);    // ����Ƶͨ��
  int         CloseChannel();                   // �ر�ͨ��

 public:
  int         CreateBmp(int w,int h);           ///����λͼ
  void        DestroyBmp();                     // ����λͼ
  int         ScaleFrame(void* frame);          // ������Ƶ֡
  void        ShowFrame();                      // ��ʾ��Ƶ֡
};
