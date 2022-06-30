#ifndef __JENGINECONFIG_H__
#define __JENGINECONFIG_H__

#define JENGINE_DEV_USB_VID                 (0x1FAE)
#define JENGINE_DEV_USB_PID                 (0x0320)
#define JENGINE_DEV_SERIAL_NUMBER_SHORT     (0x1234U)
#define JENGINE_DEV_SERIAL_NUMBER_LONG      (0x12345678UL)
#define JENGINE_DEV_HW_REVISION             (00000)
#define JENGINE_DEV_FW_REVISION             (36725)
#define JENGINE_DEV_SPOOF_REVISION          (220)
#define JENGINE_DEV_STRUCT_SIZE             (36700)

// BASE_DEVICE_CONFIG                        0x0400
// HYB02_DEVICE_CONFIG                       0x1300
// CP001_DEVICE_CONFIG                       0x1400
// VEX_DEVICE_CONFIG                         0x0300
// MSK01_DEVICE_CONFIG                       0x0800
// MSK00_DEVICE_CONFIG                       0x0100
#define JENGINE_DEV_DEVICE_CONFIG           (0x0400)


#endif
