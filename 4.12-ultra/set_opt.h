#include <termios.h>
#include <unistd.h>
#include <strings.h>
#include <stdio.h>

int set_opt(int fd,int nSpeed, int nBits, char nEvent, int nStop);