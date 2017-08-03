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
    TAG_EXT_GPIO cExt;

    fd = open("/dev/ihs_gpio", O_RDWR);
    if(fd < 0)
    {
        perror("open");
        return -1;
    }

    while (1)
    {
        static int nWatchdog = 0;
        nWatchdog = ((nWatchdog>0) ? 0 : 1);

        cExt.nState = nWatchdog;
        cExt.eType  = E_WWatchdog;

        write(fd, &cExt, sizeof(cExt));

        usleep(500);
    }

    close(fd);
    return 0;
}
#endif // IHS_GPIO
