#include "base/base/authmethods.h"

#include <time.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#include "base/base/sha1.h"
#include "base/base/base64.h"

namespace yhbase {

void *memxor(void * dest, const void * src, size_t n) {
  char const *s = (const char *)src;
  char *d = (char *)dest;

  for (; n > 0; n--)
    *d++ ^= *s++;

  return dest;
}

#define IPAD 0X36
#define OPAD 0X5C

void HelpMethos::HmacSha1(const uint8 *key, std::size_t key_size,
                          const uint8 *data, std::size_t data_size, uint8 *result) {

  SHA1_CTX inner;
  SHA1_CTX outer;
  unsigned char key_buf[64] = {0};
  char optkeybuf[20];
  char block[64];
  char innerhash[20];

  /* Reduce the key's size, so that it becomes <= 64 bytes large.  */

  if (key_size > 64) {
    SHA1_CTX keyhash;

    SHA1Init(&keyhash);
    SHA1Update(&keyhash, (const unsigned char *)key, key_size);
    SHA1Final(&keyhash, (unsigned char *)optkeybuf);

    memcpy(key_buf, optkeybuf, 20);
    key_size = 20;
  } else {
    memcpy(key_buf, key, key_size);
  }

  /* Compute INNERHASH from KEY and IN.  */

  SHA1Init(&inner);

  memset(block, IPAD, sizeof(block));
  memxor(block, key_buf, key_size);

  SHA1Update(&inner, (const unsigned char *)block, 64);
  SHA1Update(&inner, (const unsigned char *)data, data_size);

  SHA1Final(&inner, (unsigned char *)innerhash);

  /* Compute result from KEY and INNERHASH.  */

  SHA1Init(&outer);

  memset(block, OPAD, sizeof(block));
  memxor(block, key_buf, key_size);

  SHA1Update(&outer, (const unsigned char *)block, 64);
  SHA1Update(&outer, (const unsigned char *)innerhash, 20);

  SHA1Final(&outer, (unsigned char *)result);
}

void HelpMethos::HmacSha1ToBase64(const std::string &key,
                                  const std::string &data, std::string &result) {
  char *pbase64_buffer=NULL;
  unsigned char digest_buffer[SHA1_DIGEST_SIZE];

  HmacSha1((const unsigned char *)key.c_str(), key.size(),
           (const unsigned char *)data.c_str(), data.size(), digest_buffer);

  Base64::EncodeFromArray(digest_buffer, SHA1_DIGEST_SIZE, &result);
  //Base64::EncodeFromArray((const char *)digest_buffer,
  //                        SHA1_DIGEST_SIZE,
  //                        Base64::DO_STRICT,
  //                        &result,
  //                        NULL);
}

}
