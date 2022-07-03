#ifndef _UAPI__LINUX_FUNCTIONFS_H__
#define _UAPI__LINUX_FUNCTIONFS_H__

#include <stdint.h>


#ifdef __cplusplus 
extern "C" {
#endif

uint32_t le32toh(uint32_t little_endian_32bits);
uint16_t le16toh(uint16_t little_endian_16bits);

#ifdef __cplusplus 
}
#endif


// #include <linux/types.h>
// #include <linux/ioctl.h>
// 
// #include <linux/usb/ch9.h>

#define O_NDELAY		0
#define O_NONBLOCK		0

typedef uint32_t __le32;
typedef uint16_t __le16;
typedef uint8_t  __u8;

typedef struct usb_functionfs_descs_head_v2 {
	__le32 magic;
	__le32 length;
	__le32 flags;
	/*
	 * __le32 fs_count, hs_count, fs_count; must be included manually in
	 * the structure taking flags into consideration.
	 */
} usb_functionfs_descs_head_v2;

typedef struct usb_interface_descriptor  {
	__le32		bLength;
	__le32		bDescriptorType;
	__le32		bAlternateSetting;
	__le32		bNumEndpoints;
	__le32		bInterfaceClass;
	__le32		iInterface;
}   usb_interface_descriptor;

typedef struct usb_endpoint_descriptor_no_audio {
	__u8		bLength;
	__u8		bDescriptorType;
	__u8		bEndpointAddress;
	__u8		bmAttributes;
	__le16		wMaxPacketSize;
	__u8		bInterval;
}	usb_endpoint_descriptor_no_audio;


typedef struct usb_ss_ep_comp_descriptor {
	__u8 		bLength;
	__u8 		bDescriptorType;
	__u8 		bMaxBurst;
	__u8 		bmAttributes;
	__le16 		wBytesPerInterval;
}	usb_ss_ep_comp_descriptor;

typedef struct usb_os_desc_header {
	__u8	interface;
	__le32	dwLength;
	__le16	bcdVersion;
	__le16	wIndex;
	union {
		struct {
			__u8	bCount;
			__u8	Reserved;
		};
		__le16	wCount;
	};
}	usb_os_desc_header;


typedef struct usb_ext_compat_desc {
	__u8	bFirstInterfaceNumber;
	__u8	Reserved1;
	__u8	CompatibleID[8];
	__u8	SubCompatibleID[8];
	__u8	Reserved2[6];
}	usb_ext_compat_desc;

typedef struct usb_functionfs_strings_head {
	__le32 magic;
	__le32 length;
	__le32 str_count;
	__le32 lang_count;
}	usb_functionfs_strings_head;		  


typedef struct usb_ctrlrequest {
	__u8 	bRequestType;
	__u8 	bRequest;
	__le16 	wValue;
	__le16 	wIndex;
	__le16 	wLength;
}	usb_ctrlrequest;

struct usb_functionfs_event {
	union {
		struct usb_ctrlrequest  setup;
	}	u;
	__u8                type;
	__u8                _pad[3];
};


enum {
	FUNCTIONFS_DESCRIPTORS_MAGIC = 1,
	FUNCTIONFS_STRINGS_MAGIC = 2,
	FUNCTIONFS_DESCRIPTORS_MAGIC_V2 = 3,
};

enum functionfs_flags {
	FUNCTIONFS_HAS_FS_DESC = 1,
	FUNCTIONFS_HAS_HS_DESC = 2,
	FUNCTIONFS_HAS_SS_DESC = 4,
	FUNCTIONFS_HAS_MS_OS_DESC = 8,
	FUNCTIONFS_VIRTUAL_ADDR = 16,
	FUNCTIONFS_EVENTFD = 32,
	FUNCTIONFS_ALL_CTRL_RECIP = 64,
	FUNCTIONFS_CONFIG0_SETUP = 128,
};

enum usb_functionfs_event_type {
	FUNCTIONFS_BIND,
	FUNCTIONFS_UNBIND,
	FUNCTIONFS_ENABLE,
	FUNCTIONFS_DISABLE,
	FUNCTIONFS_SETUP,
	FUNCTIONFS_SUSPEND,
	FUNCTIONFS_RESUME
};

#define USB_DT_DEVICE			0x01
#define USB_DT_CONFIG			0x02
#define USB_DT_STRING			0x03
#define USB_DT_INTERFACE		0x04
#define USB_DT_ENDPOINT			0x05
#define USB_DT_DEVICE_QUALIFIER		0x06
#define USB_DT_OTHER_SPEED_CONFIG	0x07
#define USB_DT_INTERFACE_POWER		0x08
/* these are from a minor usb 2.0 revision (ECN) */
#define USB_DT_OTG			0x09
#define USB_DT_DEBUG			0x0a
#define USB_DT_INTERFACE_ASSOCIATION	0x0b
/* these are from the Wireless USB spec */
#define USB_DT_SECURITY			0x0c
#define USB_DT_KEY			0x0d
#define USB_DT_ENCRYPTION_TYPE		0x0e
#define USB_DT_BOS			0x0f
#define USB_DT_DEVICE_CAPABILITY	0x10
#define USB_DT_WIRELESS_ENDPOINT_COMP	0x11
#define USB_DT_WIRE_ADAPTER		0x21
#define USB_DT_RPIPE			0x22
#define USB_DT_CS_RADIO_CONTROL		0x23
/* From the T10 UAS specification */
#define USB_DT_PIPE_USAGE		0x24
/* From the USB 3.0 spec */
#define	USB_DT_SS_ENDPOINT_COMP		0x30


#define USB_CLASS_PER_INTERFACE		0	/* for DeviceClass */
#define USB_CLASS_AUDIO			1
#define USB_CLASS_COMM			2
#define USB_CLASS_HID			3
#define USB_CLASS_PHYSICAL		5
#define USB_CLASS_STILL_IMAGE		6
#define USB_CLASS_PRINTER		7
#define USB_CLASS_MASS_STORAGE		8
#define USB_CLASS_HUB			9
#define USB_CLASS_CDC_DATA		0x0a
#define USB_CLASS_CSCID			0x0b	/* chip+ smart card */
#define USB_CLASS_CONTENT_SEC		0x0d	/* content security */
#define USB_CLASS_VIDEO			0x0e
#define USB_CLASS_WIRELESS_CONTROLLER	0xe0
#define USB_CLASS_MISC			0xef
#define USB_CLASS_APP_SPEC		0xfe
#define USB_CLASS_VENDOR_SPEC		0xff
#define USB_SUBCLASS_VENDOR_SPEC	0xff

#define USB_ENDPOINT_NUMBER_MASK	0x0f	/* in bEndpointAddress */
#define USB_ENDPOINT_DIR_MASK		0x80

#define USB_ENDPOINT_XFERTYPE_MASK	0x03	/* in bmAttributes */
#define USB_ENDPOINT_XFER_CONTROL	0
#define USB_ENDPOINT_XFER_ISOC		1
#define USB_ENDPOINT_XFER_BULK		2
#define USB_ENDPOINT_XFER_INT		3
#define USB_ENDPOINT_MAX_ADJUSTABLE	0x80

/* The USB 3.0 spec redefines bits 5:4 of bmAttributes as interrupt ep type. */
#define USB_ENDPOINT_INTRTYPE		0x30
#define USB_ENDPOINT_INTR_PERIODIC	(0 << 4)
#define USB_ENDPOINT_INTR_NOTIFICATION	(1 << 4)

#define USB_ENDPOINT_SYNCTYPE		0x0c
#define USB_ENDPOINT_SYNC_NONE		(0 << 2)
#define USB_ENDPOINT_SYNC_ASYNC		(1 << 2)
#define USB_ENDPOINT_SYNC_ADAPTIVE	(2 << 2)
#define USB_ENDPOINT_SYNC_SYNC		(3 << 2)

#define USB_ENDPOINT_USAGE_MASK		0x30
#define USB_ENDPOINT_USAGE_DATA		0x00
#define USB_ENDPOINT_USAGE_FEEDBACK	0x10
#define USB_ENDPOINT_USAGE_IMPLICIT_FB	0x20	/* Implicit feedback Data endpoint */

#define USB_DIR_OUT			0		/* to device */
#define USB_DIR_IN			0x80		/* to host */

#define USB_DT_SS_EP_COMP_SIZE		6





#endif

