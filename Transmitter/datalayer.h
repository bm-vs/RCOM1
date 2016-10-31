#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>

#define FILE_SIZE_ID 0x00
#define FILE_NAME_ID 0x01
#define START 0x02
#define END 0x03

int write_control(int fd, char *filename, int control_field);
