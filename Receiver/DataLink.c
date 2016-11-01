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

	
const unsigned char DISC[] = {FLAG, A_RECEIVER, C_DISC, A_RECEIVER^C_DISC, FLAG};
const unsigned char RR_0_FRAME[] = {FLAG, A_RECEIVER, C_RR_0, A_RECEIVER^C_RR_0, FLAG};
const unsigned char RR_1_FRAME[] = {FLAG, A_RECEIVER, C_RR_1, A_RECEIVER^C_RR_1, FLAG};
const unsigned char REJ_0_FRAME[] = {FLAG, A_RECEIVER, C_REJ_0, A_RECEIVER^C_REJ_0, FLAG};
const unsigned char REJ_1_FRAME[] = {FLAG, A_RECEIVER, C_REJ_1, A_RECEIVER^C_REJ_1, FLAG};

volatile int STOP=FALSE;


int main(int argc, char** argv)
{
    int fd,c, res;
    struct termios oldtio,newtio;
    char rstring[255];
    int nrstring;
    nrstring = 0;
char* buffer;

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

	int file;

	llopen(fd);

	while(llread(fd, buffer, &file) != 0){
	
	}
	llclose(fd);
	

    tcsetattr(fd,TCSANOW,&oldtio);
    close(fd);
    return 0;
}

int write_file(int *file, char* buffer, int length){
	int res,k,n;
	
	switch(buffer[0]){
	case 1:
		 k = 256*buffer[2]+buffer[3];
		
		int i;			
		for(i = 0;i < k;i++ ){
			write(*file, &buffer[i+4], 1);
		}
		return 1;
		
	case 2:
		if (buffer[n] == 0){
		  n++;
		  n += buffer[n];
		  n++;
		  if (buffer[n] == 1){
		    n++;
		    int filenameSize = buffer[n];
		    n++;
		    char filename[256];
		    memcpy(&filename, &buffer[n], filenameSize);
		    *file = open(filename, O_WRONLY | O_APPEND | O_CREAT | O_TRUNC, 0666);
		  }
		}
		return 2;
	case 3:
		close(*file);
		return 3;	
	}
	
}

int llread(int fd, char* buffer, int *file){
	int ok, stuffed, length, error = FALSE, totalsize, i, counter;
	unsigned char vbcc2, R = 1,S = 0;
	char frame[512];
	int tempr = 0;
	
	
	do{
			stuffed = receiveTramaI(fd, frame);
			//printf("%d - ", stuffed);
			length = destuffing(frame, stuffed, frame);
			//printf("%d\n", length);
			vbcc2 = 0x00;

			 S = (R + 1) % 2;
			/*if(frame[2] >> 7 == R){
				error = FALSE;
				if(frame[2] == 0x40){
					if(write_file(file, buffer,counter) == 3)
						totalsize = 0;						
					write(fd,RR_0_FRAME,5);
				}
				else {
				if(write_file(file, buffer,counter) == 3)
					totalsize = 0;
				write(fd,RR_1_FRAME,5);
				}
				tempr = 1;
			}*/
			if(frame[0] == FLAG &&
					frame[length-1] == FLAG &&
					frame[1] == A_SENDER &&
					//(frame[2] ==  S << 6) &&
					frame[3] == (frame[1]^frame[2])){
					counter = 0;
				for(i = 4;i < length-2;i++ ){
					buffer[i - 4] = frame[i];
					vbcc2 ^= (unsigned char)frame[i];
					counter++;
				}
			if(vbcc2 != (unsigned char)frame[length-2])
				error = TRUE;
			 else {			
				error = FALSE;
				}
				//tempr = 0;		
			}
			else error = TRUE;	
			if(error == FALSE){
				/*if(tempr == 0)
					R = (R + 1) % 2;*/
				ok = TRUE;
				if(frame[2] == 0x40){
					if(write_file(file, buffer,counter) == 3)
						totalsize = 0;						
					write(fd,RR_0_FRAME,5);
				}
				else {
				if(write_file(file, buffer,counter) == 3)
					totalsize = 0;
				write(fd,RR_1_FRAME,5);
				}
				if(totalsize != 0)				
				totalsize += length;
			}
			else{
				ok = FALSE;
				if(frame[2] == 0x40)
					write(fd,REJ_0_FRAME,5);
				else write(fd,REJ_1_FRAME,5);
			}
	} while(!ok);

	return totalsize;
}

int llclose(int fd){
	int res;
	char frame[5];
	
	if((res =receiveDISC(fd, frame)) == -1)
		return -1;
	
	else write(fd,DISC,5);
	
	readUA(fd);
	
}

int receiveTramaI(int fd, char* frame){
	char byte;
	int state = START;
	int counter = 0;

	while(state != STOP_ST){
		int res = read(fd,&byte,1);
		if(res != 1){
			return -1;
		}

		switch(state){
			case START:
			if(byte == FLAG){
				frame[counter] = byte;
				counter++;
				state = FLAG_RCV;
			}
			break;
			case FLAG_RCV:
			if(byte == A_SENDER){
				frame[counter] = byte;
				counter++;
				state = A_RCV;
			}else
			if(byte == FLAG){
				counter = 1;
				state = FLAG_RCV;
			}else {
				counter = 0;
				state = START;
			}
			break;
			case A_RCV:
			if(byte == FLAG){
				counter = 1;
				state = FLAG_RCV;
			}else{
				frame[counter++] = byte;
				state = C_RCV;
			}
			break;
			case C_RCV:
			if(byte == (frame[1] ^ frame[2])){
				frame[counter++] = byte;
				state = BCC_RCV;
			}else
			if(byte == FLAG){
				counter = 1;
				state = FLAG_RCV;
			}else{
				counter = 0;
				state = START;
			}
			break;
			case BCC_RCV:
			if(byte == FLAG){
				frame[counter++] = byte;
				state = STOP_ST;
			}
			else if(byte != FLAG && (frame[2] == 0x00 || frame[2] == 0x40)){ // para I
				frame[counter] = byte;
				counter++;
			}
			break;
		}
	}
	return counter;
}

int receiveDISC(int fd, char *frame){
		char byte;
	int state = START;
	int counter = 0;

	while(state != STOP_ST){
		int res = read(fd,&byte,1);
		if(res != 1){
			return -1;
		}

		switch(state){
			case START:
			if(byte == FLAG){
				frame[counter] = byte;
				counter++;
				state = FLAG_RCV;
			}
			break;
			case FLAG_RCV:
			if(byte == A_SENDER){
				frame[counter] = byte;
				counter++;
				state = A_RCV;
			}else
			if(byte == FLAG){
				counter = 1;
				state = FLAG_RCV;
			}else {
				counter = 0;
				state = START;
			}
			break;
			case A_RCV:
			if(byte == FLAG){
				counter = 1;
				state = FLAG_RCV;
			}else{
				if(byte == C_DISC){
				frame[counter++] = byte;
				state = C_RCV;
				}
				else {
					counter = 0;
					state = START;
				}
			}
			break;
			case C_RCV:
			if(byte == (frame[1] ^ frame[2])){
				frame[counter++] = byte;
				state = BCC_RCV;
			}else
			if(byte == FLAG){
				counter = 1;
				state = FLAG_RCV;
			}else{
				counter = 0;
				state = START;
			}
			break;
			case BCC_RCV:
			if(byte == FLAG){
				frame[counter++] = byte;
				state = STOP_ST;
			}
			else {
				counter = 0;
				state = START;
			}
			break;
		}
	}
	return counter;
}

int readUA(int fd){ 
    unsigned char buf[255];
	unsigned char pack[5];
	int res,i,j;
	res = 0;

	while(STOP == FALSE) {
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
int llopen(int fd){
	
	if(!readPack(fd)){
	if(sendUA(fd) == 5){
		return 0;

}	
	}
return -1;
}
int sendUA(int fd){
	unsigned char UA[5] = {0x7E,0x03,0x07,0x01,0x7E};
	int res, i;
	res = 0;
	i = 0;	
	
	res = write(fd, UA, 5);
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
				return -1;			
			}
		}
		pack[i] = buf[0];
		i++;
		res = read(fd,buf,1);
		if(res == 1){
				int j;
				j =0 ;
				while(buf[0] != 0x7E){
					pack[i] = buf[0];
					i++;			
					j++;
					if(j==3)
					break;
					if((res=read(fd,buf,1)) != 1){
						return -1;}
				}
				if(pack[3] == pack[1]^pack[2]){
					read(fd,buf,1);
					if(res == 1){
						while(buf[0] != 0x7E){
						if((res=read(fd,buf,1)) != 1){
						return -1;					}
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

int destuffing(char* buf, int arraySize, char* dest){
  int i, size = 0;
        for (i = 0; i < arraySize; i++) {

                if (buf[i] == ESCAPE) {
                        dest[size++] = (buf[i+1] ^ STUFF_BYTE);
			i++;
                }
                else
                        dest[size++] = buf[i];
        }

        return size;
}

int stuffing(char* buf, int arraySize, char* dest){
  int i, size = 0;
  
  for(i=0;i< arraySize;i++){
    if(buf[i]==FLAG||buf[i]==ESCAPE){
      dest[size++] = ESCAPE;
	  dest[size++] = buf[i] ^ STUFF_BYTE;
    }
	else dest[size++] = buf[i];
  }
  return size;
}

