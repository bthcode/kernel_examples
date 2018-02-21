obj-m += hello.o
obj-m += sysfs_example.o
obj-m += kernel_thread.o
obj-m += mmapexample.o
obj-m += query_ioctl.o
obj-m += timer_example.o

all: query_app mmap_client 
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
	rm -f query_app mmap_client

query_app: query_app.c
	$(CC) query_app.c -o query_app

mmap_client: mmap_client.c
	$(CC) mmap_client.c -o mmap_client


