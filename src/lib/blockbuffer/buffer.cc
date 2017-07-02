#include "buffer.h"

PBlock::PBlock()
  : block_(NULL) {
}

PBlock::~PBlock() {
  Block *tmp;
  while (block_) {
    tmp = block_->next_;
    delete block_;
    block_ = tmp;
  }
}

///////////////////////////////////////////////////////////////////////////////

VzBuffIteraotr::VzBuffIteraotr(VzBuffIteraotr &other)
  : block_(other.block_),
    front_(other.front_),
    back_(other.back_) {
}

VzBuffIteraotr::VzBuffIteraotr(Block *t, Block *f, Block *b)
  : block_(t),
    front_(f),
    back_(b) {
}

VzBuffIteraotr::~VzBuffIteraotr() {

}

VzBuffIteraotr &VzBuffIteraotr::operator++() {
  if (block_ == NULL) {
    block_ = front_;
  } else {
    block_ = block_->next_;
  }
  return *this;
}

const VzBuffIteraotr VzBuffIteraotr::operator++(int) {
  VzBuffIteraotr old(block_, front_, back_);
  operator++();
  return old;
}

VzBuffIteraotr &VzBuffIteraotr::operator--() {
  if (block_ == NULL) {
    block_ = back_;
  } else {
    block_ = block_->prv_;
  }
  return *this;
}

const VzBuffIteraotr VzBuffIteraotr::operator--(int) {
  VzBuffIteraotr old(block_, front_, back_);
  operator--();
  return old;
}

///////////////////////////////////////////////////////////////////////////////

bool VzBuff::Init(PBlock::Ptr header, Ptr buffer_recycle, int size) {
  if (status_ == Inited) {
    return true;
  }
  status_ = Inited;

  if (size <= 0 || !header || !buffer_recycle) {
    return false;
  }

  recycle_ = header;
  int count = (size - 1) / Block::kBuffSize + 1;
  if (0 == GetBlock(&front_, &back_, count)) {
    recycle_.reset();
    return false;
  }
  capacity_ = count * Block::kBuffSize;

  buffer_recycle_ = buffer_recycle;
  next_buffer_ = NULL;
  return true;
}

void VzBuff::Clean() {
  if (status_ == Cleand) {
    return ;
  }
  status_ = Cleand;

  if (recycle_->block_) {
    recycle_->block_->prv_ = back_;
  }
  back_->next_ = recycle_->block_;
  recycle_->block_ = front_;
  front_ = back_ = NULL;
  capacity_ = 0;

  recycle_.reset();
  buffer_recycle_.reset();
}

int VzBuff::GetBlock(Block **front, Block **back, int count) {
  // 先分配一个block
  if (recycle_->block_ == NULL) {
    recycle_->block_ = Block::CreateNew();
    if (recycle_->block_ == NULL) {
      return 0;
    }
  }

  Block *iter = recycle_->block_;
  for (int have_count = 1; have_count < count; ++have_count) {
    if (iter->next_ == NULL) {
      iter->next_ = Block::CreateNew();
      if (iter->next_ == NULL) {
        return 0;
      }
      iter->next_->prv_ = iter;
    }
    iter = iter->next_;
  }
  *front = recycle_->block_;
  *back = iter;

  recycle_->block_ = back_->next_;
  if (recycle_->block_) {
    recycle_->block_->prv_ = NULL;
  }

  front_->prv_ = NULL;
  back_->next_ = NULL;
  return count;
}

VzBuff::VzBuff()
  : front_(NULL),
    back_(NULL),
    next_buffer_(NULL),
    capacity_(0),
    status_(Cleand) {
}

VzBuff::~VzBuff() {
  Clean();
}

int VzBuff::Write(void *data, int data_size, int offset) {
  if (data_size <= 0 || offset < 0 || capacity_ < offset + data_size) {
    return 0;
  }

  return front_->Write(data, data_size, offset);
}

int VzBuff::Read(void *data, int data_size, int offset) {
  if (data_size <= 0 || offset < 0 || capacity_ < offset + data_size) {
    return 0;
  }

  return front_->Read(data, data_size, offset);
}

//////////////////////////////////////////////////////////////////////////////

DeleteOrRecycleBlock::DeleteOrRecycleBlock(PBlock::Ptr resource)
  : resource_(resource) {

}

void DeleteOrRecycleBlock::operator()(Block *p) {
  // 使用prv_ 判断需要进行的操作
  if ((int)p->prv_ == -1) {
    return ;
  }
  if (resource_->block_) {
    resource_->block_->prv_ = p;
  }
  p->next_ = resource_->block_;
  p->prv_ = NULL;
  resource_->block_ = p;
}
