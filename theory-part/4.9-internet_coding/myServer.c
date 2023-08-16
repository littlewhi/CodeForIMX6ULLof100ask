
#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <sys/wait.h>


int sfd, cfd;
struct sockaddr_in saddr, caddr;
int error;
int size;
char buf[1000] = "";
	

void mysigEND(int sig)
{
	kill(0, SIGKILL);
}

void mysigChild(int sig)
{
	while(waitpid(-1, NULL, WNOHANG) > 0)
		;
}

void isConnected()
{
	
	error = listen(sfd, 10);
	if(error < 0)
	{
		fprintf(stderr, "%s\n", strerror(errno));
		close(sfd);
		return;
	}

	while(1)
	{
		memset(&caddr, 0, sizeof(struct sockaddr));
		size = sizeof(caddr);
		cfd =  accept(sfd, (struct sockaddr *)&caddr, &size);
		printf("client :\nfd = %d\nip = %s\nport = %d\n", cfd, inet_ntoa(caddr.sin_addr), caddr.sin_port);
		if(fork() == 0)//子进程
		{
			while(recv(cfd, buf, 999, 0) > 0)
			{
				puts(buf);
			}
			close(cfd);
			exit(0);
		}
	}

}

void noConnect()
{
	size = sizeof(caddr);
	while(1)
	{
		memset(&caddr, 0, sizeof(caddr));
		error = recvfrom(sfd, buf, 999, 0, (struct sockaddr *) &caddr, &size);
		if(error > 0)
		{
			printf("MSG(from %s:%d) : %s ", inet_ntoa(caddr.sin_addr), caddr.sin_port, buf);
		}
	}
}


int main(int argc, char *argv[])
{

	int choice = 0;//默认co
	
	if(argc != 3)
	{
		fprintf(stderr, "usage : %s <port> <no/co> \n", argv[0]);
		return -1;
	}

	if(strcmp("no", argv[2]) == 0)
	{
		choice = 1;
	}

	/* 对子进程的处理 */
	//signal(SIGINT, mysigEND);
	signal(SIGCHLD, mysigChild);
	if(choice)
		sfd = socket(AF_INET, SOCK_DGRAM, 0);
	else
		sfd = socket(AF_INET, SOCK_STREAM, 0);
	if(sfd < 0)
	{
		fprintf(stderr, "%s\n", strerror(errno));
		return -1;
	}

	memset(&saddr, 0, sizeof(saddr));
	saddr.sin_addr.s_addr = INADDR_ANY;
	saddr.sin_port = strtoul(argv[1], NULL, 0);
	saddr.sin_family = AF_INET;

	error =  bind(sfd, (struct sockaddr *)&saddr, sizeof(struct sockaddr));
	if(error < 0)
	{
		fprintf(stderr, "%s\n", strerror(errno));
		close(sfd);
		return -1;
	}

	switch(choice)
	{
		case 0:
			isConnected();
			break;
		case 1:
			noConnect();
			break;
		default:
			break;
	}

	close(sfd);
	
	return 0;
}
