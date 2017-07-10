/************************************************************************/
/* Author      : Sober.Peng 17-06-19                                    */
/* Description :                                                        */
/************************************************************************/
#ifndef LIBVZCONN_BYTEORDER_H_
#define LIBVZCONN_BYTEORDER_H_

#ifdef POSIX
#include <arpa/inet.h>
#endif

#ifdef WIN32
#include <stdlib.h>
#endif

#include "vzbase/base/basictypes.h"

namespace vzconn {

enum ByteOrder {
  ORDER_NETWORK = 0,  // Default, use network byte order (big endian).
  ORDER_HOST,         // Use the native order of the host.
};

// Reading and writing of little and big-endian numbers from memory
// TODO: Optimized versions, with direct read/writes of
// integers in host-endian format, when the platform supports it.

inline void Set8(void* memory, size_t offset, unsigned char v) {
  static_cast<unsigned char*>(memory)[offset] = v;
}

inline unsigned char Get8(const void* memory, size_t offset) {
  return static_cast<const unsigned char*>(memory)[offset];
}

inline void SetBE16(void* memory, unsigned short v) {
  Set8(memory, 0, static_cast<unsigned char>(v >> 8));
  Set8(memory, 1, static_cast<unsigned char>(v >> 0));
}

inline void SetBE32(void* memory, unsigned int v) {
  Set8(memory, 0, static_cast<unsigned char>(v >> 24));
  Set8(memory, 1, static_cast<unsigned char>(v >> 16));
  Set8(memory, 2, static_cast<unsigned char>(v >> 8));
  Set8(memory, 3, static_cast<unsigned char>(v >> 0));
}

inline void SetBE64(void* memory, uint64 v) {
  Set8(memory, 0, static_cast<unsigned char>(v >> 56));
  Set8(memory, 1, static_cast<unsigned char>(v >> 48));
  Set8(memory, 2, static_cast<unsigned char>(v >> 40));
  Set8(memory, 3, static_cast<unsigned char>(v >> 32));
  Set8(memory, 4, static_cast<unsigned char>(v >> 24));
  Set8(memory, 5, static_cast<unsigned char>(v >> 16));
  Set8(memory, 6, static_cast<unsigned char>(v >> 8));
  Set8(memory, 7, static_cast<unsigned char>(v >> 0));
}

inline unsigned short GetBE16(const void* memory) {
  return static_cast<unsigned short>((Get8(memory, 0) << 8) |
                             (Get8(memory, 1) << 0));
}

inline unsigned int GetBE32(const void* memory) {
  return (static_cast<unsigned int>(Get8(memory, 0)) << 24) |
         (static_cast<unsigned int>(Get8(memory, 1)) << 16) |
         (static_cast<unsigned int>(Get8(memory, 2)) << 8) |
         (static_cast<unsigned int>(Get8(memory, 3)) << 0);
}

inline uint64 GetBE64(const void* memory) {
  return (static_cast<uint64>(Get8(memory, 0)) << 56) |
         (static_cast<uint64>(Get8(memory, 1)) << 48) |
         (static_cast<uint64>(Get8(memory, 2)) << 40) |
         (static_cast<uint64>(Get8(memory, 3)) << 32) |
         (static_cast<uint64>(Get8(memory, 4)) << 24) |
         (static_cast<uint64>(Get8(memory, 5)) << 16) |
         (static_cast<uint64>(Get8(memory, 6)) << 8) |
         (static_cast<uint64>(Get8(memory, 7)) << 0);
}

inline void SetLE16(void* memory, unsigned short v) {
  Set8(memory, 0, static_cast<unsigned char>(v >> 0));
  Set8(memory, 1, static_cast<unsigned char>(v >> 8));
}

inline void SetLE32(void* memory, unsigned int v) {
  Set8(memory, 0, static_cast<unsigned char>(v >> 0));
  Set8(memory, 1, static_cast<unsigned char>(v >> 8));
  Set8(memory, 2, static_cast<unsigned char>(v >> 16));
  Set8(memory, 3, static_cast<unsigned char>(v >> 24));
}

inline void SetLE64(void* memory, uint64 v) {
  Set8(memory, 0, static_cast<unsigned char>(v >> 0));
  Set8(memory, 1, static_cast<unsigned char>(v >> 8));
  Set8(memory, 2, static_cast<unsigned char>(v >> 16));
  Set8(memory, 3, static_cast<unsigned char>(v >> 24));
  Set8(memory, 4, static_cast<unsigned char>(v >> 32));
  Set8(memory, 5, static_cast<unsigned char>(v >> 40));
  Set8(memory, 6, static_cast<unsigned char>(v >> 48));
  Set8(memory, 7, static_cast<unsigned char>(v >> 56));
}

inline unsigned short GetLE16(const void* memory) {
  return static_cast<unsigned short>((Get8(memory, 0) << 0) |
                             (Get8(memory, 1) << 8));
}

inline unsigned int GetLE32(const void* memory) {
  return (static_cast<unsigned int>(Get8(memory, 0)) << 0) |
         (static_cast<unsigned int>(Get8(memory, 1)) << 8) |
         (static_cast<unsigned int>(Get8(memory, 2)) << 16) |
         (static_cast<unsigned int>(Get8(memory, 3)) << 24);
}

inline uint64 GetLE64(const void* memory) {
  return (static_cast<uint64>(Get8(memory, 0)) << 0) |
         (static_cast<uint64>(Get8(memory, 1)) << 8) |
         (static_cast<uint64>(Get8(memory, 2)) << 16) |
         (static_cast<uint64>(Get8(memory, 3)) << 24) |
         (static_cast<uint64>(Get8(memory, 4)) << 32) |
         (static_cast<uint64>(Get8(memory, 5)) << 40) |
         (static_cast<uint64>(Get8(memory, 6)) << 48) |
         (static_cast<uint64>(Get8(memory, 7)) << 56);
}

// Check if the current host is big endian.
inline bool IsHostBigEndian() {
  static const int number = 1;
  return 0 == *reinterpret_cast<const char*>(&number);
}

inline unsigned short HostToNetwork16(unsigned short n) {
  unsigned short result;
  SetBE16(&result, n);
  return result;
}

inline unsigned int HostToNetwork32(unsigned int n) {
  unsigned int result;
  SetBE32(&result, n);
  return result;
}

inline uint64 HostToNetwork64(uint64 n) {
  uint64 result;
  SetBE64(&result, n);
  return result;
}

inline unsigned short NetworkToHost16(unsigned short n) {
  return GetBE16(&n);
}

inline unsigned int NetworkToHost32(unsigned int n) {
  return GetBE32(&n);
}

inline uint64 NetworkToHost64(uint64 n) {
  return GetBE64(&n);
}

}  // namespace vzconn
#endif  // LIBVZCONN_BYTEORDER_H_
