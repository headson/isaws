#include "buffmanager.h"

#include "block.h"

BufferManager* BufferManager::ins_ = NULL;

BufferManager *BufferManager::Instance() {
  if (ins_ == NULL) {
    ins_ = new BufferManager();
    if (!ins_->Init()) {
      delete ins_;
      ins_ = NULL;
    }
  }
  return ins_;
}

bool BufferManager::SetBlocks(int blocks) {
  return Block::SetBlocks(blocks);
}

bool BufferManager::Init() {
  resource_.reset(new PBlock());
  buff_resource_.reset(new VzBuff(), &BufferManager::DeleteAllBuff);
  return true;
}

void BufferManager::Destory() {
  if (ins_) {
    delete ins_;
    ins_ = NULL;
  }
}

BufferManager::BufferManager() {}

BufferManager::~BufferManager() {}

VzBuff::Ptr BufferManager::TackBuff(int size) {
  VzBuff::Ptr vzbuf;
  if (buff_resource_->next_buffer_) {
    vzbuf.reset(buff_resource_->next_buffer_, &BufferManager::RecycleBuff);
    buff_resource_->next_buffer_ = buff_resource_->next_buffer_->next_buffer_;
  } else {
    vzbuf.reset(new VzBuff(), &BufferManager::RecycleBuff);
  }
  if (!vzbuf->Init(resource_, buff_resource_, size)) {
    vzbuf.reset();
  }
  return vzbuf;
}

void BufferManager::DeleteAllBuff(VzBuff *p) {
  VzBuff *tmp;
  while (p) {
    tmp = p->next_buffer_;
    delete p;
    p = tmp;
  }
}

void BufferManager::RecycleBuff(VzBuff *p) {
  VzBuff::Ptr resource = p->buffer_recycle_;
  p->Clean();
  p->next_buffer_ = resource->next_buffer_;
  resource->next_buffer_ = p;
}

