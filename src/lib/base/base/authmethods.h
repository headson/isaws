#ifndef ALI_OPENSEARCH_BASE_HELP_METHODS_H_
#define ALI_OPENSEARCH_BASE_HELP_METHODS_H_

#include "basictypes.h"

#include <string>

namespace yhbase {

class HelpMethos {
 public:

  static void HmacSha1(const uint8 *key, std::size_t key_size,
                       const uint8 *data, std::size_t data_size, uint8 *result);

  static void HmacSha1ToBase64(const std::string &key,
                               const std::string &data, std::string &result);
};

}

#endif // ALI_OPENSEARCH_BASE_HELP_METHODS_H_
