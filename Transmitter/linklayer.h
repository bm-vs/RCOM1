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
#include "stats.h"

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
#define PACKET_SIZE 256
#define RR_0 0x05
#define RR_1 0x85
#define REJ_0 0x01
#define REJ_1 0x81

int llopen(char *nserial, struct termios *oldtio);
int llwrite(int fd, char *data, int size);
int llclose(int fd, struct termios *oldtio);
void createControlPacket(char* packet, char *data, char* type);
int createDataPacket(char *packet, char *data, int size);
int readPacket(int fd);
int stuffing(char* buf, int arraySize, char* dest);
void answer();
void setNRetransmissions(int n);
void setTimeout(int n);
