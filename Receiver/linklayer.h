#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <stdlib.h>



#define BAUDRATE B9600
#define _POSIX_SOURCE 1 /* POSIX compliant source */
#define FALSE 0
#define TRUE 1
//states
#define START -1
#define FLAG_RCV 0
#define A_RCV 1
#define C_RCV 2
#define BCC_RCV 3
#define STOP_ST 4
//frames
#define FLAG          0x7E
#define A_SENDER      0x03
#define A_RECEIVER    0x01
#define C_SET         0x03
#define C_DISC        0x0B
#define C_UA          0x07
#define C_RR_0        0x05
#define C_RR_1        0x85
#define C_REJ_0       0x01
#define C_REJ_1       0x81
#define ESCAPE        0x7D
#define STUFF_BYTE    0x20

#define FALSE 0
#define TRUE 1


int llread(int fd, char* buffer, int *file);
int llclose(int fd);
int receiveTramaI(int fd, char* frame);
int receiveDISC(int fd, char *frame);
int readUA(int fd);
int llopen(int fd);
int sendUA(int fd);
int readPack(int fd);
int destuffing(char* buf, int arraySize, char* dest);
