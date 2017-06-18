/************************************************************************/
/* Author      : Sober.Peng 17-06-14
/* Description :
/************************************************************************/
#include "stdafx.h"

void app_init()
{
#ifdef WIN32
  WSADATA wsaData;
  WSAStartup(MAKEWORD(2, 2), &wsaData);
  srand((unsigned int)time(NULL));
#else
  srand((unsigned int)time(NULL));
  srandom((unsigned int)time(NULL));
#endif
}

void app_destroy()
{
#ifdef WIN32
  WSACleanup();
#endif
}

//////////////////////////////////////////////////////////////////////////
int InitLogging(int argc, char* argv[]) {
  return ::InitVzLogging(argc, argv);
}
