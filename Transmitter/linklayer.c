/*Non-Canonical Input Processing*/

#include "linklayer.h"

int llopen(int nserial, struct termios *oldtio)
{
    int fd, c, res;
    struct termios newtio;
    int i, sum = 0, speed = 0;
    
    
    
    fd = open(nserial, O_RDWR | O_NOCTTY );
    if (fd <0) {perror(nserial); exit(-1); }

    if ( tcgetattr(fd,oldtio) == -1) { /* save current port settings */
      perror("tcgetattr");
      exit(-1);
    }

    bzero(&newtio, sizeof(newtio));
    newtio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;
    newtio.c_iflag = IGNPAR;
    newtio.c_oflag = OPOST;

    /* set input mode (non-canonical, no echo,...) */
    newtio.c_lflag = 0;

    newtio.c_cc[VTIME]    = 1;   /* inter-character timer unused */
    newtio.c_cc[VMIN]     = 0;   /* blocking read until 5 chars received */


    tcflush(fd, TCIFLUSH);

    if ( tcsetattr(fd,TCSANOW,&newtio) == -1) {
      perror("tcsetattr");
      exit(-1);
    }

    return fd;
}
