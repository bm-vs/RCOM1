#include <stdio.h>
#include "linklayer.h"

int conta = 0, flag = 0;
volatile int STOP=FALSE;

int main(int argc, char** argv) {
	if ((argc < 2) || 
  	     ((strcmp("/dev/ttyS0", argv[1])!=0) && 
  	      (strcmp("/dev/ttyS1", argv[1])!=0) )) {
      printf("Usage:\tnserial SerialPort\n\tex: nserial /dev/ttyS1\n");
      exit(1);
    }
    
    (void) signal(SIGALRM, atende); 
    struct termios oldtio;   
    int fd = llopen(argv[1], &oldtio);
    
    //sending
    char data[255];
	int data_size = 5;
	
	createData(data, "SET");
	
	while(conta < 3) {
		write(fd, data, data_size);
		flag = 0;
		alarm(3);

		// receive
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
