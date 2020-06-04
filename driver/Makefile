obj-m += hid-sharkoon-light2-200.o

all:
	$(MAKE) -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	$(MAKE) -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean

install:
	@cp -v hid-sharkoon-light2-200.ko /lib/modules/$(shell uname -r)/kernel/drivers/hid
	@chown -v root:root /lib/modules/$(shell uname -r)/kernel/drivers/hid/hid-sharkoon-light2-200.ko

uninstall:
	@rm -v /lib/modules/$(shell uname -r)/kernel/drivers/hid/hid-sharkoon-light2-200.ko

insmod:
	@rmmod hid-sharkoon-light2-200 || true
	@insmod hid-sharkoon-light2-200.ko

rmmod:
	@rmmod hid-sharkoon-light2-200
