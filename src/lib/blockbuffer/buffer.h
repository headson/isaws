#ifndef BLOCK_BUFFER_H_
#define BLOCK_BUFFER_H_

#include "boost/shared_ptr.hpp"

#include "block.h"

class PBlock {
 public:
  typedef boost::shared_ptr<PBlock> Ptr;
  PBlock();
  ~PBlock();
  Block *block_;
};

class DeleteOrRecycleBlock {
 public:
  DeleteOrRecycleBlock(PBlock::Ptr);
  void operator()(Block *p);

 private:
  PBlock::Ptr resource_;
};

class VzBuffIteraotr {
 public:
  VzBuffIteraotr(VzBuffIteraotr &other);
  ~VzBuffIteraotr();

  VzBuffIteraotr &operator++();
  const VzBuffIteraotr operator++(int);

  VzBuffIteraotr &operator--();
  const VzBuffIteraotr operator--(int);

  Block *operator->() {return block_;}
  Block &operator*() {return *block_;}

  bool operator==(VzBuffIteraotr &other) {return other.block_ == block_;}
  bool operator!=(VzBuffIteraotr &other) {return other.block_ != block_;}

 private:
  VzBuffIteraotr(Block *t, Block *f, Block *b);

 private:
  Block *block_;
  Block *back_;
  Block *front_;

  friend class VzBuff;
};

// VzBuff 至少需要有一个Block_对象
class VzBuff {
 public:
  typedef boost::shared_ptr<VzBuff> Ptr;
  inline int count();
  inline Block *front();
  inline Block *back();
  inline Block::Ptr tack_front();
  inline Block::Ptr tack_back();
  inline void pop_front();
  inline void pop_back();
  inline void push_front(Block::Ptr*);
  inline void push_back(Block::Ptr*);
  inline VzBuffIteraotr begin();
  inline VzBuffIteraotr end();

  int Write(void *data, int data_size, int offset);
  int Read(void *data, int data_size, int offset);

 private:
  VzBuff();
  ~VzBuff();
  bool Init(PBlock::Ptr header, Ptr buffer_recycle_, int size);
  void Clean();
  int GetBlock(Block **front, Block **back, int count);
  static void PtrCallback(Block *p, PBlock::Ptr resource);
  inline void pop(Block *p);
  inline void push_befor(Block *a, Block *toinsert);
  inline void push_after(Block *a, Block *toinsert);

 private:
  PBlock::Ptr recycle_;
  Block *front_;
  Block *back_;
  int capacity_;

  enum {
    Inited = 0,
    Cleand
  } status_;

  Ptr     buffer_recycle_;
  VzBuff *next_buffer_;
  friend class BufferManager;
};

//////////////////////////////////////////////////////////////////////////////

void VzBuff::pop(Block *p) {
  if (p->next_) {
    p->next_->prv_ = p->prv_;
  }
  if (p->prv_) {
    p->prv_->next_ = p->next_;
  }
  p->next_ = p->prv_ = NULL;
}

void VzBuff::push_befor(Block *head, Block *toinsert) {
  if (head == NULL) return;

  toinsert->prv_ = head->prv_;
  head->prv_ = toinsert;

  if (toinsert->prv_) {
    toinsert->prv_->next_ = toinsert;
  }
  toinsert->next_ = head;
}

void VzBuff::push_after(Block *head, Block *toinsert) {
  if (head == NULL) return;

  toinsert->next_ = head->next_;
  head->next_ = toinsert;

  if (toinsert->next_) {
    toinsert->next_->prv_ = toinsert;
  }
  toinsert->prv_ = head;
}

int VzBuff::count() {
  return capacity_ / Block::kBuffSize;
}

Block *VzBuff::front() {
  return front_;
}

Block *VzBuff::back() {
  return back_;
}

void VzBuff::pop_front() {
  if (capacity_ == 0) {
    return;
  }
  capacity_ -= Block::kBuffSize;

  Block *ret = front_;
  front_ = front_->next_;
  pop(ret);
  push_after(recycle_->block_, ret);
  recycle_->block_ = ret;
}

void VzBuff::pop_back() {
  if (capacity_ == 0) {
    return;
  }
  capacity_ -= Block::kBuffSize;

  Block *ret = back_;
  back_ = back_->prv_;
  pop(ret);
  push_after(recycle_->block_, ret);
  recycle_->block_ = ret;
}

void VzBuff::push_front(Block::Ptr *p) {
  capacity_ += Block::kBuffSize;

  Block *fnt = (*p).get();
  // prv_ 设置为-1 只能指针回调函数将不会对fnt进行回收
  fnt->prv_ = (Block *)(-1);
  (*p).reset();

  push_befor(front_, fnt);
  front_ = fnt;
}

void VzBuff::push_back(Block::Ptr *p) {
  capacity_ += Block::kBuffSize;

  Block *fnt = (*p).get();
  // prv_ 设置为-1 只能指针回调函数将不会对fnt进行回收
  fnt->prv_ = (Block *)(-1);
  (*p).reset();

  push_after(back_, fnt);
  back_ = fnt;
}

Block::Ptr VzBuff::tack_front() {
  if (capacity_ == 0) {
    return Block::Ptr();
  }
  capacity_ -= Block::kBuffSize;

  Block *fnt = front_;
  front_ = front_->next_;
  front_->prv_ = NULL;

  fnt->next_ = fnt->prv_ = NULL;
  return Block::Ptr(fnt, DeleteOrRecycleBlock(recycle_));
}

Block::Ptr VzBuff::tack_back() {
  if (capacity_ == 0) {
    return Block::Ptr();
  }
  capacity_ -= Block::kBuffSize;

  Block *fnt = back_;
  back_ = back_->prv_;
  back_->next_ = NULL;

  fnt->next_ = fnt->prv_ = NULL;
  return Block::Ptr(fnt, DeleteOrRecycleBlock(recycle_));
}

VzBuffIteraotr VzBuff::begin() {
  return VzBuffIteraotr(front_, front_, back_);
}

VzBuffIteraotr VzBuff::end() {
  return VzBuffIteraotr(NULL, front_, back_);
}

#endif // BLOCK_BUFFER_H_