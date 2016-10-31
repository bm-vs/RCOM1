#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>

#define FILE_SIZE_ID 0x00
#define FILE_NAME_ID 0x01
#define DATA 0x01
#define START 0x02
#define END 0x03
#define N_BYTES_READ 30
#define CONTROL_SIZE 256
#define DATA_SIZE 256


void* write_control(int fd, char *filename, int control_field);
void* write_data(int number, char *buff);
