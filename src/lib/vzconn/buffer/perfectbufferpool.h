#ifndef VZCONN_POOL_PERFECT_BUFFER_POOL_H_
#define VZCONN_POOL_PERFECT_BUFFER_POOL_H_

#include "vzbase/base/basictypes.h"

#include <list>
#include <vector>

#include "boost/shared_ptr.hpp"
#include "boost/noncopyable.hpp"

#include "vzconn/buffer/bytebuffer.h"
#include "vzbase/base/criticalsection.h"

namespace vzconn {

class PerfectBufferPool : public boost::noncopyable {
 protected:
  PerfectBufferPool();
  virtual ~PerfectBufferPool();

 public:
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
};

}

#endif // VZCONN_POOL_PERFECT_BUFFER_POOL_H_
