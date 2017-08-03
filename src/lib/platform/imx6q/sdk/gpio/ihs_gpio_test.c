#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/select.h>
#include <unistd.h>
#include <signal.h>

#include "ihs_gpio.h"

#ifdef IHS_GPIO

static int fd = 0;

typedef struct  
{
    int eType;     // 类型
    int nState;    // 状态
    int nParam;    // 占空比
} TAG_EXT_GPIO;

int main(int argc, char* argv[])
{
    int fd = 0;
    int bWrite = 1;
    TAG_EXT_GPIO cExt;   

    if(argc < 5)
    {
        printf("app [0=R,1=W] [type] [state] [value]");
        return 0;
    }

    bWrite      = atoi(argv[1]);
    cExt.eType  = atoi(argv[2]);
    cExt.nState = atoi(argv[3]);
    cExt.nParam = atoi(argv[4]);

    fd = open("/dev/ihs_gpio", O_RDWR);
    if(fd < 0)
    {
        perror("open");
        return -1;
    }

    printf("ctrl %d, type %d, state %d, value %d.\n", 
        bWrite, cExt.eType, cExt.nState, cExt.nParam);
    if(bWrite)
    {
        write(fd, &cExt, sizeof(cExt));
    }
    else
    {
        while(1){
            read(fd, &cExt, sizeof(cExt));

            printf("read value is %d %d %d.\n", 
                cExt.eType, cExt.nState, cExt.nParam);
            usleep(500);
        }
    }

    close(fd);
    return 0;
}
#endif // IHS_GPIO
