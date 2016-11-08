#include "linklayer.h"


const unsigned char UA[] = {FLAG, A_SENDER, C_UA, A_SENDER^C_UA, FLAG};
const unsigned char DISC[] = {FLAG, A_RECEIVER, C_DISC, A_RECEIVER^C_DISC, FLAG};
const unsigned char RR_0_FRAME[] = {FLAG, A_RECEIVER, C_RR_0, A_RECEIVER^C_RR_0, FLAG};
const unsigned char RR_1_FRAME[] = {FLAG, A_RECEIVER, C_RR_1, A_RECEIVER^C_RR_1, FLAG};
const unsigned char REJ_0_FRAME[] = {FLAG, A_RECEIVER, C_REJ_0, A_RECEIVER^C_REJ_0, FLAG};
const unsigned char REJ_1_FRAME[] = {FLAG, A_RECEIVER, C_REJ_1, A_RECEIVER^C_REJ_1, FLAG};

volatile int STOP=FALSE;
int packet_number = 1;
char previous = 0xFF;
int gen_error = 0;

int llopen(int fd){
	char frame[5];
	int res = 0;
	if((res = receiveTrama(fd, frame)) != 5 && frame[2] != C_SET)
		return -1;
	else printf("Received: SET\n");		
		
		write(fd,UA,5);
		printf("Sent: UA\n");	
	 return -1;
}


int llread(int fd, char* buffer, int *file){
	int ok, stuffed, length, error = FALSE, totalsize = 1, i, counter;
	unsigned char vbcc2, R = 1,S = 0;
	char frame[512];
	int tempr = 0;	
	
	do{
			printf("Received: %d - ", packet_number);
			stuffed = receiveTrama(fd, frame);
			if(frame[2] != 0x40 && frame[2] != 0x00){
				printf("frame I expected");
				return -1;
			}

			/*
			printf("%d - ", stuffed);
			printf("\n");
			*/

			length = destuffing(frame, stuffed, frame);
			//printf("%d\n", length);

			if(gen_error == 1)
				generateError(frame);
			vbcc2 = 0x00;			

			if(frame[0] == FLAG &&
					frame[length-1] == FLAG &&
					frame[1] == A_SENDER &&
					(frame[2] == 0x40 || frame[2] == 0x00) &&
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
				ok = TRUE;
				if (frame[2] == previous) {
					printf("Duplicate - ");
					incFramesRepeated();
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
					previous = frame[2];				
					write(fd,RR_0_FRAME,5);
					printf("Sent: RR_0\n");
					}
					else {
						if(write_file(file, buffer,counter) == 3)
							totalsize = 0;
						previous = frame[2];
						write(fd,RR_1_FRAME,5);
						printf("Sent: RR_1\n");
					}
					packet_number++;
					incFramesReceived();
					if(totalsize != 0)				
					totalsize += length;
				}
			}
			else{
				ok = FALSE;
				if(frame[2] == 0x40)
					write(fd,REJ_0_FRAME,5);
				else write(fd,REJ_1_FRAME,5);
				incRejectsSent();
				printf("Sent: REJ\n");
			}
	} while(!ok);

	return totalsize;
}

int llclose(int fd){
	int res;
	char frame[5];
	
	if((res = receiveTrama(fd, frame)) != 5 && frame[2] != C_DISC)
		return -1;
	else {
		printf("Received: DISC\n");
		write(fd,DISC,5);
		printf("Sent: DISC\n");
	}	

	if((res = receiveTrama(fd, frame)) != 5 && frame[2] != C_UA)
		return -1;
	else printf("Received: UA\n");

	
	
}

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


int sendUA(int fd){
	unsigned char UA[5] = {FLAG,A_SENDER,C_UA,0x01,FLAG};
	int res, i;
	res = 0;
	i = 0;	
	
	res = write(fd, UA, 5);
	return res;	
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

void generateError(char *frame){
	
	int r = (rand() % 10) + 1;

	if(r % 2)
		frame[9] = 0xAA;
	
}

void updateError(){
	gen_error = 1;
}


