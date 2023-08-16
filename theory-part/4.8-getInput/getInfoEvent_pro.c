
#include <poll.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <linux/input.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/select.h>
#include <signal.h>
#include <fcntl.h>


	int fd;
	int error;
	int len;
	int i;
	int nfds = 1;
	struct input_event event;
	struct timeval timeout;
	struct pollfd pfd;
	struct input_id id;
	unsigned int evbit[2];
	char *ev_names[] = {
		"EV_SYN ",
		"EV_KEY ",
		"EV_REL ",
		"EV_ABS ",
		"EV_MSC ",
		"EV_SW	",
		"NULL ",
		"NULL ",
		"NULL ",
		"NULL ",
		"NULL ",
		"NULL ",
		"NULL ",
		"NULL ",
		"NULL ",
		"NULL ",
		"NULL ",
		"EV_LED ",
		"EV_SND ",
		"NULL ",
		"EV_REP ",
		"EV_FF	",
		"EV_PWR ",
		};


void selectUse ()
{
	fd_set readset, readyset;
	int nready;
	FD_ZERO(&readset);
	FD_SET(fd, &readset);
	
	
	
	while (1)
	{
		memcpy(&readyset, &readset, sizeof(readset));
		timeout.tv_sec = 5;
		timeout.tv_usec = 0;
		nready = select (fd + 1, &readyset, NULL, NULL, &timeout);
		if (nready > 0)
		{
			if (FD_ISSET (fd, &readyset))
			{
				while ((len = read ( fd, &event, sizeof(event) ) == sizeof(event)))
				{
					if (len > 0)
					{
						printf ("type = %x\ncode = %x\nvalue = %x\n", event.type, event.code, event.value);
					}
					else
					{
						printf ("error1 : %s\n", strerror (errno));
						break;
					}
				}
			}
		}
		else if (!nready)
		{
			printf("TIMEOUT\n");
		}
		else
		{
			printf ("error2 : %s\n", strerror (errno));
		}
	}
	
	
}


void mySignalHandler (int sig)
{
	while (read(fd, &event, sizeof(event)) == sizeof(event))
	{
		printf("get event: type = 0x%x, code = 0x%x, value = 0x%x\n", event.type, event.code, event.value);		
	}
}

void myasy ()
{
	int flags;
	signal(SIGIO, mySignalHandler);
	/* 把进程id告诉驱动 */
	fcntl(fd, F_SETOWN, getpid());
	/* 使驱动能异步通知的标志 */
	flags = fcntl(fd, F_GETFL);
	fcntl(fd, F_SETFL, flags | FASYNC);

	while (1)
	{
		printf("worked\n");
		sleep(1);
	}
}

/* usage : name devname */
int main (int argc, char* argv[])
{
	
	
 
	if (argc != 2)
	{
		fprintf (stderr, "usage : %s <device name>\n", argv[0]);
		return -1;
	}
	
	
	fd = open (argv[1], O_RDWR | O_NONBLOCK);
		
	if (fd < 0)
	{
		fprintf(stderr, "Cannot open %s\n", argv[1]);
		return -1;
	}

	error = ioctl(fd, EVIOCGID, &id);
	if (error == 0)
	{
		printf ("bustype = 0x%x\nvendor = 0x%x\nproduct = 0x%x\nversion = 0x%x\n", id.bustype, id.product, id.vendor, id.version);
	}

	len = ioctl (fd, EVIOCGBIT (0, sizeof(evbit)), &evbit);

	if (len > 0 && len <= sizeof (evbit))
	{
		for (i = 0; i < len; ++i)
		{
			int bit = 0;
			for (; bit < 8; ++bit)
			{
				if(((unsigned char*)evbit)[i] & (1 << bit))
				{
					printf("%s\n", ev_names[i*8 + bit]);
				}
			}
		}
		fflush (stdout);
	}


	pfd.events = POLLIN;
	pfd.fd = fd;
	pfd.revents = 0;

	/* 读信息 */
	//myasy();
	selectUse();
	/*while (1)
	{
		error = poll (&pfd, nfds, 10000);
		if (error > 0)
		{
			len = read (fd, &event, sizeof(event));
			if(len > 0)
			{
				printf ("type = %x\ncode = %x\nvalue = %x\n", event.type, event.code, event.value);
			}
			else if (!len)
			{
				printf ("no read\n");
			}
			else
			{
				printf ("error : %s\n", strerror (errno));
			}
		}
		else if (len < 0)
		{
			printf ("error : %s\n", strerror (errno));
		}
		else
		{
			printf("TIMEOUT\n");
		}
		
	}
		*/
	return 0;
}

