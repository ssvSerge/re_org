#pragma once

// The following defines the contents of the _V100_MFG_STATE structure.
typedef struct
{
    // General Information
    char                MfgStateTag[8];                // Constant tag of "MFGSTATE" to easily recognize exiting data
    unsigned int        DeviceSerialNumber;            // Device Serial Number
    unsigned int        FirmwareRevision;            // SVN revision number of the firmware that wrote this structure
    char                  MfgToolRevision[12];        // major.minor.build (12 bytes)
    char                MfgDateCode[12];            // Manufacturing Date Code (12 bytes): <manufacturer code>-dd/mm/yyyy
    unsigned int        CalibrationType;            // Type of calibration present on the device
    unsigned int        SensorType;                    // One of the predefined sensor types (see _V100_SENSOR_TYPE)
    // Hardware Capability Information
    unsigned int         SecureElementType;            // Type of Secure Element component present on the device
    unsigned int         CryptoAuthType;                // Type of Secure Hashing Hardware component present on the device
    unsigned int         TopHalfType;                // Type of Top Half LED configuration
    unsigned int        ProcessorType;                // Type of processor present on the device
    unsigned int         FlashType;                    // Type of FLASH component present on the device
    unsigned int         ImagerType;                    // Type of imager component present on the device
    // Licensing Information
    unsigned int        LicenseType;                // Type of licence installed on the device
    // Cryptography Information
    unsigned int        KeyState;                    // Key Provisioning State
    // Reserved fields for future use
    unsigned int         RESERVED_0;
    unsigned int         RESERVED_1;
    unsigned int         RESERVED_2;
    unsigned int         RESERVED_3;
    unsigned int         RESERVED_4;
    unsigned int         RESERVED_5;
    unsigned int         RESERVED_6;
    unsigned int         RESERVED_7;
    unsigned int         RESERVED_8;
    unsigned int         RESERVED_9;
    unsigned int        RESERVED_10;
    // Checksum
    unsigned int         Checksum;                    // For validating data
} _V100_MFG_STATE;


// -- Individual fields are defined and explained here --
// ManufacturingStateTag
//    Description: Marker for detecting if this information is present on a device
//const char* MFG_STATE_MARKER = "MFGSTATE";

// DeviceSerialNumber
//  Description: Serial number assigned to the device at manufacturing

// FirmwareRevision;
//  Description: SVN revision number of the firmware that wrote this structure
//  Notes:       The firmware writing this structure will populate this field with it's version information

// MfgToolRevision
// Description: The version (major.minor.build) of the manufacturing tool used to write this structure (12 bytes)
// Example:     04.20.03
// Example:        IFW Update    (for field update using intermediate firmware)

// MfgDateCode
// Description: Manufacturing Date Code (12 bytes): <manufacturer code> dd/mm/yyyy
// Example:     B 04/05/2018 (for a unit built at Gitel on April 4, 2018)

// CalibrationType
//  Description: Type of calibration present on the device
//    Notes:          Determined by firmware
//                 Can be one of the following:
typedef enum Tag_MS_CAL_TYPE
{
    MS_CAL_LEGACY_FF_COLOR    = 1,    // Legacy calibration (FF coeffs + color for V, color for M)
    MS_CAL_ESPOOF_COLOR        = 2        // eSpoof calibration (brightness + color)
} MS_CAL_TYPE;

// SensorType
//  Description: Type of sensor platform.
//    Notes:          Provided by manufacturing tool
//                 Can be one of the predefined sensor types as defined by _V100_SENSOR_TYPE:



// SecureElementType
//  Description: Type of Secure Element component present on the device
//    Notes:          Determined by firmware
//                 Can be one of the following:
typedef enum Tag_MS_SECURE_ELEMENT_TYPE
{
    MS_SECURE_ELEMENT_NONE        = 0,    // Typical for non-secure sensors
    MS_SECURE_ELEMENT_MAXQ1850    = 1,    // Typical for V40x and V42x sensors
    MS_SECURE_ELEMENT_DS3641B    = 2        // Typical for M42x sensors
} MS_SECURE_ELEMENT_TYPE;

// CryptoAuthType
//  Description: Type of Secure Hashing Hardware component present on the device
//    Notes:          Determined by firmware
//                 Can be one of the following:
typedef enum Tag_MS_CRYPTO_AUTH_TYPE
{
    MS_CRYPTO_AUTH_NONE            = 0,    // Typical for non-secure sensors
    MS_CRYPTO_AUTH_ATSHA204        = 1        // Typical for V40x and V42x sensors
} MS_CRYPTO_AUTH_TYPE;

// TopHalfType
//  Description: Type of Top Half LED configuration
//    Notes:          Provided by manufacturing tool
//                 Can be one of the following:
typedef enum Tag_MS_TOPHALF_TYPE
{
    MS_TOPHALF_VERSION_1        = 1,    // Osram (legacy 000900)
    MS_TOPHALF_VERSION_2        = 2        // Everlight (new MDP-02521)
} MS_TOPHALF_TYPE;

// ProcessorType
//  Description: Type of processor present on the device
//    Notes:          Determined by firmware
//                 Can be one of the following:
typedef enum Tag_MS_PROCESSOR_TYPE
{
    MS_PROC_BF52x                = 1,    // Analog Devices Blackfin 52x
    MS_PROC_BF53x                = 2,    // Analog Devices Blackfin 53x
    MS_PROC_PC                    = 3        //
} MS_PROCESSOR_TYPE;

// FlashType
//  Description: Type of FLASH component present on the device
//    Notes:          Determined by firmware
//                 Can be one of the following:
typedef enum Tag_MS_FLASH_TYPE
{
    MS_FLASH_NUMONYX_NAND        = 1,
    MS_FLASH_MICRON_NAND        = 2,
    MS_FLASH_WINBOND_NAND        = 3,
    MS_FLASH_NUMONYX_NOR        = 4,
    MS_FLASH_MACRONIX_NOR        = 5
} MS_FLASH_TYPE;

// ImagerType
//  Description: Type of imager component present on the device
//    Notes:          Determined by firmware
//                 Can be one of the following:
typedef enum Tag_MS_IMAGER_TYPE
{
    MS_IMAGER_MICRON_COLOR        = 1,    // V-series
    MS_IMAGER_MICRON_BW         = 2,    // V-series/DAP
    MS_IMAGER_OMNIVISION_COLOR    = 3        // M-series)
} MS_IMAGER_TYPE;

// LicenseType
//  Description: Type of licence installed on the device
//  Notes:          No licensing information defined at this time
typedef enum Tag_MS_LICENSE_TYPE
{
    MS_LICENSE_NONE             = 0    // No licensing information available at this time
} MS_LICENSE_TYPE;

// KeyState
//  Description: Key Provisioning State
//    Notes:          Determined by firmware
//                 Can be a bit-mask of the following options:
typedef enum Tag_MS_KEY_STATE
{
    MS_KEY_STATE_NONE             =  0,    // No keys loaded on device
    MS_KEY_STATE_STANDARD_LTK     =  1,    // Standard Lumi Transport Key loaded on the device
    MS_KEY_STATE_RSA_KEYS_GEN    =  2,    // Device has generated RSA key pair
    MS_KEY_STATE_HID_FKL        =  4,    // Device completed HID Factory Key Load (HID PKI certificates loaded onto the device)
    MS_KEY_STATE_CUSTOMER_FKL    =  8,    // Device completed Customer Factory Key Load
    MS_KEY_STATE_FKL_LOCKED        = 16    // Device Factory Key Load is completed and locked
} MS_KEY_STATE;

// Checksum
//  Description: Used for validating data integrity
