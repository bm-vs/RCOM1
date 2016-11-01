#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
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

int llread(int fd, char* buffer){
	int ok, stuffed, length, error = FALSE, totalsize;
	unsigned char vbbc2;
	char frame[512];
	
	do{
			stuffed = receiveTramaI(fd, frame);
			length = destuffing(frame, frame, stuffed);
			vbbc2 = 0x00;
			
			else if(frame[0] == FLAG &&
					frame[length-1] == FLAG &&
					frame[1] == A_SENDER &&
					//ver git
					frame[3] == (frame[1]^frame[2])){
				for(i = 4;i < length-3;i++ ){
					buffer[i - 4] = frame[i];
				vbcc2 ^= (unsigned char)frame[i];
			}
			if(valbcc2 != (unsigned char)frame[length-2])
				error = TRUE;
		} else error = TRUE;
			
			
			if(error = FALSE){
				ok = true;
				write(fd,RR_1_FRAME,5);
				// ou write(fd,RR_0_FRAME,5);
				totalsize += length;
			}
			else{
				ok = false;
				write(fd,REJ_1_FRAME,5);
				// ou write(fd,REJ_1_FRAME,5);
			}
	} while(!ok)
	
	return totalsize;
}

int llclose(int fd){
	int res;
	char frame[5];
	
	if((res =receiveDISC(fd, frame)) == -1)
		return -1;
	
	else write(fd,DISC,5);
	
	//receive UA;
	
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




int destuffing(char* buf, int arraySize, char* dest){
  int i, size = 0;
        for (i = 0; i < arraySize; i++) {

                if (buf[i] == ESCAPE) {
                        dest[size++] = (buf[i] ^ STUFF_BYTE);
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

