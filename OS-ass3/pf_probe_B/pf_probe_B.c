#include <linux/math64.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/kprobes.h>
#include <asm/div64.h>
#define size 10000
#define MAX_SYMBOL_LEN  64
unsigned long min_addr = 0;//unsigned long max
unsigned long max_addr = 0;
//static char symbol[MAX_SYMBOL_LEN] = "_do_fork";
static char symbol[MAX_SYMBOL_LEN] = "handle_mm_fault";
module_param_string(symbol, symbol, sizeof(symbol), 0644);
static int pid=1;
module_param(pid,int,S_IRUGO);
int writecount=0,reatcount=0;
/* For each probe you need to allocate a kprobe structure */
static struct kprobe kp = {
        .symbol_name    = symbol,
};
struct timespec {                                                                                     
   time_t   tv_sec;        /* seconds */                                                             
   long     tv_nsec;       /* nanoseconds */                                                         
};
typedef struct processes {
        unsigned long address[size];
        suseconds_t nsec[size];
        time_t sec[size];

}process_data;
process_data data;
/* kprobe pre_handler: called just before the probed instruction is executed */
static int handler_pre(struct kprobe *p, struct pt_regs *regs){
    if(pid==vma->vm_mm->owner->pid){
        struct vm_area_struct *vma = regs->di;
        unsigned long address = regs->si;
        if(min_addr == 0){
                min_addr = address;
        }
        if(max_addr ==0){
                max_addr = address;
        }
        if(address > max_addr){
            max_addr = address;
        }
        if(address < min_addr){
            min_addr = address;
        }
        unsigned int flags = regs->dx;
        printk("test ->pid = %d",pid);
        printk("fault->pid = %d\n", vma->vm_mm->owner->pid);
        struct timespec t;
        clock_gettime(CLOCK_REALTIME, &t);
        data.address[writecount]=address;
        data.sec[writecount]=t.tv_sec;
        data.nsec[writecount]=t.tv_nsec;
        writecount++;
        writecount=writecount%size;
    }
    else{
         pid= vma->vm_mm->owner->pid;
    }
        return 0;
}
static int __init kprobe_init(void)
{
        int ret;
        kp.pre_handler = handler_pre;
        ret = register_kprobe(&kp);
        if (ret < 0) {
                pr_err("register_kprobe failed, returned %d\n", ret);
                return ret;
        }
        pr_info("Planted kprobe at %p\n", kp.addr);
        return 0;
}

static void __exit kprobe_exit(void)
{
        unregister_kprobe(&kp);
        pr_info("kprobe at %p unregistered\n", kp.addr);
        int flag = 1;
        int i = 0;
        int page_number = 0;
        unsigned long interval = max_addr - min_addr;
        while(flag){
            if(data.sec[i]!=0){
                unsigned long offset = data.address[i]-min_addr;
                // printk("offset1%ld\n",offset);
                do_div(interval,30);
                page_number = do_div(offset, interval);
                page_number =(int) offset;
                printk(KERN_INFO "Vir addr = 0x%lX, page number = %d, Time=%ld.%ld \n",
                    data.address[i], 
                    page_number, 
                    data.sec[i], 
                    data.nsec[i]);
                i++;
            }
            else{
                flag = 0;
            }
        }
        
}
module_init(kprobe_init)
module_exit(kprobe_exit)
MODULE_LICENSE("GPL");