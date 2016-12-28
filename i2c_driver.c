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


// Declaration of i2c_driver.c functions
int i2c_driver_init(void);
void i2c_driver_exit(void);
static int i2c_driver_open(struct inode *, struct file *);
static int i2c_driver_release(struct inode *, struct file *);
static ssize_t i2c_driver_read(struct file *, char *buf, size_t, loff_t *);
static ssize_t i2c_driver_write(struct file *, const char *buf, ssize_t, loff_t *);

// Structure that declares the usual file access functions
struct file_operations i2c_driver_fops = {

	open : i2c_driver_open,
	release : i2c_driver_release,
	read : i2c_driver_read,
	write : i2c_driver_write

};

module_init(i2c_driver_init);
module_exit(i2c_driver_exit);

//Major number
int i2c_driver_major;

//Buffer to store data
#define BUFF_LEN 80
static char i2c_driver_buffer[BUFF_LEN];


int i2c_driver_init(void) {


	result = register_chrdev(0, "i2c_driver", &i2c_driver_fops);
	if(result < 0){

		printk(KERN_ALERT "i2c_driver cannot obtain major number %d\n", i2c_driver_major);
		return result;	

	}

	i2c_driver_major = result;
	printk(KERN_ALERT "i2c_driver major number is %d\n", i2c_driver_major);
	printk(KERN_INFO "Inserting i2c_driver module...\n");


	return 0;

}

void i2c_driver_exit(void){

	// Free major number
	unregister_chrdev(i2c_driver_major, "i2c_driver");
	
	printk(KERN_INFO "Removing i2c_driver module\n");

}

static int i2c_driver_open(struct inode *inode, struct file *flip){

	return 0;

}

static int i2c_driver_release(struct inode *inode, struct file *flip){

	return 0;	

}

static ssize_t i2c_driver_read(struct file *flip, char *buf, size_t len, loff_t *f_pos) {

	return 0;

}

static ssize_t i2c_driver_write(struct file *flip, const char *buf, ssize_t len, loff_t *f_pos){

	return 0;

}
