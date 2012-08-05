# See Documentation/kbuild/modules.txt

ifneq ($(KERNELRELEASE),)
# kbuild part of makefile
obj-m += introspect.o
obj-m += enable_p2.o

else
# normal part of makefile

all:
	make ARCH=arm CROSS_COMPILE=/opt/freescale/usr/local/gcc-4.1.2-glibc-2.5-nptl-3/arm-none-linux-gnueabi/bin/arm-none-linux-gnueabi- O=build -C /home/ltib/git/linux-2.6.39.2-lpc M=${PWD} modules

clean:
	make ARCH=arm CROSS_COMPILE=/opt/freescale/usr/local/gcc-4.1.2-glibc-2.5-nptl-3/arm-none-linux-gnueabi/bin/arm-none-linux-gnueabi- O=build -C /home/ltib/git/linux-2.6.39.2-lpc M=${PWD} clean

endif
