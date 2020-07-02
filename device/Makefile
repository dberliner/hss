ifneq ($(KERNELRELEASE),)
include Kbuild
else
KDIR ?= /lib/modules/`uname -r`/build
MY_CFLAGS += -g -I/home/dan/hsstest/include
ccflags-y += ${MY_CFLAGS}
EXTRA_CFLAGS += -I/home/dan/hsstest/include

default:
	$(MAKE) -C $(KDIR) M=$$PWD EXTRA_CFLAGS="${MY_CLFAGS}"

install:
	$(MAKE) -C $(KDIR) M=$$PWD modules_install
	depmod

clean:
	rm -rf *.o .depend .*.cmd *.ko *.mod.c \
	       modules.order  Module.symvers

.PHONY: default clean

endif
