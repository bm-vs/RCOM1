#include "datalayer.h"

int write_control(int fd, char *filename, int control_field) {
	int i;

	struct stat fileStat;	
	if (fstat(fd, &fileStat) == -1) {
		perror("fstat");
		return -1;
	}

	// File Size
	int file_size = fileStat.st_size;
	unsigned char fsize[256];
	sprintf(fsize, "%x", file_size);
	int l1 = strlen(fsize);

	for (i = 0; i < l1; i++) {
		if (fsize[i] >= 48 && fsize[i] <= 57) {
			fsize[i] = fsize[i] - 48;
		}
		else if (fsize[i] >= 97 && fsize[i] <= 122) {
			fsize[i] = fsize[i] - 87;
		}
	}

	// File Name
	char l2 = strlen(filename);

	
	int pos = 0;
	// Create Control Data
	unsigned char data[256];
	data[pos++] = control_field;
	data[pos++] = FILE_SIZE_ID;
	data[pos++] = l1;
	for (i = 0; i < l1; i++) {
		data[pos++] = fsize[i];
	}
	data[pos++] = FILE_NAME_ID;
	data[pos++] = l2;
	for (i = 0; i < l2; i++) {
		data[pos++] = filename[i];
	}

/*
	for (i = 0; i < pos; i++) {
		printf("%x ", data[i]);
	}
	
	printf("\n");
*/

}
