/**
 * SFWRENG 3SH3 - Assignment 1 
 * Fady Abousifein - 400506836
 * 
 * Linux Distribution: 
 * Distributor ID:	Ubuntu 
 * Description:	        Ubuntu 22.04.5 LTS 
 * Release:	        22.04 
 * Codename:	        jammy
 *
 * Kernel Version: 
 * 6.8.0-90-generic
 */

// Includes, defines, and global variables
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>
#include <linux/jiffies.h> 


#define BUFFER_SIZE 128
#define PROC_NAME "seconds"

static unsigned long start_jiffies;

// Function prototypes
static int proc_init(void); 
static void proc_exit(void); 
static ssize_t proc_read(struct file *file, char __user* usr_buf, size_t count, loff_t *pos);

static const struct proc_ops my_proc_ops = {
        .proc_read = proc_read,
};

// Function called when module is loaded
static int proc_init(void)
{
	// save load time
	start_jiffies = jiffies; 

        // creates the /proc/seconds entry
        proc_create(PROC_NAME, 0, NULL, &my_proc_ops);

	// Prints message viewable through dmesg
        printk(KERN_INFO "/proc/%s created\n", PROC_NAME);

	return 0;
}

// Function called when module is removed
static void proc_exit(void) {

        // removes the /proc/seconds entry
        remove_proc_entry(PROC_NAME, NULL);
	
	// prints message viewable through dmesg
        printk( KERN_INFO "/proc/%s removed\n", PROC_NAME);
}

// function called everytime proc/seconds is read i.e. cat in this assignment 
static ssize_t proc_read(struct file *file, char __user *usr_buf, size_t count, loff_t *pos)
{
        int returnValue = 0;
        char buffer[BUFFER_SIZE];
        static int completed = 0;

        if (completed) {
                completed = 0;
                return 0; // EOF
        }

        completed = 1;

	unsigned long time_elapsed = (jiffies - start_jiffies) / HZ; 
        returnValue = sprintf(buffer, "%lu\n", time_elapsed);

        // copies the contents of buffer to userspace usr_buf
        if(copy_to_user(usr_buf, buffer, returnValue)) 
		return -EFAULT;  // operation failed due to bad memory address

        return returnValue;
}


/* Macros for registering module entry and exit points. */
module_init( proc_init );
module_exit( proc_exit );

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Seconds Module");
MODULE_AUTHOR("Fady Abousifein");

