/******************************<%BEGIN LICENSE%>******************************/
// (c) Copyright 2013 Lumidigm, Inc. (Unpublished Copyright) ALL RIGHTS RESERVED.
//
// For a list of applicable patents and patents pending, visit www.lumidigm.com/patents/
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
// FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
// COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
// IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
// CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//
//
/******************************<%END LICENSE%>******************************/
#pragma once

#include "lumi_stdint.h"

//
//    Stream Shared File
//    These Interface Definitions are Shared btw the Cypress 8051 Controller and Host Driver
//

// Product ID's Used to Select Platform
#define _V31x_PLATFORM_PID_        0x0021
#define _M31x_PLATFORM_PID_        0x0041

/*
**    Start of Frame Tag
*/
#define LDI_WRITE_CMD            0x70
#define LDI_READ_CMD            0x71
#define MAX_PAYLOAD_SIZE         54
#define EEPROM_MAX_BYTES        8192            // Number of Bytes in EEPROM

/*
**    Control Transfer Packet Definition
**    MUST be 64 Bytes in Length
*/
#ifdef __GNUC__
#pragma pack(push,1)
#endif
typedef struct
{
    uint8_t id;
    uint8_t cmd;
    uint8_t Device_Address;
    uint8_t Register_Address;
    uint8_t Address_MSB;
    uint8_t Address_LSB;
    uint8_t NumBytes;
    uint8_t Value1;
    uint8_t Value2;
    uint8_t Width;
    uint8_t Data[MAX_PAYLOAD_SIZE];
} _CONTROL_PACKET_TYPE;
#ifdef __GNUC__
#pragma pack(pop)
#endif

/*
**    Vendor Specific Interface Commands
*/
typedef enum
{
    LOOPBACK_WRITE    = 0x20,
    LOOPBACK_READ,
    JTAG_INIT,
    JTAG_EXEC,
    JTAG_WRITE,
    JTAG_READ,
    JTAG_STOP,
    PORTA_WRITE,
    PORTA_READ,
    PORTD_WRITE,
    PORTD_READ,
    I2C_WRITE,
    I2C_READ,
    I2C_EEPROM_WRITE,
    I2C_EEPROM_READ,
    TWI_WRITE,
    TWI_READ,
    TWI_EEPROM_WRITE,
    TWI_EEPROM_READ,
    CONFIG_PORT_DIR,
    XI_SER_WRITE,
    XI_SER_ENABLE,
    GET_FW_REVISION,
    GET_CPLD_REVISION,
    LDI_RENUM
} Vendor_Interface_Type;

/*
**    EEPROM Format
*/
#ifdef __GNUC__
#pragma pack(push,1)
#endif
typedef struct
{
    /*
    **    1st 8 Bytes MUST Comply with Cypress Boot Loader Format
    **    DO NOT MODIFY
    */
    uint8_t
        type,                // Boot Must be either 0xC0 or 0xC2
        VID_LSB,
        VID_MSB,
        PID_LSB,            // Pre-Enum Product ID
        PID_MSB,
        DEVID_LSB,
        DEVID_MSB,
        CONFIG;
    // end required by Cypress format

    // start Lumidigm assignement
    uint32_t
        Serial_Number,
        CPLD_Firmware_Revision,
        ManDateCode;
    uint16_t
        Product_ID,            // Post-Enum Product ID
        Platform_Type,
        Bx_Row,                // V31x Only
        Bx_Col,                // V31x Only
        PD_Row,                // V31x Only
        PD_Col,                // V31x Only
        DPI,                // V31x Only
        MfgStateFlag;        // Manufacturing state flag

    uint8_t
        pCalData[1520];        // V31x Only
    uint8_t
        pTagData[256];        // For V100_Set_Tag and V100_Get_Tag functionality first byte is size

} EEPROM_Format_Type;
#ifdef __GNUC__
#pragma pack(pop)
#endif

