obj-m += mydev.o
KDIR = /lib/modules/6.1.77-v8+/build


all: 
	make ARCH=arm64 CROSS_COMPILE=aarch64-linux-gnu- -C $(KDIR) M=$(shell pwd)  modules
	/usr/bin/aarch64-linux-gnu-gcc -o writer writer.c	
	/usr/bin/aarch64-linux-gnu-gcc -o reader reader.c
clean:
	make ARCH=arm64 CROSS_COMPILE=aarch64-linux-gnu- -C $(KDIR) M=$(shell pwd)  clean
	rm  writer
