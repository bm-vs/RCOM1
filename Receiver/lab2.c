#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>

#define BAUDRATE B9600
#define _POSIX_SOURCE 1 /* POSIX compliant source */
#define FALSE 0
#define TRUE 1

volatile int STOP=FALSE;


int sendUA(int fd){
	unsigned char UA[5] = {0x7E,0x03,0x07,0x01,0x7E};
	int res, i;
	res = 0;
	i = 0;	
	
	res = write(fd, UA, 5);
	while(i < 5){
		printf("%x\n", UA[i]);
		i++;	
	}
	
	return res;	
}

int readPack(int fd){ 
    unsigned char buf[255];
	unsigned char pack[5];
	int res,i,j;
	res = 0;

	while(STOP == FALSE){

	res = read(fd,buf,1);

	if(res == 1){
		i=0;
		while(buf[0] != 0x7E){
			res=read(fd,buf,1);
			if(res != 1){
				printf("ERROR\n");
				return -1;			
			}
			printf("ERROR24\n");
		}
		pack[i] = buf[0];
printf("%x\n",pack[i]);
		i++;
		res = read(fd,buf,1);
		if(res == 1){
				int j;
				j =0 ;
				while(buf[0] != 0x7E){
					pack[i] = buf[0];
printf("%x\n",pack[i]);
					i++;			
					j++;
					if(j==3)
					break;
					if((res=read(fd,buf,1)) != 1){
printf("ERROR1\n");
						return -1;}
				}
				if(pack[3] == pack[1]^pack[2]){
					read(fd,buf,1);
					if(res == 1){
						while(buf[0] != 0x7E){
						if((res=read(fd,buf,1)) != 1){
printf("ERROR2\n");
						return -1;					}
						}
					pack[i] = buf[0];
printf("%x\n",pack[i]);
					printf("Package received\n");				
					STOP = TRUE; 
					return 0;
					}					
				}
			}											
		}
	}
	return -1;
}

int main(int argc, char** argv)
{
    int fd,c, res;
    struct termios oldtio,newtio;
    char rstring[255];
    int nrstring;
    nrstring = 0;

    if ( (argc < 2) || 
  	     ((strcmp("/dev/ttyS0", argv[1])!=0) && 
  	      (strcmp("/dev/ttyS1", argv[1])!=0) )) {
      printf("Usage:\tnserial SerialPort\n\tex: nserial /dev/ttyS1\n");
      exit(1);
    }


  /*
    Open serial port device for reading and writing and not as controlling tty
    because we don't want to get killed if linenoise sends CTRL-C.
  */
  
    
    fd = open(argv[1], O_RDWR | O_NOCTTY );
    if (fd <0) {perror(argv[1]); exit(-1); }

    if ( tcgetattr(fd,&oldtio) == -1) { /* save current port settings */
      perror("tcgetattr");
      exit(-1);
    }

    bzero(&newtio, sizeof(newtio));
    newtio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;
    newtio.c_iflag = IGNPAR;
    newtio.c_oflag = 0;

    /* set input mode (non-canonical, no echo,...) */
    newtio.c_lflag = 0;

    newtio.c_cc[VTIME]    = 0;   /* inter-character timer unused */
    newtio.c_cc[VMIN]     = 1;   /* blocking read until 5 chars received */



  /* 
    VTIME e VMIN devem ser alterados de forma a proteger com um temporizador a 
    leitura do(s) próximo(s) caracter(es)
  */



    tcflush(fd, TCIOFLUSH);

    if ( tcsetattr(fd,TCSANOW,&newtio) == -1) {
      perror("tcsetattr");
      exit(-1);
    }

    printf("New termios structure set\n");
	
	
	if(readPack(fd) == 0)
		sendUA(fd);

    tcsetattr(fd,TCSANOW,&oldtio);
    close(fd);
    return 0;
}