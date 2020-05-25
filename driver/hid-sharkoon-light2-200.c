#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/usb.h>
#include <linux/hid.h>

#define USB_VENDOR_ID_SHARKOON 0x2ea8
#define USB_DEVICE_ID_SHARKOON_LIGHT2_200 0x2203

#define SHARKOON_ENDPOINT_DATA_IN 0x82
#define SHARKOON_ENDPOINT_DATA_OUT 0x03
#define SHARKOON_ENDPOINT_INTERVALL 1

#define SHARKOON_DATA_PACKAGE_SIZE 64 // 64 bytes


MODULE_DESCRIPTION("USB Sharkoon Light2 200 device driver");
MODULE_LICENSE("GPL");


/**
 * device driver data
 */
struct sharkoon_light2_200_device
{
    unsigned int bufsize; /* URB buffer size */

    struct urb *urbin; /* Input URB */
    struct urb *urbout; /* Output URB */

    unsigned char *inbuf; /* Input buffer */
    unsigned char *outbuf; /* Output buffer */

    dma_addr_t inbuf_dma; /* Input buffer dma */
    dma_addr_t outbuf_dma; /* Output buffer dma */
};


/**
 * LED color in simple RGB
 */
struct sharkoon_color {
    unsigned char red;
    unsigned char green;
    unsigned char blue;
};

/**
 * DPI for x- and y axis is stored in steps of 50. The regular value must be divided by 50.
 * The device supports regular values between 50 and 16000.
 * As x and y are limited to one byte, the rest is stored in an offset byte.
 * The first 4 bits are for x and the last 4 for y.
 * Sample
 * 0x0 0x1 0xf0 0x2c  | x = 240  Ofx = 0 | ((Ofx * 256) + y) * 50 => 12000 DPI 
 * Ofx Ofy  x    y    | y = 44   Ofy = 1 | ((Ofy * 256) + x) * 50 => 15000 DPI
 */
struct sharkoon_dpi_level {
    unsigned char offset;
    unsigned char x;
    unsigned char y;
};

struct sharkoon_dpi_settings {
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
    struct sharkoon_dpi_level dpi1;
    struct sharkoon_dpi_level dpi2;
    struct sharkoon_dpi_level dpi3;
    struct sharkoon_dpi_level dpi4;
    struct sharkoon_dpi_level dpi5;
    struct sharkoon_dpi_level dpi6;
    struct sharkoon_dpi_level dpi7;
};

struct sharkoon_illumination_settings {
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
    struct sharkoon_color color1;
    struct sharkoon_color color2;
    struct sharkoon_color color3;
    struct sharkoon_color color4;
    struct sharkoon_color color5;
    struct sharkoon_color color6;
    struct sharkoon_color color7;
};

/**
 * message for set / get device settings
 * size: 64 bytes
 */
struct sharkoon_message_settings {
    /**
     * byte 1 - version of the data protocoll
     * value: 0x04
     */
    unsigned char version;

    /**
     * bytes 2-3 - message type
     * value: 0xA001
     */
    unsigned char message_type[2];

    /**
     * byte 4 command for get or set settings
     * 0x01 - get message with current mouse settings
     * 0x02 - submit message to change mouse settings
     */
    unsigned char command;

    /**
     * bytes 5-7 UNKNOWN
     * 0x00, 0x00, 0x00 on 0x01
     * 0x01, 0x02, 0xa5 on 0x02
     */
    unsigned char unknownByte5;
    unsigned char unknownByte6;
    unsigned char unknownByte7;

    /**
     * byte 8
     * current DPI step number of the device (1-7)
     * values: 0-6
     */
    unsigned char dpi_step_id;

    /**
     * bytes 9-30 DPI setting
     */
    struct sharkoon_dpi_settings dpi_settings;

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
    struct sharkoon_illumination_settings illumination_settings;

    /**
     * byte 64 UNKNOWN
     * value: mostly 0
     */
    unsigned char unknownByte64;
};

/**
 * 64 bytes 
 * message is send on DPI changed by device
 */
struct sharkoon_message_dpi_changed {
    /**
     * byte 1 - version of the data protocoll
     * value: 0x04
     */
    unsigned char version;

    /**
     * bytes 2-3 - message type
     * value: 0xA202
     */
    unsigned char message_type[2];

    /**
     * byte 4 - DPI step number of the device (1-7)
     * values: 0-6
     */
    unsigned char dpi_step_id;

    /**
     * byte 5-7 - DPI values
     */
    struct sharkoon_dpi_level dpi;

    /**
     * byte 8-64 - zero bytes
     */
    unsigned char zero_bytes[57];
};



/**
 * creates DPI level message from human readable dpi values
 * \param x DPI value of y-axis in human readable units (value range between 50 and 16000 in steps of 50)
 * \param y DPI value of y-axis in human readable units (value range between 50 and 16000 in steps of 50)
 * \return struct sharkoon_dpi_level - for usage in sharkoon_dpi_settings
 */
struct sharkoon_dpi_level sharkoon_dpi_create_level_message_from_human_readable_values(unsigned short x, unsigned short y) 
{
    struct sharkoon_dpi_level dpi = { 0 };

    x /= 50;
    y /= 50;

    dpi.x = x & 0xFF;
    dpi.y = y & 0xFF;

    //  0x0 [0x1] 0x0 0x0
    // [0x1] 0x0
    dpi.offset = (x >> 4) & 0xF0;

    // 0x0 [0x1] 0x0 0x0
    // 0x0 [0x1]
    dpi.offset |= (y >> 8) & 0x0F;

    return dpi;
}

/**
 * get human readable dpi values from a DPI level message
 * \param dpi [IN] DPI level message
 * \param x [OUT] DPI value of y-axis in human readable units (value range between 50 and 16000 in steps of 50)
 * \param y [OUT] DPI value of y-axis in human readable units (value range between 50 and 16000 in steps of 50)
 */
void sharkoon_dpi_get_human_readable_values_from_level_message(struct sharkoon_dpi_level *dpi, unsigned short *x, unsigned short *y)
{
    //           [0x1] 0x0
    // 0x0 [0x1]  0x0  0x0
    *x  = (dpi->offset << 4) & 0x0F00;
    *x |= dpi->x;
    *x *= 50;

    //           0x0 [0x1]
    // 0x0 [0x1] 0x0  0x0
    *y  = (dpi->offset << 8) & 0x0F00;
    *y |= dpi->y;
    *y *= 50;
}

/**
 * create an empty get message which can used to ask the device for current settings
 */
static struct sharkoon_message_settings sharkoon_create_empty_get_message(void) {
    struct sharkoon_message_settings rep = { 0 };

    rep.version = 0x04;
    rep.message_type[0] = 0xA0;
    rep.message_type[1] = 0x01;
    rep.command = 0x01;

    return rep;
}

/**
 * create an empty set message to send settings to the device
 */
static struct sharkoon_message_settings sharkoon_create_empty_set_message(void) {

    struct sharkoon_message_settings rep = { 0 };
    
    rep.version = 0x04;
    rep.message_type[0] = 0xA0;
    rep.message_type[1] = 0x01;
    rep.command = 0x02;

    rep.unknownByte5 = 0x01;
    rep.unknownByte6 = 0x02;
    rep.unknownByte7 = 0xa5;

    rep.dpi_step_id = 0x01;

    return rep;
}

/**
 * creates a settings message with default settings bit with all illumination disabled
 */
static struct sharkoon_message_settings sharkoon_create_test_message(void) {

    struct sharkoon_message_settings msg = sharkoon_create_empty_set_message();
    
    msg.lod = 2;

    msg.dpi_settings.dpi1_enabled = 1;
    msg.dpi_settings.dpi2_enabled = 1;
    msg.dpi_settings.dpi3_enabled = 1;
    msg.dpi_settings.dpi4_enabled = 1;
    msg.dpi_settings.dpi5_enabled = 1;
    msg.dpi_settings.dpi6_enabled = 1;
    msg.dpi_settings.dpi7_enabled = 1;

    msg.dpi_settings.dpi1 = sharkoon_dpi_create_level_message_from_human_readable_values(400, 400);
    msg.dpi_settings.dpi2 = sharkoon_dpi_create_level_message_from_human_readable_values(800, 800);;
    msg.dpi_settings.dpi3 = sharkoon_dpi_create_level_message_from_human_readable_values(1200, 1200);;
    msg.dpi_settings.dpi4 = sharkoon_dpi_create_level_message_from_human_readable_values(2400, 2400);;
    msg.dpi_settings.dpi5 = sharkoon_dpi_create_level_message_from_human_readable_values(3200, 3200);;
    msg.dpi_settings.dpi6 = sharkoon_dpi_create_level_message_from_human_readable_values(6400, 6400);;
    msg.dpi_settings.dpi7 = sharkoon_dpi_create_level_message_from_human_readable_values(16000, 16000);;

    msg.illumination_settings.profile_id = 1;
    msg.illumination_settings.led_effect = 9;
    msg.illumination_settings.led_frequency = 2;
    msg.illumination_settings.led_brightness = 10;
    msg.illumination_settings.unknownByte41 = 1;

    return msg;
}

///////////////////////////////////////////////////////////////


#define	hid_to_usb_dev(hid_dev) \
	container_of(hid_dev->dev.parent->parent, struct usb_device, dev)



/**
 * Read device file "test"
 */
// static ssize_t sharkoon_light2_200_attr_test_show(struct device *dev, struct device_attribute *attr, char *buf)
// {
//     /*const unsigned char *data = NULL;
//     struct sharkoon_light2_200_device *sharkoon_dev = dev_get_drvdata(dev);
//     if (!sharkoon_dev)
//         return -ENODEV;

//     data = sharkoon_dev->inbuf;

//     return scnprintf(buf, PAGE_SIZE,
// "0x%02hhx, 0x%02hhx, 0x%02hhx, 0x%02hhx, 0x%02hhx, 0x%02hhx, 0x%02hhx, 0x%02hhx,\n\
// 0x%02hhx, 0x%02hhx, 0x%02hhx, 0x%02hhx, 0x%02hhx, 0x%02hhx, 0x%02hhx, 0x%02hhx,\n\
// 0x%02hhx, 0x%02hhx, 0x%02hhx, 0x%02hhx, 0x%02hhx, 0x%02hhx, 0x%02hhx, 0x%02hhx,\n\
// 0x%02hhx, 0x%02hhx, 0x%02hhx, 0x%02hhx, 0x%02hhx, 0x%02hhx, 0x%02hhx, 0x%02hhx,\n\
// 0x%02hhx, 0x%02hhx, 0x%02hhx, 0x%02hhx, 0x%02hhx, 0x%02hhx, 0x%02hhx, 0x%02hhx,\n\
// 0x%02hhx, 0x%02hhx, 0x%02hhx, 0x%02hhx, 0x%02hhx, 0x%02hhx, 0x%02hhx, 0x%02hhx,\n\
// 0x%02hhx, 0x%02hhx, 0x%02hhx, 0x%02hhx, 0x%02hhx, 0x%02hhx, 0x%02hhx, 0x%02hhx,\n\
// 0x%02hhx, 0x%02hhx, 0x%02hhx, 0x%02hhx, 0x%02hhx, 0x%02hhx, 0x%02hhx, 0x%02hhx\n",
// data[0],  data[1],  data[2],  data[3],  data[4],  data[5],  data[6],  data[7],
// data[8],  data[9],  data[10], data[11], data[12], data[13], data[14], data[15],
// data[16], data[17], data[18], data[19], data[20], data[21], data[22], data[23],
// data[24], data[25], data[26], data[27], data[28], data[29], data[30], data[31],
// data[32], data[33], data[34], data[35], data[36], data[37], data[38], data[39],
// data[40], data[41], data[42], data[43], data[44], data[45], data[46], data[47],
// data[48], data[49], data[50], data[51], data[52], data[53], data[54], data[55],
// data[56], data[57], data[58], data[59], data[60], data[61], data[62], data[63]); 
// */

//     dev_info(dev, "%s passed\n", __func__);

//     return scnprintf(buf, PAGE_SIZE, "%s\n", "Sharkoon Light2 200 device");
// }

/**
 * Write device file "test"
 */
static ssize_t sharkoon_light2_200_attr_test_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    int status;
    struct sharkoon_message_settings msg;

    struct sharkoon_light2_200_device *sharkoon_dev = dev_get_drvdata(dev);
    if (!sharkoon_dev)
        return -ENODEV;

    dev_info(dev, "%s passed\n", __func__);

    msg = sharkoon_create_empty_get_message();

    sharkoon_dev->outbuf = memcpy(sharkoon_dev->outbuf, &msg, sharkoon_dev->bufsize);

    status = usb_submit_urb(sharkoon_dev->urbout, GFP_ATOMIC); // async submit
    if (status < 0) 
    {
        dev_err(dev, "ERROR in %s - submit urbout failed, status %d\n", __func__ , status);
        return count;
    }

    dev_info(dev, "%s success\n", __func__);

    return count;
}



/**
 * Set up the device driver files
 *
 * Read-only is  0444
 * Write-only is 0220
 * Read/write is 0664
 */

static DEVICE_ATTR(test, 0220, NULL, sharkoon_light2_200_attr_test_store);


/*
 * Input interrupt completion handler.
 */
static void sharkoon_light2_200_usb_data_in(struct urb *urb)
{
    //struct hid_device *hid = urb->context;
    //struct sharkoon_light2_200_device *sharkoon_dev = hid_get_drvdata(hid);
    int	status;
    //unsigned int i;

    switch (urb->status) 
    {
	case 0:			/* success */
        //hid_info(urb->dev, "%s success\n", __func__);
        // printk(KERN_DEBUG "INDATA: ");
        // for(i = 0; i < sharkoon_dev->bufsize; i++) {
        //     printk(KERN_CONT "\\%02hhx", sharkoon_dev->inbuf[i]);
        // }
        // printk(KERN_CONT "\n");
		break;
	case -EPIPE:		/* stall */
	case -ECONNRESET:	/* unlink */
	case -ENOENT:
	case -ESHUTDOWN:	/* unplug */
	case -EILSEQ:		/* protocol error or unplug */
	case -EPROTO:		/* protocol error or unplug */
	case -ETIME:		/* protocol error or unplug */
	case -ETIMEDOUT:	/* Should never happen, but... */
        return;
	default:		    /* error */
		hid_warn(urb->dev, "urbin status %d received\n", urb->status);
	}

	status = usb_submit_urb(urb, GFP_ATOMIC);
	if (status < 0)
        hid_err(urb->dev, "ERROR in %s - resubmit urbin failed, status %d\n", __func__ , status);
}

/*
 * Output interrupt completion handler.
 */
static void sharkoon_light2_200_usb_data_out(struct urb *urb)
{
	switch (urb->status)
    {
	case 0:			/* success */
        //hid_info(urb->dev, "%s success\n", __func__);
		break;
	case -ESHUTDOWN:	/* unplug */
	case -EILSEQ:		/* protocol error or unplug */
	case -EPROTO:		/* protocol error or unplug */
	case -ECONNRESET:	/* unlink */
	case -ENOENT:
		break;
	default:		/* error */
		hid_warn(urb->dev, "urbout status %d received\n", urb->status);
	}
}

/**
 * destroy the sharkoon device
 */
static void sharkoon_light2_200_device_destroy(struct hid_device *hid)
{
    struct usb_device *usbdev = hid_to_usb_dev(hid);
    struct sharkoon_light2_200_device *sharkoon_dev = hid_get_drvdata(hid);

    if (sharkoon_dev)
    {
        if (sharkoon_dev->urbin)
            usb_free_urb(sharkoon_dev->urbin);
        if (sharkoon_dev->urbout)
            usb_free_urb(sharkoon_dev->urbout);
        if (sharkoon_dev->inbuf)
            usb_free_coherent(usbdev, sharkoon_dev->bufsize, sharkoon_dev->inbuf, sharkoon_dev->inbuf_dma);
        if (sharkoon_dev->outbuf)
            usb_free_coherent(usbdev, sharkoon_dev->bufsize, sharkoon_dev->outbuf, sharkoon_dev->outbuf_dma);

        kfree(sharkoon_dev);

        hid_set_drvdata(hid, NULL);

        device_remove_file(&hid->dev, &dev_attr_test);
    }
}

/**
 * init the sharkoon device
 */
static void sharkoon_light2_200_device_init(struct hid_device *hid)
{
    struct usb_interface *intf = to_usb_interface(hid->dev.parent);
    struct usb_device *usbdev = hid_to_usb_dev(hid);
    struct sharkoon_light2_200_device *sharkoon_dev = NULL;
    int status;

    if (intf->cur_altsetting->desc.bInterfaceProtocol != 0 ||
        intf->cur_altsetting->desc.bNumEndpoints != 2)
        return;

    sharkoon_dev = kzalloc(sizeof(struct sharkoon_light2_200_device), GFP_KERNEL);
    if (!sharkoon_dev)
        return;

    memset(sharkoon_dev, 0, sizeof(struct sharkoon_light2_200_device));
    sharkoon_dev->bufsize = SHARKOON_DATA_PACKAGE_SIZE;

    sharkoon_dev->outbuf = usb_alloc_coherent(usbdev, sharkoon_dev->bufsize, GFP_KERNEL, &sharkoon_dev->outbuf_dma);
    if (!sharkoon_dev->outbuf)
    {
        hid_err(hid, "ERROR in %s - alloc outbuf failed.\n", __func__);
        goto error;
    }
		
    sharkoon_dev->inbuf = usb_alloc_coherent(usbdev, sharkoon_dev->bufsize, GFP_KERNEL, &sharkoon_dev->inbuf_dma);
    if (!sharkoon_dev->inbuf)
    {
        hid_err(hid, "ERROR in %s - alloc inbuf failed.\n", __func__);
        goto error;
    }

    sharkoon_dev->urbout = usb_alloc_urb(0, GFP_KERNEL);
    if (!sharkoon_dev->urbout)
    {
        hid_err(hid, "ERROR in %s - alloc urbout failed.\n", __func__);
        goto error;
    }

    sharkoon_dev->urbin = usb_alloc_urb(0, GFP_KERNEL);
    if (!sharkoon_dev->urbin)
    {
        hid_err(hid, "ERROR in %s - alloc urbin failed.\n", __func__);
        goto error;
    }

    usb_fill_int_urb(sharkoon_dev->urbout, 
        usbdev, 
        usb_sndintpipe(usbdev, SHARKOON_ENDPOINT_DATA_OUT),
        sharkoon_dev->outbuf,
        sharkoon_dev->bufsize,
        sharkoon_light2_200_usb_data_out,
        hid,
        SHARKOON_ENDPOINT_INTERVALL);

    sharkoon_dev->urbout->transfer_dma = sharkoon_dev->outbuf_dma;
	sharkoon_dev->urbout->transfer_flags |= URB_NO_TRANSFER_DMA_MAP;
    
    usb_fill_int_urb(sharkoon_dev->urbin, 
        usbdev, 
        usb_rcvintpipe(usbdev, SHARKOON_ENDPOINT_DATA_IN),
        sharkoon_dev->inbuf,
        sharkoon_dev->bufsize,
        sharkoon_light2_200_usb_data_in,
        hid,
        SHARKOON_ENDPOINT_INTERVALL);


    sharkoon_dev->urbin->transfer_dma = sharkoon_dev->inbuf_dma;    
	sharkoon_dev->urbin->transfer_flags |= URB_NO_TRANSFER_DMA_MAP;

    hid_set_drvdata(hid, sharkoon_dev);

    status = usb_submit_urb(sharkoon_dev->urbin, GFP_KERNEL);
    if (status < 0)
    {
        hid_err(hid, "ERROR in %s - submit urbin failed, status %d\n", __func__, status);
        goto error;
    }

    device_create_file(&hid->dev, &dev_attr_test);

	return;

error:
    if (sharkoon_dev->urbin)
        usb_free_urb(sharkoon_dev->urbin);
    if (sharkoon_dev->urbout)
        usb_free_urb(sharkoon_dev->urbout);
    if (sharkoon_dev->inbuf)
        usb_free_coherent(usbdev, sharkoon_dev->bufsize, sharkoon_dev->inbuf, sharkoon_dev->inbuf_dma);
    if (sharkoon_dev->outbuf)
        usb_free_coherent(usbdev, sharkoon_dev->bufsize, sharkoon_dev->outbuf, sharkoon_dev->outbuf_dma);

    hid_set_drvdata(hid, NULL);
    kfree(sharkoon_dev);
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
		return retval;
	}

	retval = hid_hw_start(hid, HID_CONNECT_DEFAULT);
	if (retval)
    {
		hid_err(hid, "hw start failed\n");
		return retval;
	}

    sharkoon_light2_200_device_init(hid);

	return 0;
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
    .remove    = sharkoon_light2_200_remove
};

module_hid_driver(sharkoon_light2_200_driver);
