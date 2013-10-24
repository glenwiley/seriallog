// seriallog.c
// Glen Wiley <glen.wiley@gmail.com>
//
// read data from the serial interface and log on stdout
//

#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/termios.h>
#include <regex.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

int  g_debug         = 0;
char *DFLT_DEVSERIAL = "/dev/tty.usbserial";
int  DFLT_BAUD       = 9600;

#define MAXLINELEN  100
#define MAXCMDLEN   512

//---------------------------------------- usage
void
usage(void)
{
	printf(
		"USAGE: seriallog [-dt] [-b <baud>] [-D <serialdevice>]\n"
		"\n"
		"-b <baud>           baud rate on serial device\n"
		"                    default=%d\n"
		"-d                  enable debug output\n"
		"-D <serialdevice>   serial device connected to the tagger\n"
		"                    default=%s\n"
		"-t                  print time stamp at start of each line\n"
		"\n"
		"Reads data from the serial port and prints on stdout.\n"
		, DFLT_BAUD, DFLT_DEVSERIAL
	);

	return;
} // usage

//---------------------------------------- openserial
// return file descriptor of open serial device, -1 on error
int
openserial(char *dev, int baud)
{
	int fd = -1;
	struct termios options;

	fd = open(dev, O_RDWR | O_NOCTTY | O_NDELAY);
	if (fd == -1)
	{
		printf("open failed on %s, %d, %s\n", dev, errno, strerror(errno));
		return -1;
	}

	fcntl(fd, F_SETFL, 0);
   
	tcgetattr(fd, &options);
	cfsetispeed(&options, baud);
	cfsetospeed(&options, baud);
	options.c_cflag |= (CLOCAL | CREAD);
	options.c_cflag &= ~CSIZE; /* Mask the character size bits */
	options.c_cflag |= CS8;    /* Select 8 data bits */
	options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
	tcsetattr(fd, TCSANOW, &options);

	return fd;
} // openserial

//---------------------------------------- main
int
main(int argc, char *argv[])
{
	int       err;
	int       tstamp = 0;
	int       fd_ser;
	int       baud   = DFLT_BAUD;
	char      opt;
	char      *devserial = DFLT_DEVSERIAL;
	char      *p;
	char      txt[MAXLINELEN + 2];
	struct tm tmnow;
	FILE      *fh_ser    = NULL;
	time_t    now;

	while((opt = getopt(argc, argv, "b:D:t")) != -1)
	{
		switch(opt)
		{
			case 'b':
				baud = atoi(optarg);
				break;
				
			case 'd':
				g_debug = 1;
				break;
				
			case 'D':
				devserial = strdup(optarg);
				break;
				
			case 't':
				tstamp = 1;
				break;

			default:
				usage();
				exit(1);
				break;
		}
	}

	while(1)
	{
		// if the device is unplugged or blows up we will want to detect it
		// and retry the connection until it is put back together
		// because this runs unattended

		if(fh_ser == NULL)
		{
			if(g_debug)
				fprintf(stderr, "DEBUG: opening serial device %s", devserial);

			fd_ser = openserial(devserial, baud);
			fh_ser = fdopen(fd_ser, "r");
			if(fh_ser == NULL)
			{
				fprintf(stderr, "error opening serial device %s, %s\n"
				 , devserial, strerror(errno));
				sleep(1);
				continue;
			}
		}

		if(fgets(txt, MAXLINELEN, fh_ser) == NULL)
		{
			err = ferror(fh_ser);
			fprintf(stderr, "error on fgets() from serial device %s, %s\n"
			 , devserial, strerror(err));

			if(err != 0)
			{
				clearerr(fh_ser);
				fclose(fh_ser);
				fh_ser = NULL;
				continue;
			}
		}

		if(feof(fh_ser))
		{
			fclose(fh_ser);
			fh_ser = NULL;
		}

		p = strchr(txt, '\n');
		if(p != NULL)
			*p = '\0';

		if(g_debug)
			fprintf(stderr, "DEBUG: input line [%s]\n", txt);

		now = time(NULL);
		localtime_r(&now, &tmnow);

		if(tstamp)
			printf("%.4d%.2d%.2d%.2d%.2d%.2d "
			 , tmnow.tm_year + 1900, tmnow.tm_mon + 1, tmnow.tm_mday
			 , tmnow.tm_hour, tmnow.tm_min, tmnow.tm_sec);

		printf("%s\n", txt);
		fflush(stdout);

		if(g_debug)
		{
			fprintf(stderr, "DEBUG: sleeping 1 second\n");
			sleep(1);
		}
	} // while 1

	return 0;
} // main

// seriallog.c
