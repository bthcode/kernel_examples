obj-m += hello.o
obj-m += sysfs_example.o
obj-m += kernel_thread.o
obj-m += mmapexample.o
obj-m += ioctl_example.o
obj-m += select_example.o

all: ioctl_app mmap_client 
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
	rm -f ioctl_app mmap_client

ioctl_app: ioctl_app.c
	$(CC) ioctl_app.c -o ioctl_app

mmap_client: mmap_client.c
	$(CC) mmap_client.c -o mmap_client


