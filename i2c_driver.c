#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/types.h>
#include <linux/fcntl.h>
#include <linux/proc_fs.h>
#include <linux/string.h>
#include <linux/ioport.h>
#include <linux/ktime.h>
#include <linux/hrtimer.h>
#include <asm/io.h>
#include <asm/uaccess.h>

MODULE_LICENSE("Dual BSD/GPL");

/* GPIO base address */
#define GPIO_BASE_ADDR (0x3F200000)

/* Controller register base address */
#define BSC1_BASE_ADDR (0x3F804000)
 
/* BSC1 registers i hit */
#define BSC1_REG_C (BSC1_BASE_ADDR + 0x00000000)
#define BSC1_REG_S (BSC1_BASE_ADDR + 0x00000004)
#define BSC1_REG_DLEN (BSC1_BASE_ADDR + 0x00000008)
#define BSC1_REG_SLAVE_ADDR (BSC1_BASE_ADDR + 0x0000000C)
#define BSC1_REG_FIFO (BSC1_BASE_ADDR + 0x00000010)
#define BSC1_REG_DIV (BSC1_BASE_ADDR + 0x00000014)
#define BSC1_REG_DEL (BSC1_BASE_ADDR + 0x00000018)
#define BSC1_REG_CLKT (BSC1_BASE_ADDR + 0x0000001C)

//Handle GPIO: 0-9
/* GPIO Function Select 0. */
#define GPFSEL0_BASE_ADDR (GPIO_BASE_ADDR + 0x00000000)

//Handle GPIO: 10-19
/* GPIO Function Select 1. */
#define GPFSEL1_BASE_ADDR (GPIO_BASE_ADDR + 0x00000004)

//Handle GPIO: 20-29
/* GPIO Function Select 2. */
#define GPFSEL2_BASE_ADDR (GPIO_BASE_ADDR + 0x00000008)

//Handle GPIO: 30-39
/* GPIO Function Select 3. */
#define GPFSEL3_BASE_ADDR (GPIO_BASE_ADDR + 0x0000000C)

//Handle GPIO: 40-49
/* GPIO Function Select 4. */
#define GPFSEL4_BASE_ADDR (GPIO_BASE_ADDR + 0x00000010)

//Handle GPIO: 50-53
/* GPIO Function Select 5. */
#define GPFSEL5_BASE_ADDR (GPIO_BASE_ADDR + 0x00000014)

//--
//GPIO: 0-31
/* GPIO Pin Output Set 0. */
#define GPSET0_BASE_ADDR (GPIO_BASE_ADDR + 0x0000001C)

//GPIO: 32-53
/* GPIO Pin Output Set 1. */
#define GPSET1_BASE_ADDR (GPIO_BASE_ADDR + 0x00000020)

//--
//GPIO: 0-31
/* GPIO Pin Output Clear 0. */
#define GPCLR0_BASE_ADDR (GPIO_BASE_ADDR + 0x00000028)

//GPIO: 32-53
/* GPIO Pin Output Clear 1. */
#define GPCLR1_BASE_ADDR (GPIO_BASE_ADDR + 0x0000002C)

//--
//GPIO: 0-31
/* GPIO Pin Level 0. */
#define GPLEV0_BASE_ADDR (GPIO_BASE_ADDR + 0x00000034)

//GPIO: 32-53
/* GPIO Pin Level 1. */
#define GPLEV1_BASE_ADDR (GPIO_BASE_ADDR + 0x00000038)

//--
//GPIO: 0-53
/* GPIO Pin Pull-up/down Enable. */
#define GPPUD_BASE_ADDR (GPIO_BASE_ADDR + 0x00000094)

//GPIO: 0-31
/* GPIO Pull-up/down Clock Register 0. */
#define GPPUDCLK0_BASE_ADDR (GPIO_BASE_ADDR + 0x00000098)

//GPIO: 32-53
/* GPIO Pull-up/down Clock Register 1. */
#define GPPUDCLK1_BASE_ADDR (GPIO_BASE_ADDR + 0x0000009C)

//Using gpio as alternate 0
#define GPIO_DIRECTION_ALT0 (4)

#define PULL_NONE (0)
#define PULL_DOWN (1)
#define PULL_UP (2)

//Needed GPIO
#define GPIO_02 (2)
#define GPIO_03 (3)

#define START_TRANSFER 0x00008080
#define CLEAR_STATUS 0x00000302
#define SETUP_CTRL 0x00008100

// Declaration of i2c_driver.c functions
int i2c_driver_init(void);
void i2c_driver_exit(void);
static int i2c_driver_open(struct inode *, struct file *);
static int i2c_driver_release(struct inode *, struct file *);
static ssize_t i2c_driver_read(struct file *, char *buf, size_t , loff_t *);
static ssize_t i2c_driver_write(struct file *, const char *buf, size_t, loff_t *);

/* Buffer to store data */
#define BUFF_LEN 80
static char i2c_driver_buffer[BUFF_LEN];

/* Structure that declares the usual file access functions. */
struct file_operations i2c_driver_fops =
{
    open    :   i2c_driver_open,
    release :   i2c_driver_release,
    read    :   i2c_driver_read,
    write   :   i2c_driver_write
};

module_init(i2c_driver_init);
module_exit(i2c_driver_exit);

/* Major number */
int i2c_driver_major;

/* Register addresses */
volatile void *reg_c = NULL;
volatile void *reg_dlen = NULL;
volatile void *reg_slave_addr = NULL;
volatile void *reg_fifo = NULL;
volatile void *reg_s = NULL;

unsigned int GetGPFSELReg(char pin){
    	unsigned int addr;
     if(pin >= 0 && pin <10)
        addr = GPFSEL0_BASE_ADDR;
    else if(pin >= 10 && pin <20)
        addr = GPFSEL1_BASE_ADDR;
    else if(pin >= 20 && pin <30)
        addr = GPFSEL2_BASE_ADDR;
    else if(pin >= 30 && pin <40)
        addr = GPFSEL3_BASE_ADDR;
    else if(pin >= 40 && pin <50)
        addr = GPFSEL4_BASE_ADDR;
    else /*if(pin >= 50 && pin <53) */
        addr = GPFSEL5_BASE_ADDR;
  
  return addr;
}

char GetGPIOPinOffset(char pin){
    
	if(pin >= 0 && pin <10)
        pin = pin;
    else if(pin >= 10 && pin <20)
        pin -= 10;
    else if(pin >= 20 && pin <30)
        pin -= 20;
    else if(pin >= 30 && pin <40)
        pin -= 30;
    else if(pin >= 40 && pin <50)
        pin -= 40;
    else /*if(pin >= 50 && pin <53) */
        pin -= 50;

    return pin;
}

void SetInternalPullUpDown(char pin, char value){
    
	unsigned int base_addr_gppud; 
    unsigned int base_addr_gppudclk; 
    void *addr = NULL;
    unsigned int tmp;
    unsigned int mask;
    
    /* Get base address of GPIO Pull-up/down Register (GPPUD). */
    base_addr_gppud = GPPUD_BASE_ADDR;
    
    /* Get base address of GPIO Pull-up/down Clock Register (GPPUDCLK). */
    base_addr_gppudclk = (pin < 32) ? GPPUDCLK0_BASE_ADDR : GPPUDCLK1_BASE_ADDR;

    /* Get pin offset in register . */
    pin = (pin < 32) ? pin : pin - 32;
    
    /* Write to GPPUD to set the required control signal (i.e. Pull-up or Pull-Down or neither
       to remove the current Pull-up/down). */
    addr = ioremap(base_addr_gppud, 4);
    iowrite32(value, addr);

    /* Wait 150 cycles ^  this provides the required set-up time for the control signal */
    
    /* Write to GPPUDCLK0/1 to clock the control signal into the GPIO pads you wish to
       modify ^  NOTE only the pads which receive a clock will be modified, all others will
       retain their previous state. */
    addr = ioremap(base_addr_gppudclk, 4);
    tmp = ioread32(addr);    
    mask = 0x1 << pin;
    tmp |= mask;        
    iowrite32(tmp, addr);

    /* Wait 150 cycles ^  this provides the required hold time for the control signal */
	
	/* Write to GPPUD to remove the control signal. */
    addr = ioremap(base_addr_gppud, 4);
    iowrite32(PULL_NONE, addr);

    /* Write to GPPUDCLK0/1 to remove the clock. */
    addr = ioremap(base_addr_gppudclk, 4);
    tmp = ioread32(addr);    
    mask = 0x1 << pin;
    tmp &= (~mask);        
    iowrite32(tmp, addr);
}

void SetGpioPinDirection(char pin, char direction){

    unsigned int base_addr; 
    void *addr = NULL;
    unsigned int tmp;
    unsigned int mask;
    
    /* Get base address of function selection register. */
    base_addr = GetGPFSELReg(pin);

    /* Calculate gpio pin offset. */
    pin = GetGPIOPinOffset(pin);    
    
    /* Set gpio pin direction. */
    addr = ioremap(base_addr, 4);
    tmp = ioread32(addr);

    mask = ~(0b111 << (pin*3));
    tmp &= mask;

    mask = (direction & 0b111) << (pin*3);
    tmp |= mask;

    iowrite32(tmp, addr);
}

void InitSlave(void) {

	unsigned int temp = 0;
	unsigned int temp_d = 0;

	
	/* Clear S reg */
	iowrite32(CLEAR_STATUS, reg_s);
	
	/* Setup C reg */
	iowrite32(SETUP_CTRL, reg_c);
	temp = ioread32(reg_c);

	/* Write to FIFO reg */
	iowrite32(0x00000042, reg_fifo);
	iowrite32(0x00000000, reg_fifo);

	/* Setup DLEN reg */
	iowrite32(0x00000002, reg_dlen);
	temp = ioread32(reg_dlen);

	/* Starting transfer */
	iowrite32(START_TRANSFER, reg_c);

	temp = ioread32(reg_dlen);
	printk(KERN_ALERT "DLEN: %u\n", temp);
	
	temp = ioread32(reg_s);
	temp &= 1;
	printk(KERN_ALERT "TA: %u\n", temp);

	temp = ioread32(reg_s);
	temp &= 0x00000200;
	printk(KERN_ALERT "CLKT: %u\n", temp); 

	temp = ioread32(reg_s);
	temp_d = temp;

	if(!(temp & (1<<8))) // If ERROR == 0
		printk(KERN_ALERT "No errors detected");
	else
		printk(KERN_ALERT "Errors Detected\n");

	if(temp_d & (1 << 1)) // If DONE == 1
		printk(KERN_ALERT "Handshake completed");
	else
		printk(KERN_ALERT "Handshake error, transfer incomplete\n");

}

int i2c_driver_init(void) {

	int result;	
	unsigned int temp;	

	result = register_chrdev(0, "i2c_driver", &i2c_driver_fops);
	if(result < 0){

		printk(KERN_ALERT "i2c_driver cannot obtain major number %d\n", i2c_driver_major);
		return result;	

	}

	i2c_driver_major = result;
	printk(KERN_INFO "Inserting i2c_driver module...\n");
	printk(KERN_ALERT "i2c_driver major number is %d\n", i2c_driver_major);

	/* Setting pull up, and pin alt functions */
	SetGpioPinDirection(GPIO_02, GPIO_DIRECTION_ALT0);
	SetGpioPinDirection(GPIO_03, GPIO_DIRECTION_ALT0);
	SetInternalPullUpDown(GPIO_02, PULL_UP);
	SetInternalPullUpDown(GPIO_03, PULL_DOWN);

	/* Remaping registers virtual addreses to physical */
	reg_c = ioremap(BSC1_REG_C, 4);
	reg_dlen = ioremap(BSC1_REG_DLEN, 4);
	reg_slave_addr = ioremap(BSC1_REG_SLAVE_ADDR, 4);
	reg_fifo = ioremap(BSC1_REG_FIFO, 4);
	reg_s = ioremap(BSC1_REG_S, 4);

	/* Set device address */
	iowrite32(0x00000052, reg_slave_addr);
	temp = ioread32(reg_slave_addr);
	printk(KERN_ALERT "Value of SLAVE_ADDR: %u\n", temp);

	InitSlave();

	return 0;

}

void i2c_driver_exit(void){

	/* Free major number */
	unregister_chrdev(i2c_driver_major, "i2c_driver");
	SetInternalPullUpDown(GPIO_02, PULL_NONE);
	SetInternalPullUpDown(GPIO_03, PULL_NONE);
	
	printk(KERN_INFO "Removing i2c_driver module\n");

}

static int i2c_driver_open(struct inode *inode, struct file *flip){

	return 0;

}

static int i2c_driver_release(struct inode *inode, struct file *flip){

	return 0;	

}


static ssize_t i2c_driver_read(struct file *filp, char *buf, size_t len, loff_t *f_pos) {

	int data_size = 0;
	unsigned int temp = 0;
	unsigned int temp_d = 0;

	/* Clear status register */
	iowrite32(CLEAR_STATUS, reg_s); 
	
	/* Ready C reg for write, clear fifo */
	iowrite32(SETUP_CTRL, reg_c);

	/* Fill the fifo reg */
	iowrite32(0x00000000, reg_fifo);

	/* DLEN = 1 */
	iowrite32(0x00000001, reg_dlen);

	/* Triger transfer */
	iowrite32(START_TRANSFER, reg_c);

	temp = ioread32(reg_s);
	temp_d = temp;

	if(!(temp & 1<<8))
		printk(KERN_INFO "No Read Request Errors Detected\n");
	else
		printk(KERN_INFO "Read Read Request Error occured\n");

	if(temp_d & 1<<1)
		printk(KERN_INFO "Read Request OK\n");
	else
		printk(KERN_INFO "Read Request is Denied\n");

	if(*f_pos == 0) {

		data_size = strlen(i2c_driver_buffer);

		if(copy_to_user(buf, i2c_driver_buffer, data_size) != 0) {

			return -EFAULT;
		
		} else {

			(*f_pos) += data_size;
			return data_size;	
	
		}
	
	} else {

		return 0;

	}

}

static ssize_t i2c_driver_write(struct file *filp, const char *buf, size_t len, loff_t *f_pos){


	
	if(copy_from_user(i2c_driver_buffer, buf, len) != 0) {

		return -EFAULT;

	} else {

		// Operations:
		// TODO:
			

		return len;

	}

} 


