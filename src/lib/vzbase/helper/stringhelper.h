/************************************************************************
*Author      : Sober.Peng 17-09-19
*Description :
************************************************************************/
#ifndef LIBVZBASE_STRINGHELPER_H_
#define LIBVZBASE_STRINGHELPER_H_

#ifdef __cplusplus
#include <string>

namespace vzbase {
extern "C" {
#endif // __cplusplus

int GB2312ToUTF8_C(const char *src, size_t src_size,
                   char *dst, size_t dst_size);
int UTF8ToGB2312_C(const char *src, size_t src_size,
                   char *dst, size_t dst_size);

#ifdef __cplusplus
bool Convert(const char *from_charset, const char *to_charset,
             char *inbuf, size_t inlen,
             char *outbuf, size_t *outlen);

// GB2312 与 UTF-8 互转
bool isUTF8String(const char *pText, int nLen);
std::string GB2312ToUTF8(const std::string src);
std::string UTF8ToGB2312(const std::string src);

// int转str
std::string IntToStr(int n);
// 获取随机字符串
std::string GetRandomString(std::size_t size);
#endif

#ifdef __cplusplus
};
}
#endif // __cplusplus


#endif  // LIBVZBASE_STRINGHELPER_H_
