#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

int writeLed(int fd, char* buf)
{
    char sw;

    if(strcmp(buf, "on") == 0)
    {
        sw = 1;
    }
    else if(strcmp(buf, "off") == 0)
    {
        sw = 0;
    }
    else
    {
        printf("Unknown option %s\n", buf);
        fflush(stdout);
    }

    if(write(fd, &sw, sizeof(sw)) != sizeof(sw))
    {
        fprintf(stderr, "write failed\n");
        fflush(stderr);
        return -1;
    }

    return 0;
}

void readLed(int fd)
{
    char val;
    if(read(fd, &val, sizeof(val)) != sizeof(val))
    {
        fprintf(stderr, "read failed\n");
        fflush(stderr);
        return;
    }
 
    printf("/dev/myled %s\n", val ? "on" : "off");
    fflush(stdout);
}

/* Usage : led_drv_test <r|w> <on|off> */
int main(int argc, char **argv)
{
    int fd;

    if(argc != 3)   
    {
        printf("Usage : %s <devname> <r|w> <on|off>\n", argv[0]);
        printf("example : %s /dev/yhb_led0 r\n", argv[0]);
        printf("example : %s /dev/yhb_led1 w on\n", argv[0]);
        return -1;
    }

    fd = open(argv[1], O_RDWR);
    if(fd < 0)
    {
        printf("Failed to open \n");
        return -1;
    }

    if(strcmp(argv[2], "w") == 0)
    {
        return writeLed(fd, argv[3]);
    }
    else if(strcmp(argv[2], "r") == 0)
    {
        readLed(fd);
    }
    else
    {
        printf("Unknown command : %s\n", argv[2]);
    }
    close(fd);

    return 0;
}
