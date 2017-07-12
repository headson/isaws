// Vision Zenith System Communication Protocol (Project)
#ifndef VZBASE_BASE_HELPER_H_
#define VZBASE_BASE_HELPER_H_

#include "vzbase/base/basictypes.h"
#include <string>

#ifndef WIN32
#define vzsleep(x) usleep(x * 1000);
#else
#define vzsleep(x) Sleep(x)
#endif

namespace vzbase {

#define MAX_PLATE_SIZE 16

const std::string CHAR_SET = "1234567890abcdefghijklmnopqrstuvwxyz";
const std::string NUM_SET = "123456789";
const std::string IntToString(int n);
const std::string NetAddrToIpcAddr(const std::string addr, int port);

bool IsFileExist(const char *file_path);

uint64 GetVzTime();
uint32 GetVzTimeSec();

int Plate2Ascii(const char *plate, uint32 plate_size,
                char *result_buffer, uint32 buffer_size);
int Ascii2Plate(const char *plate, uint32 plate_size,
                char *result_buffer, uint32 buffer_size);

int Gb2312ToUtf8(const char *src, std::size_t src_size,
                 char *des_buffer, std::size_t des_buffer_size);

std::string Gb2312ToUtf8(const std::string src);

bool IsUtf8String(const char *pText, int nLen);

const std::string CalculationsFileMd5(const std::string &file_name);
const std::string Md5DigestToString(unsigned char digest[16]);
const std::string NumberToVoiceString(int n);

const std::string GetRandomString(std::size_t size);
}; // namespace

#endif