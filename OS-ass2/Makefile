obj-m+=dev_ps.o
all:
	make -C /lib/modules/$(shell uname -r)/build/ M=$(PWD) modules
	$(CC) user.c -o user
clean:
	make -C /lib/modules/$(shell uname -r)/build/ M=$(PWD) clean
	rm user
