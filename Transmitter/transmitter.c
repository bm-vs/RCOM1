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
	printf("File: %s opened\n", argv[1]);
	char *filename = basename(argv[1]);


	// Open serial port
	struct termios oldtio;
	int serial = llopen(argv[2], &oldtio);
	if (serial == -1) {
		printf("Connection timeout\n");
		return 4;
	}
	printf("Serial Port: %s opened\n", argv[2]);


	// Write opening control packet
	char *data = malloc(DATA_SIZE);
	int size;
	size = write_control(fd, data, filename, START);
	if (size == -1) {
		return 5;
	}
	if (llwrite(serial, data, size) == -1) {
		printf("Connection timeout\n");
		return 4;
	}
	memset(data, 0, sizeof data);


	// Write data packets
	char buff[N_BYTES_READ];	
	int number = 0;
	while (1) {
		int r = read(fd, buff, N_BYTES_READ);
		if (r == 0) {
			break;
		}

		//printf("%d\n", r);

		size = write_data(number, data, buff, r);
		if (llwrite(serial, data, size) == -1) {
			printf("Connection timeout\n");
			return 4;
		}
		memset(buff, 0, sizeof buff);
		number = (number+1) % 256;
	}

	// Write closing control packet
	size = write_control(fd, data, filename, END);
	if (size == -1) {
		return 5;
	}
	
	if (llwrite(serial, data, size) == -1) {
		printf("Connection timeout\n");
		return 4;
	}
	memset(data, 0, sizeof data);


	// Close serial port
	llclose(serial, &oldtio);
}
