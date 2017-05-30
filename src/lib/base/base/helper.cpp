// Vision Zenith System Communication Protocol (Project)

#include "base/base/helper.h"
#include <sstream>
#include <time.h>
#include <string.h>
#include <stdio.h>
#include "md5.h"
#include <stdlib.h>

#ifdef WIN32
#include <winsock2.h>
#include <windows.h>
#else
#include <unistd.h>
#include <sys/time.h>
#include <iconv.h>
#endif

namespace yhbase {

const std::string IntToString(int n) {
  std::stringstream ss;
  ss << n;
  return ss.str();
}

const std::string NetAddrToIpcAddr(const std::string addr, int port) {
  std::stringstream ss;
  ss << "/tmp/";
  ss << "_";
  ss << port;
  ss << ".sock";
  return ss.str();
}

bool IsFileExist(const char *file_path) {
  if(file_path == NULL) {
    return false;
  }
#ifdef WIN32
  BOOL is_exist = FALSE;
  HANDLE file_handle;
  if (NULL != file_path) {
    // Use the preferred Win32 API call and not the older OpenFile.
    file_handle = CreateFile(
                    file_path,
                    GENERIC_READ,
                    FILE_SHARE_READ | FILE_SHARE_WRITE,
                    NULL,
                    OPEN_EXISTING,
                    0,
                    0);
    if (file_handle != INVALID_HANDLE_VALUE) {
      // If the handle is valid then the file exists.
      CloseHandle(file_handle);
      is_exist = TRUE;
    }
  }
  return is_exist == TRUE;
#else
  return access(file_path, F_OK) == 0;
#endif
  return false;
}

#ifdef WIN32
int gettimeofday(struct timeval *tp, void *tzp) {
  time_t clock;
  struct tm tm;
  SYSTEMTIME wtm;
  GetLocalTime(&wtm);
  tm.tm_year     = wtm.wYear - 1900;
  tm.tm_mon     = wtm.wMonth - 1;
  tm.tm_mday     = wtm.wDay;
  tm.tm_hour     = wtm.wHour;
  tm.tm_min     = wtm.wMinute;
  tm.tm_sec     = wtm.wSecond;
  tm.tm_isdst    = -1;
  clock = mktime(&tm);
  tp->tv_sec = (unsigned long)clock;
  tp->tv_usec = wtm.wMilliseconds * 1000;
  return (0);
}
#endif

uint64 GetVzTime() {
  static struct timeval tv;
#ifndef WIN32
  static struct timezone tz;
  gettimeofday(&tv, &tz);
#else
  gettimeofday(&tv, NULL);
#endif
  return ((uint64)tv.tv_sec) * 1000000 + ((uint64)tv.tv_usec);
}

uint32 GetVzTimeSec() {
  static struct timeval current_time;
#ifndef WIN32
  static struct timezone tz;
  gettimeofday(&current_time, &tz);
#else
  gettimeofday(&current_time, NULL);
#endif
  return ((uint64)current_time.tv_sec);
}

// 0	京	13	赣	26	陕
// 1	津	14	鲁	27	甘
// 2	冀	15	豫	28	青
// 3	晋	16	鄂	29	宁
// 4	蒙	17	湘	30	新
// 5	辽	18	粤	31	港
// 6	吉	19	桂	32	澳
// 7	黑	20	琼	33	台
// 8	沪	21	渝	34	警
// 9	苏	22	川	35	使
//10	浙	23	贵	36	WJ
//11	皖	24	云	37	领
//12	闽	25	藏	38	学

static const unsigned short PLATE_CONVERT_TABLE[] = {
  0xA9BE, 0xF2BD, 0xBDBC, 0xFABD, 0xC9C3,
  0xC9C1, 0xAABC, 0xDABA, 0xA6BB, 0xD5CB,
  0xE3D5, 0xEECD, 0xF6C3, 0xD3B8, 0xB3C2,
  0xA5D4, 0xF5B6, 0xE6CF, 0xC1D4, 0xF0B9,
  0xEDC7, 0xE5D3, 0xA8B4, 0xF3B9, 0xC6D4,
  0xD8B2, 0xC2C9, 0xCAB8, 0xE0C7, 0xFEC4,
  0xC2D0, 0xDBB8, 0xC4B0, 0xA8CC, 0xAFBE,
  0xB9CA, 0x4A57, 0xECC1, 0xA7D1
};

int Plate2Ascii(const char *plate, uint32 plate_size,
                char *result_buffer, uint32 buffer_size) {
  if(buffer_size >= plate_size) {
    memcpy(result_buffer, plate, plate_size);
    return plate_size;
  }
  return 0;
}

int Ascii2Plate(const char *plate, uint32 plate_size,
                char *result_buffer, uint32 buffer_size) {
  if(buffer_size >= plate_size) {
    memcpy(result_buffer, plate, plate_size);
    return plate_size;
  }
  return 0;
}

bool IsUtf8String(const char *pText, int nLen) {
  // octets to go in this UTF-8 encoded character
  uint32 dwOctets;
  uint8 chr;
  bool bAllAscii= true;
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

std::string Gb2312ToUtf8(const std::string src) {
  if(IsUtf8String(src.c_str(), src.size())) {
    return src;
  }
  std::string result;
  char temp[128];
  int res = yhbase::Gb2312ToUtf8(src.c_str(), src.size(), temp, 128);
  if(res == 0) {
    result = src;
  } else {
    result.append(temp, res);
  }
  return result;
}

#ifdef WIN32
int Gb2312ToUtf8(const char *src,
                 std::size_t src_size,
                 char *des_buffer,
                 std::size_t des_buffer_size) {
  if(src == NULL || des_buffer == NULL) {
    return 0;
  }
  std::size_t need_size = WideCharToMultiByte(CP_UTF8,
                          0,
                          (LPCWCH)src,
                          src_size,
                          NULL,
                          0,
                          NULL,
                          NULL);
  if(need_size > des_buffer_size) {
    return 0;
  }
  int convert_size = WideCharToMultiByte(CP_UTF8,
                                         0,
                                         (LPCWCH)src,
                                         src_size,
                                         (LPSTR)des_buffer,
                                         need_size,
                                         NULL,
                                         NULL);
  if(convert_size == 0) {
    return 0;
  }
  return convert_size;
}
#else

bool EncodeConvert(
  const char *from_charset,
  const char *to_charset,
  char *inbuf,
  size_t inlen,
  char *outbuf,
  size_t *outlen) {
  // _DBG("\t >> [%s],%d\n", inbuf, inlen);
  // 1.
  iconv_t cd;
  char **pin = &inbuf;
  char **pout = &outbuf;

  // 2.
  cd = iconv_open(to_charset,from_charset);
  if(cd == (iconv_t) -1) {
    // _DBG("ICONV OPEN ERROR %s TO %s \n", from_charset, to_charset);
    return false;
  }
  if (iconv(cd,pin,&inlen,pout,outlen)==-1) {
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

int Gb2312ToUtf8(const char *src,
                 std::size_t src_size,
                 char *des_buffer,
                 std::size_t des_buffer_size) {

  char in_buf[255] = { 0 };
  char out_buffer[255] = { 0 };
  size_t out_size = des_buffer_size;
  size_t in_size = src_size;

  memcpy(in_buf, src, src_size);

  bool res = EncodeConvert("GB2312", "UTF-8",
                           in_buf, in_size, des_buffer, &out_size);

  if(res) {
    return des_buffer_size - out_size;
  }
  return 0;
}

#endif


const std::string CalculationsFileMd5(
  const std::string &file_name) {

  FILE * fp = fopen(file_name.c_str(), "rb");
  if (fp == NULL) {
    return "";
  }
  md5_state_t state;
  md5_byte_t digest[16];
  md5_init(&state);
  unsigned char temp_buffer[512];
  while (!feof(fp)) {
    int size = fread(temp_buffer, 1, 512, fp);
    md5_append(&state, (const md5_byte_t *)temp_buffer, size);
  }
  md5_finish(&state, digest);
  fclose(fp);
  return Md5DigestToString(digest);
}

const std::string Md5DigestToString(unsigned char digest[16]) {
  std::string result;
  char temp[8];
  for (int i = 0; i < 16; i++) {
    sprintf(temp, "%02x", digest[i]);
    result.append(temp, 2);
  }
  return result;
}


const int  MAX_CHAR                   =    8;
const int  MAX_NUM_STR                =    32;
const char NAGATIVE_STR[MAX_CHAR]     =    "负";
const char DIGIT_INDEX[][MAX_CHAR]    =    {"","十","百","千","万","十","百","千"};
// const char NUMBER_INDEX[][MAX_CHAR]=    {"零","壹","贰","叁","肆","伍","陆","柒","捌","玖"};
const char NUMBER_INDEX[][MAX_CHAR]   =    {"0","壹","2","3","4","5","6","7","8","9"};

int IsAllZero(char Str[],int i) {
  int len = strlen(Str);
  for(int j = i; j < len; j++)
    if(Str[j] != '0')
      return 0;
  return 1;
}

int IsBigAllZero(char Str[],int i) {
  int len = strlen(Str);
  for(int j = i; j < len - 4; j++)
    if(Str[j] != '0')
      return 0;
  return 1;
}

const std::string NumberToVoiceString(int n) {
  std::string result;
  if(n < 0) {
    n = -n;
    result += NAGATIVE_STR;
  }
  char num_str[MAX_NUM_STR];

  sprintf(num_str,"%d",n);

  int num_str_size = strlen(num_str);

  if(n == 0) {
    result += NUMBER_INDEX[0];
    return result;
  }

  if(num_str_size == 5 && (n % 10000) == 0) {
    result += NUMBER_INDEX[num_str[0] - '0'];
    result += "万";
    return result;
  }


  int Last = 1;

  for(int i = 0 ; i < num_str_size; i++) {
    //判断后面的所有数是否全部为零，如果是，处理一下 结束
    if(IsAllZero(num_str,i)) {
      if((num_str_size - i) > 3) {
        result += "万";
      }
      break;
    }
    //如果这一个数等于零，且上一个数不等于零，则输入零
    if(num_str[i] == '0' && Last != 0) {
      //如果这一个数是万级别的数，且万级别剩下的都是零，将万级别的数处理一下，更新i值 从千级别走
      if(num_str_size - i > 4 && IsBigAllZero(num_str,i)) {
        result += "万零";
        i = num_str_size - 5;
        Last = 0;
        continue;
      }
      result += NUMBER_INDEX[num_str[i] - '0'];
      Last = 0;
    }
    if(num_str[i]  != '0') {
      Last = 1;
      result += NUMBER_INDEX[num_str[i] - '0'];
      result += DIGIT_INDEX[num_str_size - i - 1];
    }
  }
  return result;
}

const char RANDOM_KEY[] = "abcdefghijkrmnopqlstuvwxyzABCDEFGHIJKRMNOPQLSTUVWXYZ0123456789";
const int MAX_RANDOM_KEY_SIZE = 62;
const std::string GetRandomString(std::size_t size) {
  std::string result;
  for(std::size_t i = 0; i < size; i++) {
    result.push_back(RANDOM_KEY[rand() % MAX_RANDOM_KEY_SIZE]);
  }
  return result;
}

}; // namespace vzbase
