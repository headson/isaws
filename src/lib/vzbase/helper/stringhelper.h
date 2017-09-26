/************************************************************************
*Author      : Sober.Peng 17-09-19
*Description :
************************************************************************/
#ifndef LIBVZBASE_STRINGHELPER_H_
#define LIBVZBASE_STRINGHELPER_H_

#ifdef __cplusplus
#include <string>
#endif

#ifdef __cplusplus
namespace vzbase {
extern "C" {
#endif // __cplusplus

int Convert(const char *from, const char *to,
            char *src, int srclen,
            char *dst, int dstlen);

int GB2312ToUTF8_C(const char *src, size_t src_size,
                   char *dst, size_t dst_size);
int UTF8ToGB2312_C(const char *src, size_t src_size,
                   char *dst, size_t dst_size);

#ifdef __cplusplus
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
