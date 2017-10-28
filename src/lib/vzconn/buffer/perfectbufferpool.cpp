/************************************************************************
*Author      : Sober.Peng 17-08-22
*Description :
************************************************************************/
#include "vzconn/buffer/perfectbufferpool.h"

#include "vzbase/helper/stdafx.h"

namespace vzconn {

PerfectBufferPool *PerfectBufferPool::Instance() {
  static PerfectBufferPool pbp;
  return &pbp;
}

PerfectBufferPool::PerfectBufferPool() {
}

PerfectBufferPool::~PerfectBufferPool() {
  LOG(L_WARNING) << "Delete perfect buffer pool";
}

ByteBuffer::Ptr PerfectBufferPool::TakeBuffer(std::size_t mini_size) {
  vzbase::CritScope buffer_mutex(&pool_mutex_);

  if(0 == buffers_.size()) {
    LOG(L_INFO) << "Create ByteBuffer " << buffers_.size() + 1;
    return ByteBuffer::Ptr(new ByteBuffer(), PerfectBufferPool::RecyleBuffer);
  }

  return TaskPerfectBuffer(mini_size);
}

void PerfectBufferPool::InsertBuffer(ByteBuffer *buffer) {
  vzbase::CritScope buffer_mutex(&pool_mutex_);
  buffer->Resize(0);

  Buffers::iterator iter = buffers_.begin();
  for ( ; iter != buffers_.end(); ++iter) {
    if ((*iter)->Capacity() > buffer->Capacity()) {
      buffers_.insert(iter, buffer);
      return;
    }
  }
  buffers_.push_back(buffer);
}

ByteBuffer::Ptr PerfectBufferPool::TaskPerfectBuffer(std::size_t mini_size) {
  BOOST_ASSERT(buffers_.size() > 0);
  // Small -> Big
  ByteBuffer::Ptr perfect_buffer;
  Buffers::iterator iter = buffers_.begin();
  for( ; iter != buffers_.end(); ++iter) {
    if((*iter)->Capacity() >= mini_size) {
      perfect_buffer = ByteBuffer::Ptr(*iter,
                                       PerfectBufferPool::RecyleBuffer);
      buffers_.erase(iter);
      return perfect_buffer;
    }
  }
  perfect_buffer = ByteBuffer::Ptr(buffers_.back(),
                                   PerfectBufferPool::RecyleBuffer);
  buffers_.pop_back();
  if(perfect_buffer->Capacity() < mini_size) {
    perfect_buffer->Resize(mini_size);
  }
  return perfect_buffer;
}

void PerfectBufferPool::RecyleBuffer(void *bytebuffer) {
  PerfectBufferPool::Instance()->InsertBuffer((ByteBuffer *)bytebuffer);
}

}  // namespace pb
