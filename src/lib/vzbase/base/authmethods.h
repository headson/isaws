#ifndef ALI_OPENSEARCH_BASE_HELP_METHODS_H_
#define ALI_OPENSEARCH_BASE_HELP_METHODS_H_

#include <string>
//#include "basictypes.h"

#include "vzbase/base/basictypes.h"

namespace vzbase {

  class HelpMethos{
  public:

    static void HmacSha1(const uint8 *key, std::size_t key_size,
      const uint8 *data, std::size_t data_size, uint8 *result);

    static void HmacSha1ToBase64(const std::string &key,
      const std::string &data, std::string &result);
  };

}

#endif // ALI_OPENSEARCH_BASE_HELP_METHODS_H_
