/*Non-Canonical Input Processing*/

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <signal.h>

#define BAUDRATE B9600
#define MODEMDEVICE "/dev/ttyS1"
#define _POSIX_SOURCE 1 /* POSIX compliant source */
#define FALSE 0
#define TRUE 1

#define FLAG 0x7E
#define TXSTART 0x03
#define SETUP 0x03
#define UNACK 0x07

volatile int STOP=FALSE;

int conta = 0, flag = 0;

void atende();

int main(int argc, char** argv)
{
    int fd,c, res;
    struct termios oldtio,newtio;
    char buf[255];
	char data[255];
    int i, sum = 0, speed = 0;

	(void) signal(SIGALRM, atende);
    
    if ( (argc < 2) || 
  	     ((strcmp("/dev/ttyS0", argv[1])!=0) && 
  	      (strcmp("/dev/ttyS1", argv[1])!=0) )) {
      printf("Usage:\tnserial SerialPort\n\tex: nserial /dev/ttyS1\n");
      exit(1);
    }

    fd = open(argv[1], O_RDWR | O_NOCTTY );
    if (fd <0) {perror(argv[1]); exit(-1); }

    if ( tcgetattr(fd,&oldtio) == -1) { /* save current port settings */
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

    for (i = 0; i < 255; i++) {
      buf[i] = 'a';
    }
    
    /* sending */
	printf("Write: ");
	gets(buf);
	int data_size = 5;
	createData(data, "SET");
	for (i = 0; i < data_size; i++) {
		printf("0x%02x\n", data[i]);
	}	

	while(conta < 3) {
		write(fd, data, data_size);
		flag = 0;
		alarm(3);

		/* receive */
		if (readPack(fd) == 0) {
			break;
		}
	}


    if ( tcsetattr(fd,TCSANOW,&oldtio) == -1) {
      perror("tcsetattr");
      exit(-1);
    }
    close(fd);
    return 0;
}


void createData(char* data, char* type) {
	data[0] = FLAG;
	data[1] = TXSTART;

	if (!strcmp(type, "SET")) {
		data[2] = SETUP;
	}
	else if (!strcmp(type, "UA")) {
		data[2] = UNACK;
	}

	data[3] = data[1]^data[2];
	//data[4] = 0x00;
	data[4] = FLAG;
}


int readPack(int fd){ 
    unsigned char buf[255];
	unsigned char pack[5];
	int res,i,j;
	res = 0;

	while(STOP == FALSE && !flag) {
		res = read(fd,buf,1);
		if(res == 1) {
			i=0;

			while(buf[0] != 0x7E) {
				res = read(fd,buf,1);
				if(res != 1) {
					printf("ERROR\n");
					return -1;			
				}
				
				printf("ERROR24\n");
			}

			pack[i] = buf[0];
			printf("%x\n",pack[i]);
			i++;
			res = read(fd,buf,1);

			if(res == 1) {
				int j;
				j = 0;

				while(buf[0] != 0x7E) {
					pack[i] = buf[0];
					printf("%x\n",pack[i]);
					i++;			
					j++;
					if(j==3)
						break;
					if((res=read(fd,buf,1)) != 1) {
						printf("ERROR1\n");
						return -1;}
					}
				
					if(pack[3] == pack[1]^pack[2]) {
						read(fd,buf,1);
						if(res == 1){
							while(buf[0] != 0x7E) {
								if((res=read(fd,buf,1)) != 1) {
									printf("ERROR2\n");
									return -1;
								}
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


void atende() {
	flag = 1;
	conta++;
	printf("Alarme #%d\n", conta);
}
