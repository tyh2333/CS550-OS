obj-m +=pf_probe_B.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules -w

clean:
	rm -rf *.o *~ core .depend .*.cmd *.ko *.mod.c .tmp_versions *.order *.symvers *.out
