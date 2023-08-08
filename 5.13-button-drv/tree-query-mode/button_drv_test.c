#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

/* Usage : name <devname>*/
int main(int argc, char** argv)
{
    char val;

    if(argc != 2)
    {
        fprintf(stderr, "Usage : %s <devname>\n", argv[0]);
        return 1;
    }

    int fd = open(argv[1], O_RDWR);
    if(fd < 0)
    {
        fprintf(stderr, "failed to open %s\n", argv[1]);
        return 1;
    }

    while(1)
    {
        char c;
        printf("enter q is to quit, other is to continue.\n");
        c = getchar();
        if(c != 'q')
            read(fd, &val, 1);
        else
            break;

        printf("%s is %s\n", argv[1], val ? "on" : "off");
        fflush(stdout);
    }
    
    
    close(fd);

    return 0;
}