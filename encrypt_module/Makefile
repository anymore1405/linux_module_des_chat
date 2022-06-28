CC = gcc

obj-m := des_module.o

KDIR := /lib/modules/`uname -r`/build

all:
	make -C $(KDIR) M=`pwd` modules

test: test.c
	${CC} -o test test.c

clean:
	make -C $(KDIR) M=`pwd` clean

install:
	sudo dmesg -C
	sudo insmod des_module.ko

remove:
	sudo rmmod des_module