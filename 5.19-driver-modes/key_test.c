#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <poll.h>
#include <signal.h>

static int fd;

void block_read(void)
{
    short val[4];
    while(1)
    {
        if( getchar() == EOF )
            return;
        if(read(fd, val, sizeof(val)) != sizeof(short) * 2)
        {
            fprintf( stderr, "ERROR : read error\n" );
           return;
        }
        printf( "key%d is %s\n", val[0], val[1] ? "released" : "pressed" );
    }
}

void poll_read(void)
{
    short val[2] = { 0 };

    int timeout = 5000;
    struct pollfd fds[1];
    fds[0].fd = fd;
    fds[0].events = POLLIN;

    while(1)
    {
        if( 1 == poll( fds, sizeof(fds) / sizeof(fds[0]), timeout ) )
        {
            if(fds[0].revents & POLLIN)
            {
                if(read(fd, val, sizeof(val))!= sizeof(short) * 2)
                {
                    fprintf( stderr, "ERROR : read error\n" );
                    return;
                }
                printf( "key%d is %s\n", val[0], val[1] ? "released" : "pressed" );
            }
        }
        else
        {
            fprintf( stderr, "timeout!\n" );
            return;
        }

    }
}

void mySigHandler( int sig )
{ 
    short val[2];
    if( read( fd, val, sizeof(val) ) != sizeof(val))
    {
        fprintf( stderr, "read error\n" );
        return;
    }

    printf( "key%d is %s\n", val[0], val[1] ? "released" : "pressed" );
}
void fasy_read()
{
   

    signal( SIGIO, mySigHandler );

    while(1)
    {
        puts("I am working\n");
        sleep(2);
    }
}

int main(int argc, char **argv)
{
    if(argc != 2)
    {
        printf( "Usage : %s <number> \n", argv[0] );
        printf( "Example : %s 0 \n", argv[0] );
        printf( "Otpions : 0 -> block read\n" );
        printf( " \t 1-> poll read\n" );
        printf( " \t 2-> fasy read\n" );

        return -1;
    }

    fd = open( "/dev/yhb_key", O_RDWR );
    if( fd < 0 )
    {
        fprintf( stderr, "ERROR : Could not open /dev/yhb_key\n" );
        return 1;
    }

    switch( argv[1][0] )
    {
        case '0':
            block_read();
            break;
        case '1':
            poll_read();
            break;
        case '2':
            fasy_read();
            break;
        default:
            printf(" Invalid option number\n");
            break;
    }

    close( fd );

    return 0;
}