#include <linux/init.h>// __init __exit
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>// file system
#include <linux/miscdevice.h>
#include <linux/sched.h> // task structure
#include <linux/sched/signal.h>
#include <linux/uaccess.h>// copy to user function
#include <linux/slab.h>
#define DEVICE_NAME "process_list"
MODULE_LICENSE("DUAL BSD/GPL");
static int __init my_module_init(void);
static int my_open(struct inode *, struct file *);
static int my_close(struct inode *, struct file *);
static ssize_t my_read(struct file *, char *, size_t, loff_t *);
static int Device_Open = 0; //Indicates whether device is open or not
typedef struct process {
	pid_t pid;
	pid_t ppid;
	int cpu;
	long state;
	int  process_count;
}P;
P *p; 
int call_count = 0;
int total_process = 0;
// Step 1: Declare a file operations structure
// should write this step first, bc my_fops should be define first then 
// be used in device struct
static struct file_operations my_fops =
{
        .owner = THIS_MODULE,
        .read = my_read,
        .open = my_open,
        .release = my_close,
};
// Step 2: Declare a device struct
static struct miscdevice my_misc_device =
{
        .minor = MISC_DYNAMIC_MINOR,
        .name = "process_list",
        .fops = &my_fops,
        .mode = S_IRUSR | S_IWUSR | S_IXUSR | S_IROTH
};

// Step 3: Register the device in module_init function
// Registration creates an entry in /dev for "mydevice" • 
// and connects file operations to my_fops
// Runs when kernel module is loaded (when 'insmod' command is executed)
static int __init my_module_init(void)
{
	// register device
	int error = misc_register(&my_misc_device);
	if(error){
		printk("\nfailed: %d", error);
		return error;
	}
	printk(KERN_ALERT "Character device successfully registered!");	
	return 0;	
}
// Step 5: Runs when kernel module is removed (when 'rmmod' command is executed)
static void __exit my_module_exit(void)
{
	misc_deregister(&my_misc_device);//Deregisters character device
	printk(KERN_ALERT "Character device de-registered!");
}
//Runs when a process opens the /dev/mydevice file
static int my_open(struct inode *inode, struct file *file)
{
	if(Device_Open)
		return -EBUSY;
	Device_Open++;
	   printk(KERN_INFO "opened : %d\n", Device_Open);
	return 0;
}

/// Step 4: Implement the fops functions
//Runs when a process closes the /dev/mydevice file
static int my_close(struct inode *inode, struct file *file)
{
	call_count = 0;
	// good habit to free pointers and make it NULL
	kfree(p);
	p = NULL; 
	if(Device_Open>0)
	{
	    Device_Open--;//count when file is closed
	}
	return 0;
}
void my_printk(struct task_struct *ts1)
{
	printk(KERN_INFO "************************************");
    printk(KERN_INFO "             NEW TEST");
    printk(KERN_INFO "************************************");
	for_each_process(ts1)
	{
		printk(KERN_INFO "kernel: PID=%d ", ts1->pid);
		printk(KERN_INFO "kernel: PPID=%d ", ts1->real_parent->pid);
		printk(KERN_INFO "kernel: CPU=%d ", task_cpu(ts1));
	    printk(KERN_INFO "kernel: STATE=%s ", ts1->state);
	}
}
//Runs when a process tries to read from /dev/mydevice file
static ssize_t my_read(struct file *file,
                       char *buffer, 
                       size_t length, 
                       loff_t *offset)
{   // do twice for each process
    // (1) record the number of the processes
    // (2) copy the p_data into p
	struct task_struct *ts1;
	if(call_count==0){
	    for_each_process(ts1)
	    {
	        total_process++;// record number of processes
	        my_printk(ts1); 
	    }
	}
	p = kmalloc(sizeof(P) * total_process,GFP_KERNEL);
    int i = 0;// For cycle
    for_each_process(ts1) 
	{
		p[i].pid = ts1->pid;
		p[i].ppid = ts1->parent->pid;
		p[i].cpu = task_cpu(ts1);
		p[i].state = ts1->state;
		p[i].process_count = total_process;
		i++;
	}
	/* function to copy kernel space buffer to user space*/
	// copy_to_user:( * to, *from, size) and returns 0 on success
    if (!copy_to_user(buffer, &p[call_count], sizeof(P))){
        printk(KERN_INFO "Copy_to_user Succeed");
        return sizeof(P);
    }else {
        printk(KERN_INFO "Copy_to_user Failed");
        return -EFAULT;
    }
    call_count++;
    // Check if finished reading all of the process.
    if(call_count == total_process){
       call_count = 0;
       kfree(p);
       p=NULL;
       return sizeof(P);
    }
}

module_init(my_module_init)
module_exit(my_module_exit)
