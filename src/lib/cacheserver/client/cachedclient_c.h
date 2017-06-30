/************************************************************************
*Author      : Sober.Peng 17-06-27
*Description : filecache 客户端接口
************************************************************************/
#ifndef LIBFCACHECLIENT_CACHEDCLIENT_C_H_
#define LIBFCACHECLIENT_CACHEDCLIENT_C_H_

#ifdef WIN32
#define EXPORT_DLL _declspec(dllexport)
#else
#define EXPORT_DLL
#endif

#ifdef __cplusplus
extern "C" {
#endif

// Get Cahced File callback function
typedef void(*Cached_GetFileCallback)(const char *path,
                                      int         path_size,
                                      const char *data,
                                      int         data_size,
                                      void       *user_data);

// Return of Cached File Server Interface
#define CACHED_RET_SUCCEED 0
#define CACHED_RET_FAILURE 1

EXPORT_DLL int Cached_Start(const char *server, unsigned short port);
EXPORT_DLL int Cached_Stop();

// SetKey, if the path not exist, then create the path
// Return CACHED_RET_SUCCEED, CACHED_RET_FAILURE
EXPORT_DLL int Cached_SaveFile(const char *path, int path_size,
                               const char *data, int data_size);
// Get the file
// Return CACHED_RET_SUCCEED, CACHED_RET_FAILURE
EXPORT_DLL int Cached_GetFile(const char *path, int path_size,
                              Cached_GetFileCallback call_back,
                              void *user_data);
// Delete the file
// Return CACHED_RET_SUCCEED, CACHED_RET_FAILURE
EXPORT_DLL int Cached_DeleteFile(const char *path, int path_size);

#ifdef __cplusplus
}
#endif

#endif  // LIBFCACHECLIENT_CACHEDCLIENT_C_H_
