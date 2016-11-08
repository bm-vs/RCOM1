#include "stats.h"

int n_frames_sent = 0;
int n_frames_resent = 0;
int n_timeout = 0;
int n_rej_received = 0;

void printStats() {
	printf("\n\n=============================\n");
	printf("Transmitter stats\n\n");
	printf("Total I frames of file: %d\n", n_frames_sent - n_frames_resent);
	printf("I Frames sent: %d\n", n_frames_sent);
	printf("I Frames resent: %d\n", n_frames_resent);
	printf("Number of timeouts: %d\n", n_timeout);
	printf("Number of REJ received: %d\n", n_rej_received);
}

void incNFramesSent() {
	n_frames_sent++;
}

void incNFramesResent() {
	n_frames_resent++;
}

void incNTimeouts() {
	n_timeout++;
}

void incNRejReceived() {
	n_rej_received++;
}
