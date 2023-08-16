#include <string.h>
#include <sys/types.h>			/* See NOTES */
#include <sys/socket.h>
#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>

#define BUFFERSIZE 1000

int sfd, error;
struct sockaddr_in caddr, saddr;
char buf[BUFFERSIZE] = "";


void totalConnect()
{
	error = connect(sfd, (struct sockaddr *)&saddr, sizeof(struct sockaddr));
	if(error < 0)
	{
		fprintf(stderr, "Cannot connect with server(%s<%d>:%d)\n", "", saddr.sin_addr.s_addr, saddr.sin_port);
		return;
	}

	while(1)
	{
		if(fgets(buf, BUFFERSIZE - 1, stdin))
		{
			error = send(sfd, buf, strlen(buf), 0 );
			if(error < 0)
			{
				fprintf(stderr, "%s\n", strerror(errno));
				close(sfd);
				exit(-1);
			}
		}
	}

	close(sfd);
}

void partConnect()
{
	totalConnect();
}

void noConnect()
{
	while(1)
	{
		if(fgets(buf, BUFFERSIZE - 1, stdin))
		{
			error = sendto(sfd, buf, strlen(buf), 0, (struct sockaddr *)&saddr, sizeof(struct sockaddr));
			if(error < 0)
			{
				fprintf(stderr, "%s\n", strerror(errno));
				fflush(stderr);
				exit(-1);
			}
		}
		
	}
}

/* name <serverip>  <port>*/
int main(int argc, char *argv[])
{
	int choice = 0;//默认是全连接
	
	if(argc != 4)
	{
		printf("usage : %s <severIP> <port> <total/part/no>\n", argv[0]);
		return 0;
	}

	if(strcmp("no", argv[3]) == 0)
		choice = 2;
	else if(strcmp("part", argv[3]) == 0)
		choice = 1;
	if(choice)
		sfd = socket(AF_INET, SOCK_DGRAM, 0);
	else
		sfd = socket(AF_INET, SOCK_STREAM, 0);
		
	if(sfd < 0)
	{
		fprintf(stderr, "%s\n", strerror(errno));
		return -1;
	}

	memset(&saddr, 0, sizeof(caddr));

	error = inet_aton(argv[1], &(saddr.sin_addr));
	if(error < 0)
	{
		fprintf(stderr, "severIP format is wrong\nyour input is %s\n", argv[1]);
		close(sfd);
	}
	saddr.sin_family = AF_INET;
	saddr.sin_port = strtoul(argv[2], NULL, 0);

	switch(choice)
	{
		case 2:
			noConnect();
			break;
		case 1:
			partConnect();
			break;
		case 0:
			totalConnect();
		default:
			break;
	}

	close(sfd);
	
	return 0;
}

