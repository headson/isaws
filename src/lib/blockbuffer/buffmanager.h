#ifndef BLOCK_MANAGER_H_
#define BLOCK_MANAGER_H_

#include "boost/shared_ptr.hpp"

#include "buffer.h"

class BufferManager {
 public:
  static BufferManager *Instance();
  static void Destory();
  static bool SetBlocks(int blocks);
  VzBuff::Ptr TackBuff(int size);

 private:
  BufferManager();
  ~BufferManager();
  bool Init();
  static void RecycleBuff(VzBuff *p);
  static void DeleteAllBuff(VzBuff *p);

 private:
  PBlock::Ptr resource_;
  VzBuff::Ptr buff_resource_;

  static BufferManager* ins_;
};


#endif // BLOCK_MANAGER_H_