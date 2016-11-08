#include "datalayer.h"
	
int size = 0;
static char filename[256];

int write_file(int *file, char* buffer, int length){
	int res,k,n = 1,i;	
	
	switch(buffer[0]){
	case 1:
		 k = 256*buffer[2]+buffer[3];
		//printf("%d\n", k);

		for(i = 0;i < k;i++ ){
		  	//printf("%x ", buffer[i+4]);
			write(*file, &buffer[i+4], 1);
		}
			//printf("\n");
		return 1;
		
	case 2:
		if (buffer[n] == 0){
		  n++;
	
		  for (i = 1; i <= buffer[n]; i++) {
		  	size = (size*16+buffer[n+i]);
		  }

		  n += buffer[n];
		n++;
		  if (buffer[n] == 1){
		    n++;
		    int filenameSize = buffer[n];
		    n++;
		    memcpy(&filename, &buffer[n], filenameSize);
		    *file = open(filename, O_WRONLY | O_APPEND | O_CREAT | O_TRUNC, 0666);
		  }
		}
		return 2;
	case 3:
		close(*file);
		return 3;	
	}
	
}

int getCPsize(){
	return size;
}

char* getFileName(){
	return filename;
}
