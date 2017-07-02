#include "block.h"

#include <stdlib.h>
#include <memory.h>

#include "basedefine.h"

int Block::block_counts = 0;
int Block::counts_limit_ = 256;

Block *Block::CreateNew() {
  Block *buff = new Block();
  if (block_counts > counts_limit_ || !buff->InitBuff()) {
    delete buff;
    buff = NULL;
  }
  return buff;
}

Block::Block()
  : buff_(NULL),
    next_(NULL),
    prv_(NULL) {
  ++block_counts;
}

Block::~Block() {
  FreeBuff();
  --block_counts;
}

bool Block::InitBuff() {
  buff_ = malloc(kBuffSize);
  return buff_ != NULL;
}

void Block::FreeBuff() {
  if (buff_) {
    free(buff_);
  }
}

int Block::Write(void *buff, int size, int start) {
  return IterIO(buff, size, start, true);
}

int Block::Read(void *buff, int size, int start) {
  return IterIO(buff, size, start, false);
}

int Block::IO(void **buff, int *size, int start, bool in) {
  if (kBuffSize <= start || start < 0) {
    return 0;
  }

  // 在偏移条件下，当前block可以拷贝的数据大小
  int copy_size = kBuffSize - start;
  //当前block需要拷贝的数据大小
  copy_size = *size > copy_size ? copy_size : *size;
  if (in) {
    memcpy((char *)buff_ + start, *buff, copy_size);
  } else {
    memcpy(*buff, (char *)buff_ + start, copy_size);
  }
  *size -= copy_size;
  *buff = (char *)(*buff) + copy_size;

  return copy_size;
}

int Block::IterIO(void *buff, int size, int start, bool in) {
  if (buff == NULL || size < 0 || start < 0) {
    return 0;
  }

  Block *iter = this;
  int less = size;
  while (iter) {
    if (kBuffSize > start) {
      iter->IO(&buff, &less, start, in);
      if (less == 0) {
        break;
      }
      start = 0;
    } else {
      start -= kBuffSize;
    }
    iter = iter->next_;
  }

  return size - less;
}