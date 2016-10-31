#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>

#define START -1
#define FLAG_RCV 0
#define A_RCV 1
#define C_RCV 2
#define BCC_RCV 3
#define STOP_ST 4




int receiveTrama(int fd, char* frame){
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
			if(byte == A){
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
				frame[counter] = byte;
				counter++;
				state = C_RCV;
			}
			break;
			case C_RCV:
			if(byte == (frame[1] ^ frame[2])){
				frame[counter] = byte;
				counter++;
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
				frame[counter] = byte;
				counter++;
				state = STOP_ST;
			}
			else if(byte != FLAG && (frame[2] == 0x00 || frame[2] == 0x40)){ // para I
				frame[counter] = byte;
				counter++;
			}
			break;
		}
	}
}

int destuffing(char* buf, int arraySize, char* dest){
  int i, size = 0;
        for (i = 0; i < arraySize; i++) {

                if (buf[i] == 0x7D) {
                        dest[size++] = (buf[i] ^ 0x20);
                }
                else
                        dest[size++] = buf[i];
        }

        return size;
}

int stuffing(char* buf, int arraySize, char* dest){
  int i,flag= 0, size = 0;
  
  for(i=0;i< arraySize;i++){
    if(buf[i]==0x7E||buf[i]==0x7D){
      dest[size++] = 0x7D;
	  dest[size++] = buf[i] ^ 0x20;
    }
	else dest[size++] = buf[i];
  }
  return size;
}

