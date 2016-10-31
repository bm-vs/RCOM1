#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <strings.h>
#include <unistd.h>
#include <string.h>

#define BAUDRATE B9600
#define MODEMDEVICE "/dev/ttyS1"
#define _POSIX_SOURCE 1 /* POSIX compliant source */
#define FALSE 0
#define TRUE 1

#define FLAG 0x7E
#define TXSTART 0x03
#define SETUP 0x03
#define UNACK 0x07
#define DISC 0x0B


int llopen(char *nserial, struct termios *oldtio);
int llwrite();
int llclose(int fd, struct termios *oldtio);
void createControlPacket(char* packet, char* type);
int readPacket(int fd);
void answer();
