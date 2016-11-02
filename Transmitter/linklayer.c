/*Non-Canonical Input Processing*/

#include "linklayer.h"

volatile int STOP=FALSE;
int count = 0, flag = 0;
int ns = 0;

int llopen(char *nserial, struct termios *oldtio)
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


    // Send SET and wait for UA
    (void) signal(SIGALRM, answer); 
    char packet[PACKET_SIZE];
    int packet_size = 5;

	char data[PACKET_SIZE];
    createControlPacket(packet, data, "SET");

/*
	for (i = 0; i < 5; i++) {
		printf("%x\n", packet[i]);
	}
*/

    while(count < 3) {
		write(fd, packet, packet_size);
		flag = 0;
		alarm(3);

		// receive UA
		if (readPacket(fd) == 1) {
			break;
		}
    }

	alarm(0);

    if (count == 3) {
         return -1;	
    }

    count = 0; flag = 0;
    return fd;
}


int llwrite(int fd, char *data, int size) {
    (void) signal(SIGALRM, answer); 
    char packet[PACKET_SIZE];

    int packet_size = createDataPacket(packet, data, size);

    while(count < 3) {
    	//printf("%d\n", packet_size);
		write(fd, packet, packet_size);
		flag = 0;
		alarm(3);

		// receive RR -> continue
		int r = readPacket(fd);
		/*
		if (r == 2) {
			ns = (ns+1) % 2;
			break;
		}
		// receive REJ -> retransmit
		else if (r == 3) {
			count = 0; flag = 0;
		}*/
		if (r == 1) {
			break;
		}

    }

	alarm(0);

    if (count == 3) {
         return -1;	
    }

    count = 0; flag = 0;
    return fd;
}




int llclose(int fd, struct termios *oldtio) {
	// Send DISC and wait for DISC
	(void) signal(SIGALRM, answer); 
	char packet[255];
	int packet_size = 5;
	char data[PACKET_SIZE];

	createControlPacket(packet, data, "DISC");

	while(count < 3) {
		write(fd, packet, packet_size);
		flag = 0;
		alarm(3);

		// receive
		//if (readPacket(fd) == 4) {
		if (readPacket(fd) == 1) {
			break;
		}
	}

	// Connection timeout
	if (count == 3) {
		if (tcsetattr(fd,TCSANOW, oldtio) == -1) {
		perror("tcsetattr");
		exit(-1);
		}
	
	    	close(fd);

		return -1;	
	}
	// Send UA and close
	else {
		createControlPacket(packet, data, "UA");
		write(fd, packet, packet_size);
	
		if (tcsetattr(fd,TCSANOW, oldtio) == -1) {
		perror("tcsetattr");
		exit(-1);
		}
	
	    close(fd);
		count = 0; flag = 0;

		return 0;
	}
}


//===================================================================================================
// Auxiliar

void createControlPacket(char* packet, char *data, char* type) {
	data[0] = FLAG;
	data[1] = TXSTART;

	if (!strcmp(type, "SET")) {
		data[2] = SETUP;
	}
	else if (!strcmp(type, "UA")) {
		data[2] = UNACK;
	}
	else if (!strcmp(type, "DISC")) {
		data[2] = DISC;
	}

	data[3] = data[1]^data[2];
	data[4] = FLAG;

	stuffing(data, 5, packet);
}

int createDataPacket(char *packet, char *data, int size) {
	char *buff = malloc(PACKET_SIZE);
	int i;
	int pos = 0;
	buff[pos++] = FLAG;
	buff[pos++] = TXSTART;
	if (ns == 0) {
		buff[pos++] = 0x00;
	}
	else {
		buff[pos++] = 0x40;
	}	
	buff[pos++] = buff[1]^buff[2];
	
	int xor;
	for (i = 0; i < size; i++) {
		buff[pos++] = data[i];

		if (i == 0) {
			xor = data[i];
		}
		else {
			xor = xor^data[i];
		}
	}
	
	buff[pos++] = xor;
	buff[pos++] = FLAG;

	return stuffing(buff, pos, packet);
}


int readPacket(int fd){ 
    unsigned char buf[255];
	unsigned char pack[5];
	int res,i,j;
	res = 0;
	STOP = FALSE;

	while(STOP == FALSE && !flag) {
		res = read(fd,buf,1);
		if(res == 1) {
			i = 0;

			while(buf[0] != 0x7E) {
				res = read(fd,buf,1);
				if(res != 1) {
					return -1;			
				}
			}

			pack[i] = buf[0];
			i++;
			res = read(fd,buf,1);

			if(res == 1) {
				int j;
				j = 0;

				while(buf[0] != 0x7E) {
					pack[i] = buf[0];
					i++;			
					j++;
					if(j==3)
						break;
					if((res=read(fd,buf,1)) != 1) {
						return -1;}
					}
				
					if(pack[3] == pack[1]^pack[2]) {
						read(fd,buf,1);
						if(res == 1){
							while(buf[0] != 0x7E) {
								if((res=read(fd,buf,1)) != 1) {
									return -1;
								}
							}
		
							pack[i] = buf[0];			
							STOP = TRUE;
							return 1;
/*
							if (pack[2] == UNACK) {
								return 1;
							}							
							else if (pack[2] == RR_0 || pack[2] == RR_1) {
								return 2;
							}
							else if (pack[2] == REJ_0 || pack[2] == REJ_1) {
								return 3;
							}
							else if (pack[2] == DISC) {
								return 4;
							}
*/
					}					
				}
			}											
		}
	}

	
	return -1;
}


int stuffing(char* buf, int arraySize, char* dest){
  int i,flag= 0, size = 0;

  for(i=0;i < arraySize;i++){
    if((buf[i]==0x7E||buf[i]==0x7D) && (i>=3 && i < arraySize-1)){
      dest[size++] = 0x7D;
	  dest[size++] = buf[i] ^ 0x20;
    }
	else dest[size++] = buf[i];
  }

  //printf("%d - ", arraySize);

  return size;
}


void answer() {
	flag = 1;
	count++;
	printf("Alarm #%d\n", count);
}
