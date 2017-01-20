#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>

#define BUF_LEN 80

int main()
{
    int file_desc;
    unsigned int slave_addr;
	unsigned char buff[BUF_LEN];

	/* Initializig variables */	
	memset(buff, '\0', BUF_LEN);

	/* Open dummy file. */
    file_desc = open("/dev/i2c_dummy", O_RDWR);

    if(file_desc < 0)
    {
       	printf("Error, 'i2c_dummy' not opened\n");
       	return -1;
    }

	buff[0] = 'A';
	buff[1] = (unsigned char)slave_addr;

	fpritnf("%s", buff);	
    
	close(file_desc);
    return 0;
}
