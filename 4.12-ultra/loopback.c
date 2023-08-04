
#include "set_opt.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

int main(int argc, char **argv)
{
    int fd;
    char wc, rc;

    if(argc != 2)
    {
        fprintf(stderr, "Usage: %s filename\n", argv[0]);
        return -1;
    }

    /* O_NOCTTY 不让此进程称为打开的终端串口的控制进程 */
    fd = open(argv[1], O_RDWR | O_NOCTTY);
    if(fd < 0)
    {
        fprintf(stderr, "Error opening %s\n", argv[1]);
        return -1;
    }
    if(fcntl(fd, F_SETFL, 0))
    {
        fprintf(stderr, "Error setting 'BLOCK' flags on %s\n", argv[1]);
        return -1;
    }

    set_opt(fd, 115200, 8, 'N', 1);

    while(1)
    {
        printf("please enter a character : ");
        wc = getchar();
        if(wc == EOF) /* 为了方便也可以换成其他字符结束 */
        {
            break;
        }

        if(write(fd, &wc, 1) != 1|| read(fd, &rc, 1) != 1)
        {
            fprintf(stderr, "Error on reading or writing\n");
            break;
        }
        printf("\nECHO : %c\n", rc);
    }

    close(fd);
    return 0;
}