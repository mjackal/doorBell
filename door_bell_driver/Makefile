#!/bin/bash
#编译.c文件成中间文件.o
obj-m += bell_driver.o

#内核源码目录
KDIR := /home/mjackal/kernel/iTop4412_Kernel_3.0

#当前目录路径
PWD ?= $(shell pwd)

#make -C就是指调用执行路径
#modules要执行的操作
all:
	make -C $(KDIR) M=$(PWD) modules

#make clean执行的操作是删除后缀为.o的文件
clean:
	rm -rf *.o
