#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/usb.h>
#include <linux/usb/input.h>
#include <linux/hid.h>

#include "hid-sharkoon-light2-200.h"




#define SHARKOON_LIGHT2_200_CMD_GET_REPORT 0x01;
#define SHARKOON_LIGHT2_200_CMD_SET_REPORT 0x02;


struct sharkoon_report_color {
    unsigned char red;
    unsigned char green;
    unsigned char blue;
};

struct sharkoon_report_dpi_level {
    unsigned char y_offset : 4;
    unsigned char x_offset : 4;
    unsigned char x_value;
    unsigned char y_value;
};

struct sharkoon_report_dpi_settings {
    /**
     * byte 9 
     * DPI steps 1-7  ON/OFF bit mask  1=on / 0=off
     *  0    1     1     1     1     1     1     1
     *  -   DPI7  DPI6  DPI5  DPI4  DPI3  DPI2  DPI1
     */
    unsigned char dpi1_enabled : 1;
    unsigned char dpi2_enabled : 1;
    unsigned char dpi3_enabled : 1;
    unsigned char dpi4_enabled : 1;
    unsigned char dpi5_enabled : 1;
    unsigned char dpi6_enabled : 1;
    unsigned char dpi7_enabled : 1;
    unsigned char unused : 1;

    /**
     * bytes 10-30 
     * dpi level for each step
     */
    struct sharkoon_report_dpi_level dpi1;
    struct sharkoon_report_dpi_level dpi2;
    struct sharkoon_report_dpi_level dpi3;
    struct sharkoon_report_dpi_level dpi4;
    struct sharkoon_report_dpi_level dpi5;
    struct sharkoon_report_dpi_level dpi6;
    struct sharkoon_report_dpi_level dpi7;
};

struct sharkoon_report_illumination_settings {
    /**
     * byte 38 LED effect type
     * value range : 0-9
     */
    unsigned char led_effect;

    /**
     * byte 39 LED frequency
     * value range : 0-2 (reversed order, the lower the faster)
     */
    unsigned char led_frequency;

    /**
     * byte 40 LED brightness
     * value range : 0-10 (the higher the brighter)
     */
    unsigned char led_brightness;

    /**
     * byte 41 UNKNOWN 
     * value: mostly 0x01
     */
    unsigned char unknownByte41;

    /**
     * byte 42 the profile for which the settings should be set/get
     * value range : 1-5
     */
    unsigned char profile_id;

    /**
     * bytes 43-63 color definitions 1-7
     */
    struct sharkoon_report_color color1;
    struct sharkoon_report_color color2;
    struct sharkoon_report_color color3;
    struct sharkoon_report_color color4;
    struct sharkoon_report_color color5;
    struct sharkoon_report_color color6;
    struct sharkoon_report_color color7;
};

// 64-bytes
struct sharkoon_report {
    /**
     * bytes 1-3
     * on set/get commands 0x04, 0xa0, 0x01
     * on DPI key press / DPI step change  0x04, 0xa2, 0x02
     */
    unsigned char header[3];

    /**
     * byte 4 command for get or set settings
     * SHARKOON_LIGHT2_200_CMD_GET_REPORT - get report with current mouse settings
     * SHARKOON_LIGHT2_200_CMD_SET_REPORT - submit report to change mouse settings
     * on DPI key press / DPI step change
     */
    unsigned char command;

    /**
     * bytes 5-7 UNKNOWN
     * 0x00, 0x00, 0x00 on SHARKOON_LIGHT2_200_CMD_GET_REPORT
     * 0x01, 0x02, 0xa5 on SHARKOON_LIGHT2_200_CMD_SET_REPORT
     */
    unsigned char unknownByte5;
    unsigned char unknownByte6;
    unsigned char unknownByte7;

    /**
     * byte 8
     * current DPI step number of the device 1-7
     * values: 0-6
     */
    unsigned char unknownByte8;

    /**
     * bytes 9-30 DPI setting
     */
    struct sharkoon_report_dpi_settings dpi_settings;

    /**
     * bytes 31-33 UNKNOWN
     */
    unsigned char unknownByte31;
    unsigned char unknownByte32;
    unsigned char unknownByte33;

    /**
     * byte 34 Lift-Off distance
     * UI -> value
     *  1 -> 2
     *  2 -> 3
     *  3 -> 4
     */
    unsigned char lod;

    /**
     * bytes 35-37 UNKNOWN
     * values vary
     */
    unsigned char unknownByte35;
    unsigned char unknownByte36;
    unsigned char unknownByte37;

    /**
     * bytes 38-64 DPI setting
     */
    struct sharkoon_report_illumination_settings illumination_settings;

    /**
     * byte 64 UNKNOWN
     * value: mostly 0
     */
    unsigned char unknownByte64;
};

/**
 * human readable DPI level values for x-axis and y-axis
 * values: between 50 and 16000 in steps of 50
 */
struct sharkoon_dpi_level_human_readable {
    unsigned short x;
    unsigned short y;
};

/**
 * creates DPI level report from human readable dpi values
 * \param x DPI value of y-axis in human readable units (value range between 50 and 16000 in steps of 50)
 * \param y DPI value of y-axis in human readable units (value range between 50 and 16000 in steps of 50)
 * \return struct sharkoon_report_dpi_level - for usage in sharkoon_report_dpi_settings
 */
struct sharkoon_report_dpi_level sharkoon_dpi_create_level_report_from_human_readable_values(unsigned short x, unsigned short y) 
{
    struct sharkoon_report_dpi_level dpi_report = { 0 };

    x /= 50;
    y /= 50;

    dpi_report.x_value  = x & 0xFF;
    dpi_report.y_value  = y & 0xFF;
    dpi_report.x_offset = (x >> 8) & 0xF;
    dpi_report.y_offset = (y >> 8) & 0xF;

    return dpi_report;
}

/**
 * get human readable dpi values from a DPI level report
 */
struct sharkoon_dpi_level_human_readable sharkoon_dpi_get_human_readable_values_from_level_report(struct sharkoon_report_dpi_level *dpi_report) 
{
    struct sharkoon_dpi_level_human_readable dpi_human = { 0 };

    dpi_human.x = ((dpi_report->x_offset << 8) & 0x0F00);
    dpi_human.x |= dpi_report->x_value;
    dpi_human.x *= 50;

    dpi_human.y = ((dpi_report->y_offset << 8) & 0x0F00);
    dpi_human.y |= dpi_report->y_value;
    dpi_human.y *= 50;

    return dpi_human;
}

/**
 * create an empty get report which can used to ask the device for current settings
 */
static struct sharkoon_report sharkoon_create_empty_get_report(void) {
    struct sharkoon_report rep = { 0 };
    
    rep.header[0] = 0x04;
    rep.header[1] = 0xa0;
    rep.header[2] = 0x01;

    rep.command = SHARKOON_LIGHT2_200_CMD_GET_REPORT;

    return rep;
}

/**
 * create an empty set report to send settings to the device
 */
static struct sharkoon_report sharkoon_create_empty_set_report(void) {

    struct sharkoon_report rep = { 0 };
    
    rep.header[0] = 0x04;
    rep.header[1] = 0xa0;
    rep.header[2] = 0x01;

    rep.command = SHARKOON_LIGHT2_200_CMD_SET_REPORT;
    rep.unknownByte5 = 0x01;
    rep.unknownByte6 = 0x02;
    rep.unknownByte7 = 0xa5;
    rep.unknownByte8 = 0x01;

    return rep;
}

/**
 * reset device to default settings with disabled illimination
 */
static struct sharkoon_report sharkoon_create_empty_set_defaults_no_illumintaion_report(void) {

    struct sharkoon_report rep = sharkoon_create_empty_set_report();

    struct sharkoon_report_dpi_level dpi_rep = sharkoon_dpi_create_level_report_from_human_readable_values(16000, 16000);
    
    rep.lod = 2;

    rep.dpi_settings.dpi2_enabled = 1;
    rep.dpi_settings.dpi1 = dpi_rep;
    rep.dpi_settings.dpi2 = dpi_rep;
    rep.dpi_settings.dpi3 = dpi_rep;
    rep.dpi_settings.dpi4 = dpi_rep;
    rep.dpi_settings.dpi5 = dpi_rep;
    rep.dpi_settings.dpi6 = dpi_rep;
    rep.dpi_settings.dpi7 = dpi_rep;

    //rep.illumination_settings.profile_id = 1;
    rep.illumination_settings.led_effect = 9;
    //rep.illumination_settings.led_frequency = 2;
    //rep.illumination_settings.led_brightness = 10;
    //rep.illumination_settings.unknownByte41 = 1;

    return rep;
}

///////////////////////////////////////////////////////////////


/*
 * Input interrupt completion handler.
 */
// static void sharkoon_light2_200_irq_in(struct urb *urb)
// {
// }

/*
 * Output interrupt completion handler.
 */
static void sharkoon_light2_200_irq_out(struct urb *urb)
{
//     struct hid_device *hid = urb->context;
// 	struct sharkoon_light2_200_device *sharkoon_dev = hid_get_drvdata(hid);

	switch (urb->status) {
	case 0:			/* success */
        printk(KERN_DEBUG "sharkoon_light2_200_irq_out() success \n");
		break;
	case -ESHUTDOWN:	/* unplug */
	case -EILSEQ:		/* protocol error or unplug */
	case -EPROTO:		/* protocol error or unplug */
	case -ECONNRESET:	/* unlink */
	case -ENOENT:
		break;
	default:		/* error */
		hid_warn(urb->dev, "output irq status %d received\n",
			 urb->status);
	}
}


/**
 * Write device file "test"
 */
static ssize_t sharkoon_light2_200_attr_test_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    struct hid_device *hid = to_hid_device(dev);
    struct usb_device *usbdev = interface_to_usbdev(to_usb_interface(dev->parent));
    int r;

    struct sharkoon_light2_200_device *sharkoon_dev = dev_get_drvdata(dev);
    if (!sharkoon_dev)
        return -ENODATA;

    if (!sharkoon_dev->urbout)
    {
        struct sharkoon_report report = sharkoon_create_empty_set_defaults_no_illumintaion_report();

        sharkoon_dev->urbout = usb_alloc_urb(0, GFP_KERNEL);

        // data
        sharkoon_dev->outbuf = memcpy(sharkoon_dev->outbuf, &report, 64);

        usb_fill_int_urb(sharkoon_dev->urbout, 
            usbdev, 
            usb_sndintpipe(usbdev, sharkoon_dev->endpoint_out->bEndpointAddress),
            sharkoon_dev->outbuf,
            64,
            sharkoon_light2_200_irq_out,
            hid,
            sharkoon_dev->endpoint_out->bInterval);

        r = usb_submit_urb(sharkoon_dev->urbout, GFP_ATOMIC); // async submit
        if (r < 0) 
        {
            hid_err(hid, "usb_submit_urb(out) failed: %d\n", r);
            return r;
        }
    }

    return 64;
}



/**
 * Set up the device driver files
 *
 * Read-only is  0444
 * Write-only is 0220
 * Read/write is 0664
 */

static DEVICE_ATTR(test, 0220, NULL, sharkoon_light2_200_attr_test_store);


/**
 * destroy the sharkoon device
 */
static void sharkoon_light2_200_device_destroy(struct hid_device *hid)
{
    struct usb_interface *intf = to_usb_interface(hid->dev.parent);
    struct usb_device *usbdev = interface_to_usbdev(intf);
    struct sharkoon_light2_200_device *sharkoon_dev = hid_get_drvdata(hid);

    if (sharkoon_dev)
    {
        usb_free_urb(sharkoon_dev->urbin);
        usb_free_urb(sharkoon_dev->urbout);
        sharkoon_dev->urbin = NULL;
        sharkoon_dev->urbout = NULL;

        kfree(sharkoon_dev);
        hid_set_drvdata(hid, NULL);

        device_remove_file(&hid->dev, &dev_attr_test);

        usb_free_coherent(usbdev, 64, sharkoon_dev->inbuf, sharkoon_dev->inbuf_dma);
	    usb_free_coherent(usbdev, 64, sharkoon_dev->outbuf, sharkoon_dev->outbuf_dma);
    }
}

/**
 * init the sharkoon device
 */
static int sharkoon_light2_200_device_init(struct hid_device *hid)
{
    struct usb_interface *intf = to_usb_interface(hid->dev.parent);
    struct usb_device *usbdev = interface_to_usbdev(intf);
    struct usb_host_interface *host_intf = intf->cur_altsetting;
    struct sharkoon_light2_200_device *sharkoon_dev = NULL;
    unsigned int i;

    if (host_intf->desc.bInterfaceProtocol == 0 &&
        host_intf->desc.bNumEndpoints == 2)
    {
        sharkoon_dev = kzalloc(sizeof(struct sharkoon_light2_200_device), GFP_KERNEL);

        if (!sharkoon_dev)
            return -ENOMEM;

        sharkoon_dev->endpoint_in = NULL;
        sharkoon_dev->endpoint_out = NULL;

        // find the endpoints for in an out direction
        for (i = 0; i < host_intf->desc.bNumEndpoints; i++)
        {
            // endpoint must have 'interrupt' type
            if (usb_endpoint_xfer_int(&host_intf->endpoint[i].desc))
            {
                if (usb_endpoint_dir_in(&host_intf->endpoint[i].desc))
                    sharkoon_dev->endpoint_in = &host_intf->endpoint[i].desc;
                else
                    sharkoon_dev->endpoint_out = &host_intf->endpoint[i].desc;
            }
        }

        if (sharkoon_dev->endpoint_in && sharkoon_dev->endpoint_out)
        {
            hid_set_drvdata(hid, sharkoon_dev);
            device_create_file(&hid->dev, &dev_attr_test);

            sharkoon_dev->inbuf = usb_alloc_coherent(usbdev, 64, GFP_KERNEL, &sharkoon_dev->inbuf_dma);
	        sharkoon_dev->outbuf = usb_alloc_coherent(usbdev, 64, GFP_KERNEL, &sharkoon_dev->outbuf_dma);
        }
        else
        {
            kfree(sharkoon_dev);
            sharkoon_dev = NULL;
        }
	}

	return 0;
}



/**
 * if report in report_table, this hook is called (NULL means nop)
 */
static int sharkoon_light2_200_raw_event(struct hid_device *hid, struct hid_report *report, u8 *data, int size)
{
    // todo
    return 0;
}


/**
 * new device inserted
 */
static int sharkoon_light2_200_probe(struct hid_device *hid, const struct hid_device_id *id)
{
	int retval;

	retval = hid_parse(hid);
	if (retval)
    {
		hid_err(hid, "parse failed\n");
		goto exit;
	}

	retval = hid_hw_start(hid, HID_CONNECT_DEFAULT);
	if (retval)
    {
		hid_err(hid, "hw start failed\n");
		goto exit;
	}

	retval = sharkoon_light2_200_device_init(hid);
	if (retval)
    {
		hid_err(hid, "couldn't install mouse\n");
		goto exit_stop;
	}

	return 0;

exit_stop:
	hid_hw_stop(hid);
exit:
	return retval;
}

/**
 * device removed
 */
static void sharkoon_light2_200_remove(struct hid_device *hid)
{
    sharkoon_light2_200_device_destroy(hid);
    hid_hw_stop(hid);
}

/**
 * supported devices
 */
static const struct hid_device_id sharkoon_light2_200_devices[] = {
    { HID_USB_DEVICE(USB_VENDOR_ID_SHARKOON, USB_DEVICE_ID_SHARKOON_LIGHT2_200) },
    { }
};

MODULE_DEVICE_TABLE(hid, sharkoon_light2_200_devices);


/**
 * define the driver
 */
static struct hid_driver sharkoon_light2_200_driver = {
    .name      = "sharkoon-light2-200",
    .id_table  = sharkoon_light2_200_devices,
    .probe     = sharkoon_light2_200_probe,
    .remove    = sharkoon_light2_200_remove,
    .raw_event = sharkoon_light2_200_raw_event,
};

module_hid_driver(sharkoon_light2_200_driver);


MODULE_DESCRIPTION("USB Sharkoon Light2 200 device driver");
MODULE_LICENSE("GPL v2");
