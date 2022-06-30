#include "epconst.h"

/******************** Descriptors and Strings *******************************/


struct Descriptors get_descriptors()
{
    struct Descriptors descriptors = {
        .header = {
            .magic = cpu_to_le32(FUNCTIONFS_DESCRIPTORS_MAGIC_V2),
            .flags = cpu_to_le32(FUNCTIONFS_HAS_FS_DESC |
                        FUNCTIONFS_HAS_HS_DESC |
                        FUNCTIONFS_HAS_SS_DESC |
                        FUNCTIONFS_HAS_MS_OS_DESC),
            .length = cpu_to_le32(sizeof descriptors),
        },
        .fs_count = cpu_to_le32(3),
        .fs_descs = {
            .intf = {
                .bLength = sizeof descriptors.fs_descs.intf,
                .bDescriptorType = USB_DT_INTERFACE,
                .bNumEndpoints = 2,
                .bInterfaceClass = USB_CLASS_VENDOR_SPEC,
                .iInterface = 1,
            },
            .sink = {
                .bLength = sizeof descriptors.fs_descs.sink,
                .bDescriptorType = USB_DT_ENDPOINT,
                .bEndpointAddress = 1 | USB_DIR_IN,
                .bmAttributes = USB_ENDPOINT_XFER_BULK,
                /* .wMaxPacketSize = autoconfiguration (kernel) */
            },
            .source = {
                .bLength = sizeof descriptors.fs_descs.source,
                .bDescriptorType = USB_DT_ENDPOINT,
                .bEndpointAddress = 2 | USB_DIR_OUT,
                .bmAttributes = USB_ENDPOINT_XFER_BULK,
                /* .wMaxPacketSize = autoconfiguration (kernel) */
            },
        },
        .hs_count = cpu_to_le32(3),
        .hs_descs = {
            .intf = {
                .bLength = sizeof descriptors.fs_descs.intf,
                .bDescriptorType = USB_DT_INTERFACE,
                .bNumEndpoints = 2,
                .bInterfaceClass = USB_CLASS_VENDOR_SPEC,
                .iInterface = 1,
            },
            .sink = {
                .bLength = sizeof descriptors.hs_descs.sink,
                .bDescriptorType = USB_DT_ENDPOINT,
                .bEndpointAddress = 1 | USB_DIR_IN,
                .bmAttributes = USB_ENDPOINT_XFER_BULK,
                .wMaxPacketSize = cpu_to_le16(512),
            },
            .source = {
                .bLength = sizeof descriptors.hs_descs.source,
                .bDescriptorType = USB_DT_ENDPOINT,
                .bEndpointAddress = 2 | USB_DIR_OUT,
                .bmAttributes = USB_ENDPOINT_XFER_BULK,
                .wMaxPacketSize = cpu_to_le16(512),
                .bInterval = 1, /* NAK every 1 uframe */
            },
        },
        .ss_count = cpu_to_le32(5),
        .ss_descs = {
            .intf = {
                .bLength = sizeof descriptors.fs_descs.intf,
                .bDescriptorType = USB_DT_INTERFACE,
                .bNumEndpoints = 2,
                .bInterfaceClass = USB_CLASS_VENDOR_SPEC,
                .iInterface = 1,
            },
            .sink = {
                .bLength = sizeof descriptors.hs_descs.sink,
                .bDescriptorType = USB_DT_ENDPOINT,
                .bEndpointAddress = 1 | USB_DIR_IN,
                .bmAttributes = USB_ENDPOINT_XFER_BULK,
                .wMaxPacketSize = cpu_to_le16(1024),
            },
            .sink_comp = {
                .bLength = USB_DT_SS_EP_COMP_SIZE,
                .bDescriptorType = USB_DT_SS_ENDPOINT_COMP,
                .bMaxBurst = 0,
                .bmAttributes = 0,
                .wBytesPerInterval = 0,
            },
            .source = {
                .bLength = sizeof descriptors.hs_descs.source,
                .bDescriptorType = USB_DT_ENDPOINT,
                .bEndpointAddress = 2 | USB_DIR_OUT,
                .bmAttributes = USB_ENDPOINT_XFER_BULK,
                .wMaxPacketSize = cpu_to_le16(1024),
                .bInterval = 1, /* NAK every 1 uframe */
            },
            .source_comp = {
                .bLength = USB_DT_SS_EP_COMP_SIZE,
                .bDescriptorType = USB_DT_SS_ENDPOINT_COMP,
                .bMaxBurst = 0,
                .bmAttributes = 0,
                .wBytesPerInterval = 0,
            },
        },
        .os_count = cpu_to_le32(1),
        .os_header = {
            .interface = cpu_to_le32(1),
            .dwLength = cpu_to_le32(sizeof(descriptors.os_header) + sizeof(descriptors.os_desc)),
            .bcdVersion = cpu_to_le32(1),
            .wIndex = cpu_to_le32(4),
            .bCount = cpu_to_le32(1),
            .Reserved = cpu_to_le32(0),
        },
        .os_desc = {
            .bFirstInterfaceNumber = 0,
            .Reserved1 = cpu_to_le32(1),
            .CompatibleID = {'H', 'I', 'D', 'G', '0', '0', '3', '0'},
            .SubCompatibleID = {'C', 'T', 'R', 'L', 'B', 'L', 'D', 'I'},
            .Reserved2 = {0},
        },
    };

    return descriptors;
}
struct Strings get_strings()
{
    static const struct Strings strings = {
        .header = {
            .magic = cpu_to_le32(FUNCTIONFS_STRINGS_MAGIC),
            .length = cpu_to_le32(sizeof strings),
            .str_count = cpu_to_le32(1),
            .lang_count = cpu_to_le32(1),
        },
        .lang0 = {
            cpu_to_le16(0x0409), /* en-us */
            STR_INTERFACE_,
        },
    };

    return strings;
}