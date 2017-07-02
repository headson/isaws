//#include<stdio.h>
//#include<sys/socket.h>
//#include<netinet/in.h>
//#include<arpa/inet.h>
//#include<assert.h>
//#include<unistd.h>
//#include<stdlib.h>
//#include<errno.h>
//#include<string.h>
//#include<sys/types.h>
//#include<fcntl.h>
//#include<aio.h>
#include "vzlogging/logging/vzloggingcpp.h"

#define BUFFER_SIZE 1024

int MAX_LIST = 2;

//void aio_completion_handler( int signo, siginfo_t *info, void *context ) {
//  struct aiocb *req;
//
//
//  /* Ensure it's our signal */
//  if (info->si_signo == SIGIO) {
//
//    req = (struct aiocb *)info->si_value.sival_ptr;
//
//    /* Did the request complete? */
//    if (aio_error( req ) == 0) {
//      /* Request completed successfully, get the return status */
//      ret = aio_return( req );
//    }
//  }
//
//  return;
//}
int main(int argc,char **argv) {
  InitVzLogging(argc, argv);
  ShowVzLoggingAlways();
//  //aio操作所需结构体
//  struct aiocb rd;
//
//  int fd,ret,couter;
//
//  fd = open(argv[1],O_RDONLY);
//  if(fd < 0) {
//    perror("test.txt");
//  }
//
//
//
//  //将rd结构体清空
//  bzero(&rd,sizeof(rd));
//
//
//  //为rd.aio_buf分配空间
//  rd.aio_buf = malloc(BUFFER_SIZE + 1);
//
//  //填充rd结构体
//  rd.aio_fildes = fd;
//  rd.aio_nbytes =  BUFFER_SIZE;
//  rd.aio_offset = 0;
//
//  //进行异步读操作
//  ret = aio_read(&rd);
//  if(ret < 0) {
//    perror("aio_read");
//    exit(1);
//  }
//
//  couter = 0;
////  循环等待异步读操作结束
//  while(aio_error(&rd) == EINPROGRESS) {
//    usleep(1000);
//  }
//  //获取异步读返回值
//  ret = aio_return(&rd);
//
//  printf("return :%d\n",ret);
//
//  LOG(L_INFO).write((const char *)(rd.aio_buf), ret);

  return 0;
}