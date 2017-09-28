/************************************************************************
*Author      : Sober.Peng 17-09-19
*Description :
************************************************************************/
#include "stringhelper.h"

#include <string.h>
#include <stdlib.h>

#include <vector>
#include <string>
#include <sstream>

#include <iconv.h>
#ifndef ICONV_CONST
#define ICONV_CONST const
#endif

#ifdef __cplusplus
namespace vzbase {
#endif // __cplusplus

bool Convert(const char *from_charset, const char *to_charset,
             char *inbuf, size_t inlen,
             char *outbuf, size_t *outlen) {
  // _DBG("\t >> [%s],%d\n", inbuf, inlen);
  // 1.
  iconv_t cd;
  char **pin = &inbuf;
  char **pout = &outbuf;

  // 2.
  cd = iconv_open(to_charset, from_charset);
  if (cd == (iconv_t)-1) {
    // _DBG("ICONV OPEN ERROR %s TO %s \n", from_charset, to_charset);
    return false;
  }
  if (iconv(cd, (const char**)pin, &inlen, pout, outlen) == (size_t)-1) {
    //_DBG("THE FORMAT ERROR %s to %s [%s %d] [%s %d]\n", from_charset, to_charset,
    //     inbuf, inlen, outbuf, *outlen);
    iconv_close(cd);
    return false;
  } else {
    //_DBG("CONVERT SUCCEED %s to %s\n", from_charset, to_charset);
  }
  iconv_close(cd);
  return true;
}

bool isUTF8String(const char *pText, int nLen) {
  // octets to go in this UTF-8 encoded character
  unsigned char chr;
  unsigned int dwOctets;
  bool bAllAscii = true;
  // 循环检测
  dwOctets = 0;
  for (int i = 0; i < nLen; i++) {
    // 取出当前字符
    chr = *(pText + i);
    if ((chr & 0x80) != 0)
      // 当前字符不是 ascii
      bAllAscii = false;
    if (dwOctets == 0) {
      // 7 bit ascii after 7 bit ascii is just fine.  Handle start of encoding case
      if (chr >= 0x80) {
        // count of the leading 1 bits is the number of characters encoded
        do {
          chr <<= 1;
          dwOctets++;
        } while ((chr & 0x80) != 0);
        // count includes this character
        dwOctets--;
        //if (dwOctets == 0)
        if (dwOctets < 2)
          // must start with 11xxxxxx
          return false;
      }
    } else {
      // non-leading bytes must start as 10xxxxxx
      if ((chr & 0xC0) != 0x80)
        return false;
      // processed another octet in encoding
      dwOctets--;
    }
  }
  // End of text.  Check for consistency
  if (dwOctets > 0)
    // anything left over at the end is an error
    return false;
  if (bAllAscii)
    // Not utf-8 if all ascii.  Forces caller to use code pages for conversion
    return false;
  return true;
}

std::string GB2312ToUTF8(const std::string src) {
  if (isUTF8String(src.c_str(), src.size())) {
    return src;
  }

  int dst_size = 2 * src.length() + 16;
  std::vector<unsigned char> dst_buff(dst_size);

  size_t dst_out = dst_size;
  bool res = Convert("GB2312", "UTF-8",
                     (char *)src.c_str(), src.length(),
                     (char *)&dst_buff[0], &dst_out);
  if (res) {
    int dstlen = dst_size - dst_out;
    dst_buff.resize(dstlen);  // 去除多余字符

    return std::string(dst_buff.begin(), dst_buff.end());
  }
  return "";
}

int GB2312ToUTF8_C(const char *src, size_t src_size,
                   char *dst, size_t dst_size) {
  if (true == isUTF8String(src, src_size)) {
    if (dst_size >= src_size) {
      memcpy(dst, src, src_size);
      return src_size;
    }
    return 0;
  }

  std::string src_str;
  src_str.append(src, src_size);

  size_t dst_out = dst_size;
  bool res = Convert("GB2312", "UTF-8",
                     (char *)src_str.c_str(), src_str.size(),
                     (char *)dst, &dst_out);
  if (res) {
    return (dst_size - dst_out);
  }
  return 0;
}

int UTF8ToGB2312_C(const char *src, size_t src_size,
                   char *dst, size_t dst_size) {
  if (false == isUTF8String(src, src_size)) {
    if (dst_size >= src_size) {
      memcpy(dst, src, src_size);
      return src_size;
    }
    return 0;
  }

  std::string src_str;
  src_str.append(src, src_size);

  size_t dst_out = dst_size;
  bool res = Convert("UTF-8", "GB2312",
                     (char *)src_str.c_str(), src_str.size(),
                     (char *)dst, &dst_out);
  if (res) {
    return (dst_size - dst_out);
  }
  return 0;
}

std::string UTF8ToGB2312(const std::string src) {
  if (!isUTF8String(src.c_str(), src.size())) {
    return src;
  }

  size_t dst_size = 2 * src.length() + 16;
  std::vector<unsigned char> dst_buff(dst_size);

  size_t dst_out = dst_size;
  bool res = Convert("UTF-8", "GB2312",
                     (char *)src.c_str(), src.length(),
                     (char *)&dst_buff[0], &dst_out);
  if (res) {
    int dstlen = dst_size - dst_out;
    dst_buff.resize(dstlen);  // 去除多余字符

    return std::string(dst_buff.begin(), dst_buff.end());
  }
  return "";
}

std::string IntToStr(int n) {
  std::stringstream ss;
  ss << n;
  return ss.str();
}

const int MAX_RANDOM_KEY_SIZE = 62;
const char RANDOM_KEY[] = "abcdefghijkrmnopqlstuvwxyzABCDEFGHIJKRMNOPQLSTUVWXYZ0123456789";
std::string GetRandomString(std::size_t size) {
  std::string result;
  for (std::size_t i = 0; i < size; i++) {
    result.push_back(RANDOM_KEY[rand() % MAX_RANDOM_KEY_SIZE]);
  }
  return result;
}

#ifdef __cplusplus
}
#endif // __cplusplus


