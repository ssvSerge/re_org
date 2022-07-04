#pragma once

#define __BYTE_ORDER __LITTLE_ENDIAN
#include <linux/usb/functionfs.h>


/******************** Little Endian Handling ********************************/

//#define cpu_to_le16(x)  htole16(x)
//#define cpu_to_le32(x)  htole32(x)

/*
 * cpu_to_le16/32 are used when initializing structures, a context where a
 * function call is not allowed. To solve this, we code cpu_to_le16/32 in a way
 * that allows them to be used when initializing structures.
 */
#if __BYTE_ORDER == __LITTLE_ENDIAN
    #define cpu_to_le16(x)  (x)
    #define cpu_to_le32(x)  (x)
#else
    #define cpu_to_le16(x)  ((((x) >> 8) & 0xffu) | (((x) & 0xffu) << 8))
    #define cpu_to_le32(x)  ((((x) & 0xff000000u) >> 24) | (((x) & 0x00ff0000u) >>  8) |  (((x) & 0x0000ff00u) <<  8) | (((x) & 0x000000ffu) << 24))
#endif

#define le32_to_cpu(x)  le32toh(x)
#define le16_to_cpu(x)  le16toh(x)

#ifdef __cplusplus
#define _C_ "C"
#else
#define _C_
#endif


struct Descriptors {
    struct usb_functionfs_descs_head_v2 header;
    __le32 fs_count;
    __le32 hs_count;
    __le32 ss_count;
    __le32 os_count;
    struct {
        struct usb_interface_descriptor intf;
        struct usb_endpoint_descriptor_no_audio sink;
        struct usb_endpoint_descriptor_no_audio source;
    } __attribute__((packed)) fs_descs, hs_descs;
    struct {
        struct usb_interface_descriptor intf;
        struct usb_endpoint_descriptor_no_audio sink;
        struct usb_ss_ep_comp_descriptor sink_comp;
        struct usb_endpoint_descriptor_no_audio source;
        struct usb_ss_ep_comp_descriptor source_comp;
    } ss_descs;
    struct usb_os_desc_header os_header;
    struct usb_ext_compat_desc os_desc;
}   __attribute__((packed));

#define STR_INTERFACE_ "Camera Control"

struct Strings {
    struct usb_functionfs_strings_head header;
    struct {
        __le16 code;
        const char str1[sizeof STR_INTERFACE_];
    }   __attribute__((packed)) lang0;
}   __attribute__((packed));

#define STR_INTERFACE_ "Camera Control"

#ifdef __cplusplus 
extern _C_{
#endif

struct Strings {
    struct usb_functionfs_strings_head header;
    struct {
        __le16 code;
        const char str1[sizeof STR_INTERFACE_];
    } lang0;
} ;


struct Descriptors get_descriptors();
struct Strings     get_strings();

#ifdef __cplusplus 
}
#endif
