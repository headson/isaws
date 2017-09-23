// Vision Zenith System Communication Protocol (Project)

#include "vzbase/base/helper.h"
#include "vzbase/base/timeutils.h"
#include <sstream>
#include <time.h>
#include <stdio.h>
#include <string.h>
#include <stdio.h>
#include "md5.h"
#include <stdlib.h>

#ifdef WIN32
#include <Windows.h>
#else
#include <unistd.h>
#include <sys/time.h>
#include <iconv.h>
#endif

namespace vzbase {

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

//#ifdef WIN32
//int gettimeofday(timeval *tp, void *tzp) {
//  time_t clock;
//  struct tm tm;
//  SYSTEMTIME wtm;
//  GetLocalTime(&wtm);
//  tm.tm_year     = wtm.wYear - 1900;
//  tm.tm_mon     = wtm.wMonth - 1;
//  tm.tm_mday     = wtm.wDay;
//  tm.tm_hour     = wtm.wHour;
//  tm.tm_min     = wtm.wMinute;
//  tm.tm_sec     = wtm.wSecond;
//  tm.tm_isdst    = -1;
//  clock = mktime(&tm);
//  tp->tv_sec = (unsigned long)clock;
//  tp->tv_usec = wtm.wMilliseconds * 1000;
//  return (0);
//}
//#endif

uint64 GetVzTime() {
  return vzbase::TimeNanos();
}

uint32 GetVzTimeSec() {
  return vzbase::Time();
}

// 0	��	13	��	26	��
// 1	��	14	³	27	��
// 2	��	15	ԥ	28	��
// 3	��	16	��	29	��
// 4	��	17	��	30	��
// 5	��	18	��	31	��
// 6	��	19	��	32	��
// 7	��	20	��	33	̨
// 8	��	21	��	34	��
// 9	��	22	��	35	ʹ
//10	��	23	��	36	WJ
//11	��	24	��	37	��
//12	��	25	��	38	ѧ

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

const std::string CalculationsFileMd5(const std::string &file_name) {
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
const char NAGATIVE_STR[MAX_CHAR]     =    "��";
const char DIGIT_INDEX[][MAX_CHAR]    =    {"","ʮ","��","ǧ","��","ʮ","��","ǧ"};
// const char NUMBER_INDEX[][MAX_CHAR]=    {"��","Ҽ","��","��","��","��","½","��","��","��"};
const char NUMBER_INDEX[][MAX_CHAR]   =    {"0","Ҽ","2","3","4","5","6","7","8","9"};

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
    result += "��";
    return result;
  }

  int Last = 1;
  for(int i = 0 ; i < num_str_size; i++) {
    //�жϺ�����������Ƿ�ȫ��Ϊ�㣬����ǣ�����һ�� ����
    if(IsAllZero(num_str,i)) {
      if((num_str_size - i) > 3) {
        result += "��";
      }
      break;
    }
    //�����һ���������㣬����һ�����������㣬��������
    if(num_str[i] == '0' && Last != 0) {
      //�����һ�������򼶱���������򼶱�ʣ�µĶ����㣬���򼶱��������һ�£�����iֵ ��ǧ������
      if(num_str_size - i > 4 && IsBigAllZero(num_str,i)) {
        result += "����";
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

}; // namespace vzbase
