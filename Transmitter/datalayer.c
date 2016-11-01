#include "datalayer.h"

int write_control(int fd, char *data, char *filename, int control_field) {
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

	return pos;
}



int write_data(int number, char *data, char *buff) {
	int i;
	int pos = 0;
	int n_bytes = strlen(buff);
	data[pos++] = DATA;
	data[pos++] = number;
	data[pos++] = n_bytes / 256;
	data[pos++] = n_bytes % 256;
	for (i = 0; i < n_bytes; i++) {
		data[pos++] = buff[i];
	}

	/*
	for (i = 0; i < pos; i++) {
		if (i > 3) {
			printf("%c", data[i]);
		}
		else {
			printf("%x ", data[i]);
		}
	}
	printf("\n");
	*/

	return pos;
}


















