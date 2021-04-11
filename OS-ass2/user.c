#include<stdio.h>
#include<fcntl.h>
#include<stdlib.h>
#include<sys/stat.h>
#include<sys/types.h>
#include<unistd.h>
#include<stdio.h>
#include<stdlib.h>
#include<errno.h>
#include<string.h>
enum state
{
   TASK_RUNNING=0,
   TASK_INTERRUPTIBLE=1,
   TASK_UNINTERRUPTIBLE=2,
   __TASK_STOPPED=4,
   __TASK_TRACED=8,
   EXIT_DEAD = 16,
   EXIT_ZOMBIE = 32,
   EXIT_TRACE = 16|32,
   TASK_DEAD = 64,
   TASK_WAKEKILL = 128,
   TASK_WAKING = 256,
   TASK_PARKED = 512,
   TASK_NOLOAD = 1024,
   TASK_NEW = 2048,
   TASK_STATE_MAX = 4096,
   TASK_KILLABLE = 2|128,
   TASK_STOPPED = 4|128,
   TASK_TRACED = 8|128,
   TASK_IDLE = 2|1024,
   TASK_NORMAL = 1|2,
   TASK_ALL = 1|2|4|8,
   TASK_REPORT = 0|1|2|4|8|16|32
}status;

char * deform_state(enum state old)
{
   char *new;// For storing string after deforming
   switch(old)
    {
        case TASK_RUNNING:   new="TASK_RUNNING"; break;
        case TASK_INTERRUPTIBLE: new= "TASK_INTERRUPTIBLE"; break;
        case TASK_UNINTERRUPTIBLE: new="TASK_UNINTERRUPTIBLE"; break;
        case __TASK_STOPPED: new="TASK_STOPPED"; break;
        case __TASK_TRACED: new="TASK_TRACED"; break;
        case EXIT_DEAD: new="EXIT_DEAD"; break;
        case EXIT_ZOMBIE: new="EXIT_ZOMBIE"; break;
        case EXIT_TRACE: new="EXIT_ZOMBIE | EXIT_DEAD"; break;
        case  TASK_DEAD: new="TASK_DEAD"; break;
        case TASK_WAKEKILL: new="TASK_WAKEKILL"; break;
        case TASK_WAKING: new="TASK_WAKING"; break;
        case TASK_PARKED: new="TASK_PARKED"; break;
        case TASK_NOLOAD: new="TASK_NOLOAD"; break;
        case TASK_NEW: new="TASK_NEW"; break;
        case TASK_STATE_MAX: new="TASK_STATE_MAX";break;
        case TASK_KILLABLE: new="TASK_WAKEKILL | TASK_UNINTERRUPTIBLE";break;
        case TASK_STOPPED: new="TASK_WAKEKILL | __TASK_STOPPED";break;
        case TASK_TRACED: new="TASK_WAKEKILL | __TASK_TRACED";break;
        case TASK_IDLE: new="TASK_UNINTERRUPTIBLE | TASK_NOLOAD";break;
        case TASK_NORMAL: new="TASK_INTERRUPTIBLE | TASK_UNINTERRUPTIBLE";break;
        case TASK_ALL: new="TASK_NORMAL | __TASK_STOPPED | __TASK_TRACED";break;
        case TASK_REPORT: new="TASK_RUNNING | TASK_INTERRUPTIBLE | TASK_UNINTERRUPTIBLE | __TASK_STOPPED | __TASK_TRACED | EXIT_ZOMBIE | EXIT_DEAD";break;
    }
   return new;
}
typedef struct process {
    pid_t pid;
    pid_t ppid;
    int cpu;
    long state;
    int process_count;//count of process
}P;
int main()
{
    // Judge if can open the device:
    // And create fd for read operation:
    int fd = open("/dev/process_list", O_RDONLY);
    if (fd < 0){
      perror("Open Device Failed!\n");
      return errno;
    }
    int ifFirst = 1; // if First to run the cycle
    int count = 1; // For Ternimation of the cycle
    while(count){
        P *p = malloc(sizeof(P));// allocate space for struct P
        if(read(fd, p, sizeof(P))<0){// Read data from character device   
           printf("Read Failed!\n");
           exit(1);
        }
        if(ifFirst == 1){
           // init count as process_count
           count = p->process_count;
           ifFirst = 0;// let flag for first run be 0;
        }
        printf("PID=%d PPID=%d CPU=%d STATE=%s\n",
             p->pid,
             p->ppid,
             p->cpu,
             deform_state(p->state));
        free(p);
        count--;
    }
    close(fd);
	return 0;	
}
