#include <linux/init.h> // for module init/exit macros
#include <linux/module.h> // for all modules 
#include <linux/kernel.h> // for printk
#include <linux/sched.h> // for struct task_struct
#include <linux/sched/signal.h> // for task_tgid_nr

void print_init_PCB(void); 

/* This function is called when the module is loaded. */
static int simple_init(void)
{
       printk(KERN_INFO "Loading Module\n");
       print_init_PCB();  
       return 0;
}

/* This function is called when the module is removed. */
static void simple_exit(void) {
	printk(KERN_INFO "Removing Module\n");
}

// Function prints the task_struct information for the init task
void print_init_PCB(void) {
	struct task_struct* p = &init_task; 
	printk(KERN_INFO "pid: %d\nstate: %d\nflags: %u\nrt_priority: %u\npolicy: %u\ntgid: %u\n", p->pid, 0, p->flags, p->rt_priority, p->policy, task_tgid_nr(p));  
}

/* Macros for registering module entry and exit points. */
module_init( simple_init );
module_exit( simple_exit );

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Simple");
MODULE_AUTHOR("Fady Abousifein");


