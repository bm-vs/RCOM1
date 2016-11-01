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

    createControlPacket(packet, "SET");

    while(count < 3) {
	write(fd, packet, packet_size);
	flag = 0;
	alarm(3);

	// receive
	if (readPacket(fd) == 0) {
		break;
	}
    }

    if (count == 3) {
         return -1;	
    }

    count = 0; flag = 0;
    return fd;
}


int llwrite(int fd, char *data, int size) {
    // Send SET and wait for UA
    (void) signal(SIGALRM, answer); 
    char packet[PACKET_SIZE];

    int packet_size = createDataPacket(packet, data, size);
/*
    while(count < 3) {
	write(fd, packet, packet_size);
	flag = 0;
	alarm(3);

	// receive
	if (readPacket(fd) == 0) {
		break;
	}
    }

    if (count == 3) {
         return -1;	
    }

    count = 0; flag = 0;
    return fd;*/
}




int llclose(int fd, struct termios *oldtio) {
	// Send DISC and wait for DISC
	(void) signal(SIGALRM, answer); 
	char packet[255];
	int packet_size = 5;


	createControlPacket(packet, "DISC");

	while(count < 3) {
		write(fd, packet, packet_size);
		flag = 0;
		alarm(3);

		// receive
		if (readPacket(fd) == 0) {
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
		createControlPacket(packet, "UA");
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

void createControlPacket(char* packet, char* type) {
	packet[0] = FLAG;
	packet[1] = TXSTART;

	if (!strcmp(type, "SET")) {
		packet[2] = SETUP;
	}
	else if (!strcmp(type, "UA")) {
		packet[2] = UNACK;
	}
	else if (!strcmp(type, "DISC")) {
		packet[2] = DISC;
	}

	packet[3] = packet[1]^packet[2];
	packet[4] = FLAG;
}

int createDataPacket(char *packet, char *data, int size) {
	int i;
	int pos = 0;
	packet[pos++] = FLAG;
	packet[pos++] = TXSTART;
	if (ns == 0) {
		packet[pos++] = 0x00;
	}
	else {
		packet[pos++] = 0x40;
	}	
	packet[pos++] = packet[1]^packet[2];
	
	int xor;
	for (i = 0; i < size; i++) {
		packet[pos++] = data[i];

		if (i == 0) {
			xor = data[i];
		}
		else {
			xor = xor^data[i];
		}
	}
	
	packet[pos++] = xor;
	packet[pos++] = FLAG;

	/*
	for (i = 0; i < pos; i++) {
		printf("%x ", packet[i]);
	}
	printf("\n");
	*/

	return pos;
}


int readPacket(int fd){ 
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
							return 0;
					}					
				}
			}											
		}
	}

	return -1;
}




void answer() {
	flag = 1;
	count++;
	printf("Alarm #%d\n", count);
}
