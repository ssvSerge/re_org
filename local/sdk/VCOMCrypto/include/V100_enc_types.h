/*************************************************************************************************************************
**                                                                                                                      **
** ┬ęCopyright 2019 HID Global Corporation/ASSA ABLOY AB. ALL RIGHTS RESERVED.                                           **
**                                                                                                                      **
** For a list of applicable patents and patents pending, visit www.hidglobal.com/patents                                **
**                                                                                                                      **
** THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR                                           **
** IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS                                     **
** FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR                                       **
** COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER                                       **
** IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN                                              **
** CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                                           **
**                                                                                                                      **
*************************************************************************************************************************/
#ifndef __V100_ENC_TYPES
#define __V100_ENC_TYPES
#include "lumi_stdint.h"

// Encrypted data structures.

#ifdef _VDSP
#pragma diag(suppress: 301)
#endif



#define KCV_LENGTH            4     // Key Check Value Length in Bytes
#define RSA_LENGTH          256     // RSA key length (2048 bits)
#define ANBIO_LENGTH         16     // Size of the Random Number generated by the Device
#define ANSOL_LENGTH         16     // Size of the Random Number generated by the trusted secure endpoint
#define BLOCK_LENGTH         16     // For Crypto the buffer needs be aligned of BLOCK_LENGTH
#define MAX_VERIFY_RECORDS   32     // Maximum number of records allowed to be passed into the Verify function



typedef u8 u128 [  16 ];
typedef u8 u256 [  32 ];
typedef u8 u2048[ 256 ];


// Firmware modes
typedef enum
{
    FIRMWARE_MODE_LEGACY    = 0x00,
    FIRMWARE_MODE_UNLOCKED    = 0x01,
    FIRMWARE_MODE_LOCKED    = 0x02,
    FIRMWARE_MODE_SHIPPING    = 0x0B,
    FIRMWARE_MODE_PKI_SC    = 0x0C,
}_V100_ENC_FIRMWARE_MODE;


typedef enum
{
    CAPTURE_IMAGE        = 0x00,
    CAPTURE_VERIFY        = 0x10, // Capture is being done for verification purposes
    CAPTURE_ENROLL_1    = 0x21, // First  Enrollment Capture
    CAPTURE_ENROLL_2    = 0x22, // Second Enrollment Capture
    CAPTURE_ENROLL_3    = 0x23, // Third Enrollment Capture
    CAPTURE_FINGER_LIFT = 0x30, // Make sure finger is lifted after previous insertion
} _V100_ENC_CAPTURE_TYPE;


typedef enum
{
    SPOOF_PROTECT_DONT_CHANGE        = 0x00,
    SPOOF_PROTECT_NONE                = 0x10,
    SPOOF_PROTECT_HIGH_SECURE        = 0x11,
    SPOOF_PROTECT_MEDIUM_SECURE        = 0x12,
    SPOOF_PROTECT_CONVENIENT_SECURE = 0x13,
} _V100_ENC_SPOOF_PROTECTION_LEVEL;


typedef struct
{
    u128 nANSOL;            // Random number generated by trusted host
    u32  nSpoofScore;        // Spoof Score (u32 max will indicate spoof is turned off)
} _V100_ENC_SPOOF_RESULT;


/**
 * This is the header structure of a BIR
 */
#pragma pack(push)
#pragma pack(1)
typedef struct
{
    unsigned char  uszHdLength[4];         // Header Size + BiometricDataBlock + 4 bytes for zero SB
    unsigned char  uszHeaderVersion[1];
    unsigned char  uszBIRDataType[1];
    unsigned char  uszFormatIDOwner[2];
    unsigned char  uszFormatIDType[2];
    unsigned char  uszQuality[1];
    unsigned char  uszPurpose[1];
    unsigned char  uszBiometricType[4];
    unsigned char  uszProductIDOwner[2];
    unsigned char  uszProductIDType[2];
    unsigned char  uszCreationYear[2];
    unsigned char  uszCreationMonth[1];
    unsigned char  uszCreationDay[1];
    unsigned char  uszCreationHour[1];
    unsigned char  uszCreationMinute[1];
    unsigned char  uszCreationSecond[1];
    unsigned char  uszSubType[1];
    unsigned char  uszExpirationDateYear[2];
    unsigned char  uszExpirationDateMonth[1];
    unsigned char  uszExpirationDateDay[1];
    unsigned char  uszSBFormatOwner[2];
    unsigned char  uszSBFormatType[2];
    unsigned char  uszIndex[16];
} _V100_ENC_BIR_HEADER;
#pragma pack(pop)

// if set in uszBIRDataType field, the uszIndex field is present
#define    BioAPI_BIR_INDEX_PRESENT    (0x80)

/**
 * The Biometric Data Block (BDB) is defined as follows:
 * BDB = DataSize (4 bytes) + Data (DataSize bytes)
 */
typedef struct
{
    u32   nDataSize;
    void* pData;
} _V100_ENC_BIO_DATA;

/**
 * This is the structure representing a Biometric Information Record (BIR)
 * to support transporting biometric data as inputs and outputs when the
 * Record Mode is set to BIR_MODE.
 */
typedef struct
{
    _V100_ENC_BIR_HEADER stHeader;
    _V100_ENC_BIO_DATA   stBiometricData;
    _V100_ENC_BIO_DATA   stSecurityBlock;
} _V100_ENC_BIR;


typedef struct
{
    u128  nANSOL;                  // A random number generated by the secure host
    u64   nSerialNumber;          // The serial number of the unit
    u64   nHardwareUniqueID;    // 8-byte serial number of the hardware secure element
    u64   nVersion;                 // [ Zeros(2) FW_REV(2) HW_SEC_ELEMENT_Rev(2) Spoof_Rev(2)]
                                // MSB to LSB
    u8    zeropad[24];
}_V100_ENC_SENSOR_INFO;


typedef struct
{
    u32   serial_number;
    u32   fw_rev;
    u32   lfd_rev;
    u32   smfw_rev;
    u32   fta_status;
    u32   bit_status;
    u32   tamper_status;
    u32   boot_validation;
    u32   authentication_status;
    u32   pd_exposure;
    u32   s1_exposure;
    u32   s2_exposure;
    u32   s3_exposure;
    u32   s4_exposure;
    u32   dark_exposure;
    u32   image_noise;
    u32   mpv_dark;
    u32   focus;
    u32   alignment;
    u32   latent_status;
    u32   retry_status;
    u32   mask_size;
    u32   key_status;
    u256  keySlotPopulated;
    u256  keySlotLocked;
    u32   reserved_1;
    u32   reserved_2;
    u32   reserved_3;
    u32   reserved_4;
    u32   reserved_5;
    u32   reserved_6;
    u32   reserved_7;
    u32   reserved_8;
}_V100_ENC_DIAG_STATUS;

#define _V100_ENC_DIAG_STATUS_BOOT_VALIDATION_FAILED    1


/*
**  Extended Key Area Key names
*/
typedef enum {
    KT_EXTKEY_VEND              =  0,        // Reserved, Not Supported
    KT_EXTKEY_CTK               =  1,        // Customer Transport key for provisioning
    KT_EXTKEY_BTK               =  2,        // Back end Transport key
    KT_EXTKEY_AES0              =  3,
    KT_EXTKEY_AES1              =  4,
    KT_EXTKEY_AES2              =  5,
    KT_EXTKEY_AES3              =  6,
    KT_EXTKEY_TDES0             =  7,        // Legacy, Not Supported
    KT_EXTKEY_TDES1             =  8,        // Legacy, Not Supported
    KT_EXTKEY_TDES2             =  9,
    KT_EXTKEY_TDES3             = 10,
    KT_EXTKEY_AES_VEND          = 11,        // Vendor AES Key.
    KT_EXTKEY_KSN_0             = 12,        // Legacy, Not Supported
    KT_EXTKEY_KSN_1             = 13,        // Legacy, Not Supported
    KT_EXTKEY_SPARE_2           = 14,        // Reserved, Not Supported
    KT_EXTKEY_SPARE_3           = 15,        // Reserved, Not Supported
    KT_EXTKEY_HOST_PUBLIC       = 16,        // Legacy, not supported.
    KT_EXTKEY_DEVICE_PUBLIC     = 17,        // Personalization of HID device
    KT_EXTKEY_DEVICE_PRIVATE    = 18,        // Personalization of HID device
    KT_EXTKEY_DEVICE_P          = 19,        // 
    KT_EXTKEY_DEVICE_Q          = 20,        // 
    KT_EXTKEY_PUBLIC_EXP        = 21,        // Legacy, Not Supported
    KT_EXTKEY_PRIVATE_EXP       = 22,        // Legacy, Not Supported
    KT_EXT_LAST,
    KT_EXT_DSK                  = 0x1000,    // Device-generated Session Key
    KT_EXT_BSK                  = 0x1001,    // Backend-generated Session Key
    KT_EXT_SP                   = 0x1002,    // Legacy, Not Supported
    KT_MSK_MKD                  = 0x2001,    // Master Key, Diversified
    KT_MSK_SK                   = 0x2003     // Session Key
}   _V100_ENC_KEY_TYPE;

/*
**  Extended Key Area Key sizes
*/
#define ENC_KEY_SIZE_AES128        16
#define ENC_KEY_SIZE_AES256        32
#define ENC_KEY_SIZE_TDES128    16
#define ENC_KEY_SIZE_TDES192    24

typedef enum {
    KM_MODE_NONE        = 0,        //
    KM_AES_256_CBC        = 1,        // AES-256 using Cipher Block Chaining (CBC) cipher mode
    KM_AES_128_CBC        = 2,        // AES-128 using Cipher Block Chaining (CBC) cipher mode
    KM_TDES_ABA_ECB        = 3,        // Two-Key TDES (ABA Key Format) using Electronic Cookbook (ECB) cipher mode
    KM_TDES_ABA_CBC        = 4,        // Two-Key TDES (ABA Key Format) using Cipher Block Chaining (CBC) cipher mode
    KM_TDES_ABC_ECB        = 5,        // Three-Key TDES (ABC Key Format) using Electronic Cookbook (ECB) cipher mode
    KM_TDES_ABC_CBC        = 6,        // Three-Key TDES (ABC Key Format) using Cipher Block Chaining (CBC) cipher mode
    KM_RSA_2048_v15        = 0x1000,    // RSA-2048 (version 1.5)
    KM_RSA_2048_v21        = 0x1001,    // RSA-2048 (version 2.1)
    KM_DUKPT_IPEK_128    = 0x1002,    // DUKPT - Initial Pin Encryption Key
    KM_DUKPT_KSN_64        = 0x1003,    // DUKPT - Key Serial Number
}    _V100_ENC_KEY_MODE;


/*
**  Capture Statistics Information
*/
typedef struct
{
    u32 nImageArea;                 // Area(in pixel units) of the Finger Segmentation Mask
                                    //   Range 0 ÔÇô max(Image Size)
    u8  nImageQuality;              // Image Quality Metric customized for MSI Imagery Based on Single Sample
                                    //   (RESERVED for FUTURE USE)
                                    //   Range Level 1 to 5 (1 = Highest 5 = Poorest)
                                    //   Predictive of Matching Performance
    u8  nImageQualityVersion;       // Algorithm versioning identifier
                                    //   (RESERVED for FUTURE USE)
    u8  nImageMinutiaCount;         // Number of Minutia for Last Template Exaction
                                    //   Range 0 ÔÇô 255
    u8  nEnrollQuality;             // Enroll on Sensor Quality Metric customized for MSI Imagery based on Two Sample Insertions
                                    //   (RESERVED for FUTURE USE)
                                    //   Range Level 1 to 5 (1 = Highest 5 = Poorest)
                                    //   Predictive of Matching Performance
    u8  nEnrollQualityVersion;      // Algorithm versioning identifier
                                    //   (RESERVED for FUTURE USE)
    // Additional reserved fields for future use
    u8  _reserved_1;
    u8  _reserved_2;
    u8  _reserved_3;
    u32 _reserved_4;
} _V100_CAPTURE_STATS;


#endif