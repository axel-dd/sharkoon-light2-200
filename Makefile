obj-m += hid-sharkoon-light2-200.o

all:
	@make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	@make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean

install:
	@cp -v hid-sharkoon-light2-200.ko /lib/modules/$(shell uname -r)/kernel/drivers/hid
	@chown -v root:root /lib/modules/$(shell uname -r)/kernel/drivers/hid/hid-sharkoon-light2-200.ko
	@depmod
	@modprobe -v -r hid-sharkoon-light2-200
	@modprobe -v hid-sharkoon-light2-200

uninstall:
	@rm -v /lib/modules/$(shell uname -r)/kernel/drivers/hid/hid-sharkoon-light2-200.ko