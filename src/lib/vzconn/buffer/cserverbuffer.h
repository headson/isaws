/************************************************************************
*Author      : Sober.Peng 17-07-11
*Description :
************************************************************************/
#ifndef LIBVZCONN_CBLKBUFFPOOL_H_
#define LIBVZCONN_CBLKBUFFPOOL_H_

#include "vzbase/base/basictypes.h"

#include <list>

#include "vzconn/buffer/cblockbuffer.h"

namespace vzconn {

class CSockBuffer {
 public:
  CSockBuffer();
  virtual ~CSockBuffer();

 protected:
   CSockBuffer *prev_;
   CSockBuffer *next_;
};

class CBlkBuffPool {
 public:
  CBlkBuffPool() {
    all_buffer_.clear();
    free_buffer_.clear();
    for (int i = 0; i < kDefSize; i++) {
      CBlockBuffer *p_blk = new CBlockBuffer();
      if (NULL != p_blk) {
        all_buffer_.push_back(p_blk);
        free_buffer_.push_back(p_blk);
      }
    }
  }
  ~CBlkBuffPool() {
    std::list<CBlockBuffer*>::iterator iter;
    for (iter = all_buffer_.begin();
         iter != all_buffer_.end(); iter++) {
      delete (*iter);
    }
    all_buffer_.clear();
    free_buffer_.clear();
  }

  static CBlkBuffPool *Instance() {
    VZBASE_DEFINE_STATIC_LOCAL(CBlkBuffPool, k_blks_pool, ());
    return &k_blks_pool;
  }

  CBlockBuffer *Pop() {
    CBlockBuffer *p_blk = NULL;
    if (free_buffer_.size() <= 0) {
      if (all_buffer_.size() < kMaxSize) {
        p_blk = new CBlockBuffer();
        if (NULL == p_blk) {
          return NULL;
        }
        all_buffer_.push_back(p_blk);
        free_buffer_.push_back(p_blk);

        p_blk = NULL;
      }
    }

    p_blk = free_buffer_.front();
    free_buffer_.pop_front();
    return p_blk;
  }
  bool Push(CBlockBuffer *p_buffer) {
    if (NULL == p_buffer) {
      return false;
    }

    free_buffer_.push_back(p_buffer);
    return true;
  }

 private:
  static const int kDefSize = 5;
  static const int kMaxSize = 25;

 private:
  std::list<CBlockBuffer*> all_buffer_;
  std::list<CBlockBuffer*> free_buffer_;

};

}  // namespace vzconn

#endif  // LIBVZCONN_CBLKBUFFPOOL_H_
