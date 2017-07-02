#ifndef BLOCK_BLOCK_H_
#define BLOCK_BLOCK_H_

#include "boost/shared_ptr.hpp"

class Block {
 public:
  typedef boost::shared_ptr<Block> Ptr;
  static const int kBuffSize = 4096;

  inline static bool SetBlocks(int limit);
  inline static int counts();
  int Write(void *buff, int size, int start = 0);
  int Read(void *buff, int size, int start = 0);
  inline void *Data();

 private:
  // 初始化失败 会返回空指针
  static Block *CreateNew();
  Block();
  ~Block();
  int IO(void **buff, int *size, int start, bool in);
  int IterIO(void *buff, int size, int start, bool in);

  inline bool InitBuff();
  inline void FreeBuff();

 private:
  // 管理的内存起始地址
  void *buff_;

  // 链表
  Block *next_;
  Block *prv_;

  static int block_counts;
  static int counts_limit_;

  friend class VzBuff;
  friend class PBlock;
  friend class VzBuffIteraotr;
  friend class DeleteOrRecycleBlock;
};

bool Block::SetBlocks(int limit) {
  if (limit < block_counts) {
    return false;
  }
  counts_limit_ = limit;
  return true;
}

int Block::counts() {
  return block_counts;
}

void *Block::Data() {
  return buff_;
}

#endif // BLOCK_BLOCK_H_