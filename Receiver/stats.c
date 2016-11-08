#include "stats.h"
int n_frames_repeated = 0;
int n_rej_sent = 0;
int n_frames_received = 0;

int printStats(){
	
	printf("\n\n======================\n");
	printf("Receiver stats\n\n");
	printf("Frames Received:%d\n", n_frames_received +  n_frames_repeated);
	printf("Repeated Frames Received:%d\n", n_frames_repeated);
	printf("File Frames:%d\n",  n_frames_received) ;
	printf("Rejects Sent:%d\n", n_rej_sent);

}

void incFramesReceived(){
	n_frames_received++;
}
void incFramesRepeated(){
	n_frames_repeated++;
}
void incRejectsSent(){
	n_rej_sent++;
}
