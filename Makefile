obj-m	:= auth_engine.o
auth_engine-objs := kernmod.o

KDIR	:= /lib/modules/$(shell uname -r)/build
PWD	:= $(shell pwd)

default:
	gcc -o dimicraft_auth dimicraft_checker.c
	$(MAKE) -C $(KDIR) SUBDIRS=$(PWD) modules

clean:
	rm -f *.ko
	rm -f *.mod.*
	rm -f .*.cmd
	rm -f *.o
	rm -f dimicraft_auth
