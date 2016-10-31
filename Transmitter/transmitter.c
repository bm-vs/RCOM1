#include <stdio.h>
#include <libgen.h>
#include "datalayer.h"
#include "linklayer.h"

int main(int argc, char *argv[]) {
	// Check arguments
	if (argc != 3) {
		printf("Usage:\n");
		printf("  transmitter <file_name> <n_serial_port>\n");
		printf("  Ex.: transmitter penguin.gif /dev/ttyS1\n");
		return 1;
	}

	if ((strcmp("/dev/ttyS0", argv[2])!=0) && (strcmp("/dev/ttyS1", argv[2])!=0)) {
		printf("n_serial_port as to be: /dev/ttyS0 or /dev/ttyS1\n");
		return 2;
	}


	// Open file
	int fd = open(argv[1], O_RDONLY);
	if (fd == -1) {
		perror("File opening failed: ");
		return 3;
	}

	char *filename = basename(argv[1]);

	// Open serial port
	/*
	struct termios oldtio;
	int serial = llopen(argv[2], &oldtio);
	if (serial == -1) {
		printf("Connection timeout\n");
		return 4;
	}
	*/

	// Write opening control packet
	
	char *data;/*
	data = write_control(fd, filename, START);
	if (data == NULL) {
		return 5;
	}
	*/
	// llwrite(data);

	// Write data packets
	char buff[N_BYTES_READ];	
	int number = 0;
	while (read(fd, buff, N_BYTES_READ) != 0) {
		data = write_data(number, buff);
		//llwrite(data)
		memset(buff, 0, sizeof buff);
		number = (number+1) % 256;
	}

	// Write closing control packet
	/*
	data = write_control(fd, filename, END);
	if (data == NULL) {
		return 5;
	}
	*/
	// llwrite(data);

	// Close serial port
	/*
	llclose(serial, &oldtio);
	*/
}
