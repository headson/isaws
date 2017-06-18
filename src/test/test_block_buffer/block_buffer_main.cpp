#include "stdafx.h"

#include "vzconn/base/cblockbuffer.h"

int main(int argc, char* argv[]) {

  CBlockBuffer c_blk;

  char s_data[] = "hello world";
  for (int32 i = 0; i < 10000; i++) {
    if (c_blk.FreeSize() >= strlen(s_data)) {
      memcpy(c_blk.GetWritePtr(), s_data, strlen(s_data));
      c_blk.MoveWritePtr(strlen(s_data));
      printf("write size %d, block used size %d, free size %d.\n",
             strlen(s_data), c_blk.UsedSize(), c_blk.FreeSize());
    }
  }

  for (int32 i = 0; i < 10000; i++) {
    //if (c_blk.UsedSize() >= 8) 
    {
      c_blk.MoveReadPtr(7);
      printf("read size 7, block used size %d, free size %d.\n", 
             c_blk.UsedSize(), c_blk.FreeSize());
    }
    if (c_blk.UsedSize() == 0) {
      continue;
    } 
  }

  return 0;
}
