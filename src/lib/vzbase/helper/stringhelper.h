/************************************************************************
*Author      : Sober.Peng 17-09-19
*Description :
************************************************************************/
#ifndef LIBVZBASE_STRINGHELPER_H_
#define LIBVZBASE_STRINGHELPER_H_

#include <string>

#ifdef __cplusplus
namespace vzbase {
extern "C" {
#endif // __cplusplus

// GB2312 �� UTF-8 ��ת
bool isUTF8String(const char *pText, int nLen);
std::string GB2312ToUTF8(const std::string src);
std::string UTF8ToGB2312(const std::string src);

int GB2312ToUTF8_C(const char *src, std::size_t src_size,
                 char *dst, std::size_t dst_size);

// intתstr
std::string IntToStr(int n);
// ��ȡ����ַ���
std::string GetRandomString(std::size_t size);

#ifdef __cplusplus
};
}
#endif // __cplusplus


#endif  // LIBVZBASE_STRINGHELPER_H_
