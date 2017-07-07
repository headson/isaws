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
typedef void(*Cached_GetFileCallback)(const char *s_path,
                                      int         n_path,
                                      const char *p_data,
                                      int         n_data,
                                      void       *p_usr_arg);

// Return of Cached File Server Interface
#define CACHED_RET_SUCCEED 0
#define CACHED_RET_FAILURE 1

EXPORT_DLL int Cached_Start(const char *s_srv_ip, unsigned short n_srv_port);
EXPORT_DLL int Cached_Stop();

// SetKey, if the path not exist, then create the path
// Return CACHED_RET_SUCCEED, CACHED_RET_FAILURE
EXPORT_DLL int Cached_SaveFile(const char *s_path, int n_path,
                               const char *p_data, int n_data);
// Get the file
// Return CACHED_RET_SUCCEED, CACHED_RET_FAILURE
EXPORT_DLL int Cached_GetFile(const char *s_path, int n_path,
                              Cached_GetFileCallback p_callback,
                              void *p_usr_arg);
// Delete the file
// Return CACHED_RET_SUCCEED, CACHED_RET_FAILURE
EXPORT_DLL int Cached_DeleteFile(const char *s_path, int n_path);

#ifdef __cplusplus
}
#endif

#endif  // LIBFCACHECLIENT_CACHEDCLIENT_C_H_
