#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>

#define BUF_LEN 80

int file_desc;
unsigned char buff[BUF_LEN];

void* print_state(void* param){

	unsigned char temp;
	unsigned char mask;

	while(1){
	 	
		file_desc = open("/dev/i2c_dummy", O_RDWR);

		if(file_desc < 0)
		{
			printf("Error, 'i2c_dummy' not opened\n");
			return 0;
		}

		system("clear");	
		read(file_desc, buff, BUF_LEN);

		printf("Joystick X: %d\n", (short int)buff[0]);
		printf("Joystick Y: %d\n", (short int)buff[1]);

		temp = buff[2];			
		temp = temp << 2;
		
		mask = buff[5];
		mask &= 0x0C;
		mask = mask >> 2;

		temp = temp ^ mask;

		printf("Accelometer X: %d\n", (short int)temp);

		temp = buff[2];			
		temp = temp << 2;
		
		mask = buff[5];
		mask &= 0x30;
		mask = mask >> 2;

		temp = temp ^ mask;

		printf("Accelometer Y: %d\n", (short int)temp);

		temp = buff[2];			
		temp = temp << 2;
		
		mask = buff[5];
		mask &= 0xC0;
		mask = mask >> 2;

		temp = temp ^ mask;

		printf("Accelometer Z: %d\n", (short int)temp);

		temp = buff[5];
		temp &= 0x01;

		printf("Z Button: %d\n", (short int)temp);

		temp = buff[5];
		temp &= 0x02;
		temp = temp >> 1;

		printf("C Button: %d\n", (short int)temp);

		printf("To Exit from this program, simply type in 'q' and press Enter\n");

		close(file_desc);

	}

	return 0;

}

int main()
{
	unsigned int slave_addr;
	unsigned char c; 	
   
	pthread_t h_print_state;

	/* Initializig variables */	
	memset(buff, '\0', BUF_LEN);

	/* Open dummy file. */
    file_desc = open("/dev/i2c_dummy", O_RDWR);

    if(file_desc < 0)
    {
       	printf("Error, 'i2c_dummy' not opened\n");
       	return -1;
    }

	printf("Enter Slave Address: (hexadecimal value)\n");
	scanf("%x", &slave_addr);
	printf("Slave Address You Entered: %X\n", slave_addr);

	buff[0] = 'A';
	buff[1] = (unsigned char)slave_addr;

	write(file_desc, buff, BUF_LEN);

	close(file_desc);

	pthread_create(&h_print_state, NULL, print_state, 0);
	pthread_detach(h_print_state);
 
	while(1){
			
		scanf("%c", &c);
		if(c == 'q')
			break;
	}
    
   return 0;
}
