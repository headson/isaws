#ifndef VZCONN_POOL_PERFECT_BUFFER_POOL_H_
#define VZCONN_POOL_PERFECT_BUFFER_POOL_H_

#include "vzbase/base/basictypes.h"

#include <list>
#include <vector>

#include "vzbase/base/criticalsection.h"
#include "vzbase/base/boost_settings.hpp"

#include "vzconn/buffer/bytebuffer.h"

namespace vzconn {
typedef unsigned int (*NetHeadSizeCB)();
typedef unsigned int (*NetHeadPacketCB)(char *phead, int nhead,
                                        unsigned int nbody, unsigned short eflag);
typedef int          (*NetHeadParseCB)(const char *phead, int nhead, unsigned short *eflag);

class PerfectBufferPool : public vzbase::noncopyable {
 protected:
  PerfectBufferPool();
  virtual ~PerfectBufferPool();

 public:
  void SetOwnHeadSize(size_t nhead);
  static PerfectBufferPool *Instance();

  // Thread safed
  ByteBuffer::Ptr TakeBuffer(std::size_t mini_size);

 private:
  void InsertBuffer(ByteBuffer *buffer);
  ByteBuffer::Ptr TaskPerfectBuffer(std::size_t mini_size);

  // Thread safed
  static void RecyleBuffer(void *bytebuffer);

 private:
  typedef std::list<ByteBuffer *> Buffers;
  Buffers                         buffers_;
  vzbase::CriticalSection         pool_mutex_;

 private:
  size_t  head_size_;
};

#define VzConnBufferPool() PerfectBufferPool::Instance()

}

#endif // VZCONN_POOL_PERFECT_BUFFER_POOL_H_
