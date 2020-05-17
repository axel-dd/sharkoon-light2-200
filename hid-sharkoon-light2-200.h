#ifndef __HID_SHARKOON_LIGHT2_200_H
#define __HID_SHARKOON_LIGHT2_200_H

#define USB_VENDOR_ID_SHARKOON 0x2ea8
#define USB_DEVICE_ID_SHARKOON_LIGHT2_200 0x2203

struct sharkoon_light2_200_device
{
	struct usb_endpoint_descriptor *endpoint_in;
    struct usb_endpoint_descriptor *endpoint_out;

    struct urb *urbin; /* Input URB */
    struct urb *urbout; /* Output URB */

    char *inbuf; /* Input buffer */
    char *outbuf; /* Output buffer */

    dma_addr_t outbuf_dma; /* Output buffer dma */
    dma_addr_t inbuf_dma; /* Input buffer dma */
};

#endif // __HID_SHARKOON_LIGHT2_200_H
