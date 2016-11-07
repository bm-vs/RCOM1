
#include "linklayer.h"

	
const unsigned char DISC[] = {FLAG, A_RECEIVER, C_DISC, A_RECEIVER^C_DISC, FLAG};
const unsigned char RR_0_FRAME[] = {FLAG, A_RECEIVER, C_RR_0, A_RECEIVER^C_RR_0, FLAG};
const unsigned char RR_1_FRAME[] = {FLAG, A_RECEIVER, C_RR_1, A_RECEIVER^C_RR_1, FLAG};
const unsigned char REJ_0_FRAME[] = {FLAG, A_RECEIVER, C_REJ_0, A_RECEIVER^C_REJ_0, FLAG};
const unsigned char REJ_1_FRAME[] = {FLAG, A_RECEIVER, C_REJ_1, A_RECEIVER^C_REJ_1, FLAG};

volatile int STOP=FALSE;
int packet_number = 1;
char previous = 0xFF;

int llread(int fd, char* buffer, int *file){
	int ok, stuffed, length, error = FALSE, totalsize = 1, i, counter;
	unsigned char vbcc2, R = 1,S = 0;
	char frame[512];
	int tempr = 0;	
	
	do{
			printf("Received: %d - ", packet_number);
			stuffed = receiveTramaI(fd, frame);
			//printf("%d - ", stuffed);
			/*
			int a;
			for (a = 0; a < stuffed; a++) {
				printf("%x ", frame[a]);
			}
			printf("\n");
			*/
			length = destuffing(frame, stuffed, frame);
			//printf("%d\n", length);
			vbcc2 = 0x00;
			
			/*
			S = (R + 1) % 2;
			if(frame[2] >> 6 == R){
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
				tempr = 0;
			}*/
			if(frame[0] == FLAG &&
					frame[length-1] == FLAG &&
					frame[1] == A_SENDER &&
					//frame[2] ==  S << 6 &&
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
				tempr = 1;
				}		
			}
			else error = TRUE;	
			if(error == FALSE){
				/*if(tempr == 1){
					R = (R + 1) % 2;*/
				ok = TRUE;
				//}

				if (frame[2] == previous) {
					printf("Duplicate - ");

					if (frame[2] == 0x40) {
						write(fd,RR_0_FRAME,5);
						printf("Sent: RR_0\n");
					}
					else {
						write(fd,RR_1_FRAME,5);
						printf("Sent: RR_1\n");
					}
				}
				else {
					if(frame[2] == 0x40){
					if(write_file(file, buffer,counter) == 3)
						totalsize = 0;

					packet_number++;
					previous = frame[2];				
					write(fd,RR_0_FRAME,5);
					printf("Sent: RR_0\n");
					}
					else {
						if(write_file(file, buffer,counter) == 3)
							totalsize = 0;

						packet_number++;
						previous = frame[2];
						write(fd,RR_1_FRAME,5);
						printf("Sent: RR_1\n");
					}
					if(totalsize != 0)				
					totalsize = 1;
				}
			}
			else{
				ok = FALSE;
				if(frame[2] == 0x40)
					write(fd,REJ_0_FRAME,5);
				else write(fd,REJ_1_FRAME,5);
				printf("Sent: REJ\n");
			}
	} while(!ok);

	return totalsize;
}

int llclose(int fd){
	int res;
	char frame[5];
	
	if((res =receiveDISC(fd, frame)) == -1)
		return -1;
	else {
		printf("Received: DISC\n");
		write(fd,DISC,5);
		printf("Sent: DISC\n");
	}	

	readUA(fd);
	printf("Received: UA\n");
	
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
		printf("Received: SET\n");		
		
		if(sendUA(fd) == 5){
			printf("Sent: UA\n");
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


