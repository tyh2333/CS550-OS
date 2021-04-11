#include <linux/math64.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/kprobes.h>
#include <asm/div64.h>
#define size 10000
#define MAX_SYMBOL_LEN  64
unsigned long min_addr = 0;
unsigned long max_addr = 0;
static char symbol[MAX_SYMBOL_LEN] = "handle_mm_fault";
module_param_string(symbol, symbol, sizeof(symbol), 0644);
static int pid=1;
module_param(pid,int,S_IRUGO); 
int writecount=0,reatcount=0;
/* For each probe you need to allocate a kprobe structure */
static struct kprobe kp = {
        .symbol_name    = symbol,
};
/* kprobe pre_handler: called just before the probed instruction is executed */
static int handler_pre(struct kprobe *p, struct pt_regs *regs)
{
    struct vm_area_struct *vma = regs->di;
    unsigned long address = regs->si; 
    unsigned int flags = regs->dx;
    printk("test ->pid = %d",pid);
    // printk("fault->pid = %d\n", vma->vm_mm->owner->pid);
    /// judge if the pid caused page fault is the current-test pid
    if(pid==vma->vm_mm->owner->pid){
        pr_info("<%s> pre_handler: fault pid = %d, fault->addr = 0x%p, fault->flags = 0x%lx\n",
                p->symbol_name, 
                vma->vm_mm->owner->pid, 
                address, 
                flags);
    }else{
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
}

module_init(kprobe_init)
module_exit(kprobe_exit)
MODULE_LICENSE("GPL");