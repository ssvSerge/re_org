/*************************************************************************************************************************
**                                                                                                                      **
** ©Copyright 2019 HID Global Corporation/ASSA ABLOY AB. ALL RIGHTS RESERVED.                                           **
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
/******************************************************************************
**
**    FILENAME
**        V100_shared_types.h
**
**    DESCRIPTION:
**        Include File for V100 Exported Interface
**
**    COMMENTS:
**        VCOM Interface data types.
**
**    REVISION INFORMATION:
**        $Id: V100_shared_types.h 34592 2019-04-03 20:33:51Z cduran $
**        $Rev: 34592 $
**        $Date: 2019-04-03 14:33:51 -0600 (Wed, 03 Apr 2019) $
**        $Author: cduran $
**
**    COPYRIGHT INFORMATION:
**        This software is proprietary and confidential.
**        By using this software you agree to the terms and conditions of the
**        associated Lumidigm Inc. License Agreement.
**
**        Lumidigm Inc Copyright 2011 All Rights Reserved.
**
******************************************************************************/

#ifndef __V100_SHAREDTYPES__
#define __V100_SHAREDTYPES__

/*
Device configuration mappings
*/
#define BASE_DEVICE_CONFIG              0x0400    // The BASE Firmware Configuration that units are shipped with from HID Factory
#define HYB02_DEVICE_CONFIG             0x1300
#define CP001_DEVICE_CONFIG             0x1400
#define VEX_DEVICE_CONFIG               0x0300
#define MSK01_DEVICE_CONFIG             0x0800
#define MSK00_DEVICE_CONFIG             0x0100


/******************************************************************************
**
**        E  N  U  M  E  R  A  T  I  O  N  S
**
******************************************************************************/

/*
**    COM/USB PORT COMMAND ENUM
*/
/*
**    COM/USB PORT COMMAND ENUM
*/
typedef enum
{
    /**** START ****/
    CMD_NONE                          = 0x00,    // none

    /**** MACRO COMMANDS ****/
    CMD_MATCH                         = 0x04,    // Match 2 Templates
    CMD_VID_STREAM                    = 0x06,    // Enable Video Streaming Mode

    /**** ATOMIC COMMANDS ****/
    CMD_LOG                           = 0x12,    // get, clear, enable or disable log
    CMD_GET_IMAGE                     = 0x41,    // Upload Native Image Buffer
    CMD_SET_IMAGE                     = 0x42,    // Download Native Image Buffer
    CMD_GET_COMPOSITE_IMAGE           = 0x43,    // Upload Composite Image Buffer
    CMD_SET_COMPOSITE_IMAGE           = 0x44,    // Download Composite Image to Buffer
    CMD_GET_TEMPLATE                  = 0x45,    // Device->Host Template Xfer
    CMD_SET_TEMPLATE                  = 0x46,    // Host->Device Template Xfer
    CMD_ARM_TRIGGER                   = 0x47,    // Trigger Acquisition
    CMD_GET_ACQ_STATUS                = 0x48,    // ACQ Status
    CMD_GET_CONFIG                    = 0x49,    // get Configuration structure
    CMD_GET_STATUS                    = 0x4A,    // get Status Structure
    CMD_GET_VERSION                   = 0x3F,
    CMD_GET_CMD                       = 0x4C,    // get Command Struct
    CMD_SET_CMD                       = 0x4D,    // set Command Struct
    CMD_GET_SERIAL_NUMBER             = 0x55,    // Get device serial number
    CMD_SET_LED                       = 0x54,    // Diagnostic Turn on LED by Number
    CMD_CONFIG_COMPORT                = 0x56,    // Configure Com port.
    CMD_SET_OPTION                    = 0x58,    // Set Option
    CMD_RESET                         = 0xC2,    // Reset Processor
    CMD_TRUNCATE_378                  = 0x59,
    /**** Spoof *****/
    CMD_MATCH_EX                      = 0x60,    // Match Minutia and/or Spoof (Universal or Enrollment)
    CMD_SPOOF_GET_TEMPLATE            = 0x61,    // Get Spoof template from Image or 378 template.
    CMD_SET_GPIO                      = 0x62,    // Set GPIO pins
    CMD_GET_GPIO                      = 0x63,
    CMD_CANCEL_OPERATION              = 0x64,    // Cancel Operation
    CMD_GET_FIR_IMAGE                 = 0x65,    // Returns last composite image in FIR format
    /**** 1:1 COMMANDS ****/
    CMD_SET_VERIFICATION_RULES        = 0x80,    // Configurations for Enrollment
    CMD_GET_VERIFICATION_RULES        = 0x81,
    CMD_ENROLL_USER                   = 0x82,
    CMD_VERIFY_USER                   = 0x83,
    CMD_DELETE_USER                   = 0x84,
    CMD_GET_DB_METRICS                = 0x85,
    CMD_FORMAT_DB                     = 0x86,
    CMD_GET_USER                      = 0x88,
    CMD_ADD_USER                      = 0x89,
    CMD_GET_OP_STATUS                 = 0x8B,
    CMD_SET_TAG                       = 0x87,
    CMD_GET_TAG                       = 0x8C,
    CMD_GET_SYSTEM_STATE              = 0x91,    // Get System State
    /* Special 1:1 Commands */
    /**** FILE MANAGEMENT ****/
    CMD_FILE_DELETE                   = 0xB5,
    /* IDENTIFICATION */
    CMD_ID_SET_API_KEY                = 0x13,    /* Sets Key for databases */
    CMD_ID_CREATE_DB                  = 0x70,    /* Create a Database */
    CMD_ID_SET_WORKING_DB             = 0x71,    /* Set working DB, must poll for completion */
    CMD_ID_VERIFY_USER_RECORD         = 0x73,    /* Verify a user record */
    CMD_ID_GET_USER_RECORD            = 0x74,    /* Get a user record from current DB */
    CMD_ID_GET_USER_RECORD_HEADER     = 0xA3,    /* Get a user record header from curr DB*/
    CMD_ID_SET_USER_RECORD            = 0x75,    /* Add a user record to current DB */
    CMD_ID_ENROLL_USER_RECORD         = 0x76,    /* Enroll, using DB parameters */
    CMD_ID_IDENTIFY_378               = 0x77,    /* Perform an ID from a 378 template */
    CMD_ID_DELETE_DB                  = 0x78,    /* Delete database */
    CMD_ID_IDENTIFY                   = 0x79,    /* Identify */
    CMD_ID_GET_DB_METRICS             = 0x7A,    /* Grab info about current set */
    CMD_ID_GET_RESULT                 = 0x7B,    /* What was result of ID?      */
    CMD_ID_DELETE_USER_RECORD         = 0x7C,    /* Delete a User or a User's Finger */
    CMD_ID_GET_SYSTEM_METRICS         = 0x7D,    /* Get Global info on databases */
    CMD_ID_GET_PARAMETERS             = 0x7E,    /* Get ID Parameters */
    CMD_ID_SET_PARAMETERS             = 0x7F,    /* Set ID Parameters */
    CMD_ID_VERIFY_378                 = 0xA0,    /* Verify 378 Template */
    CMD_VERIFY_378                    = 0x9A,    /* Verify 378 Template (against one passed in) */
    CMD_ID_GET_TEMPLATE               = 0x5A,    /* Gets the template for the Capture type requested*/
    CMD_ID_GET_IMAGE                  = 0x5B,    /* Gets the image for the Capture type requested*/
    CMD_ID_GET_SPOOF_SCORE            = 0x5C,    /* Gets the SpoofScore for the Capture type requested*/
    CMD_ID_COMMIT                     = 0x5D,    /* Commits DB changes */
    CMD_ID_PURGE_DB_ALL               = 0x5E,    /* Purges all databases */


    /* DATA LOGGING */
    CMD_SAVE_LAST_CAPTURE             = 0xA1,    /* Last Capture */
    /* FIRMWARE UPDATE */
    CMD_UPDATE_FIRMWARE               = 0xA2,    /* Update Firmware */
    CMD_SET_LICENSE_KEY               = 0x92,    // Set License Key
    CMD_SET_RECORD                    = 0x4F,    // Sends "Record" from Host->Device
    CMD_WRITE_FLASH                   = 0x52,    // Trigger Flash Update

    /* VERIFY MANY*/
    CMD_ID_VERIFY_MANY                = 0xA4,
    /*BARCODE*/
    CMD_BARCODE_GET_TEXT              = 0xA8,
    CMD_BARCODE_GET_DETAILS           = 0xA9,

    /* ENCRYPTION */
    CMD_ENC_CHALLENGE                 = 0xD0,    //
    CMD_ENC_GET_IMAGE                 = 0xD1,    //
    CMD_ENC_GET_NONCE                 = 0xD2,    //
    CMD_ENC_GET_TEMPLATE              = 0xD3,    //
    CMD_ENC_LOCK_KEY                  = 0xD4,    //
    CMD_ENC_SET_ACTIVE_KEY            = 0xD5,    //
    CMD_ENC_UNLOCK_KEY                = 0xD6,    //
    CMD_ENC_VERIFY                    = 0xD7,    //
    CMD_ENC_GET_DIAG_STATUS           = 0xD8,    //
    CMD_ENC_GET_PARAMETERS            = 0xD9,    //
    CMD_ENC_MATCH                     = 0xDA,    //

    CMD_ENC_GET_SERIAL_NUMBER         = 0xAA,
    CMD_ENC_GET_RND_NUMBER            = 0xAB,
    CMD_ENC_DECRYPT                   = 0xAC,
    CMD_ENC_SET_PARAMETERS            = 0xAD,
    CMD_ENC_VERIFYMATCH               = 0xAE,
    CMD_ENC_ENROLL                    = 0xAF,

    CMD_ENC_RETURNCAPTUREDIMAGE       = 0xE1,
    CMD_ENC_RETURNCAPTUREDBIR_IM      = 0xE2,
    CMD_ENC_GET_CAPTURE_STATS         = 0xE4,

    CMD_ENC_CAPTURE                   = 0xF0,
    CMD_ENC_RETURNCAPTUREDBIR         = 0xF1,
    CMD_ENC_FACTORY_SET_KEY           = 0xF2,
    CMD_ENC_VERIFYMATCH_RESULT        = 0xF3,
    CMD_ENC_VERIFYMATCH_MANY          = 0xF4,
    CMD_ENC_GENERATE_SESSIONKEY       = 0xF5,
    CMD_ENC_GET_KEY                   = 0xF6,
    CMD_ENC_GET_KEYVERSION            = 0xF7,
    CMD_ENC_SET_KEY                   = 0xF8,
    CMD_ENC_SET_ACTIVEKEY             = 0xF9,
    CMD_ENC_GET_SPOOF_SCORE           = 0xFA,
    CMD_ENC_RETURNCAPTUREDWSQ         = 0xFB,
    CMD_ENC_CLEAR                     = 0xFC,
    CMD_ENC_GET_SENSOR_INFO           = 0xFD,
    CMD_ENC_RETURNCAPTUREDTEMPLATE    = 0xFE,
    CMD_ENC_GENERATE_RSA_KEYS         = 0xE3,
    CMD_ENC_GET_KCV                   = 0xEE,
    CMD_ENC_SET_SESSION_KEY           = 0xEF,
    CMD_ENC_IDENTIFYMATCH             = 0xED,

    /** HID SPECIFIC OPERATIONS **/
    CMD_HID_BASE                      = 0x100,
    CMD_HID_INIT,
    CMD_HID_ENUM_CAMS,
    CMD_HID_TERMINATE,
    CMD_HID_SET_PARAM_INT,
    CMD_HID_GET_PARAM_INT,
    CMD_HID_SET_PARAM_STR,
    CMD_HID_GET_PARAM_STR,
    CMD_HID_SET_PARAM_BIN,
    CMD_HID_GET_PARAM_BIN,
    CMD_HID_SET_PARAM_LONG,
    CMD_HID_GET_PARAM_LONG,
    CMD_HID_GET_VIDEO_FRAME,
    CMD_HID_CAPTURE_IMAGE,
    CMD_HID_OPEN_CONTEXT,
    CMD_HID_CLOSE_CONTEXT,
    CMD_HID_CLOSE_OPERATION,
    CMD_HID_STOP_CMD_ASYNC,
    CMD_HID_ASYNC_EXTRACT_TEMPLATE,
    CMD_HID_ASYNC_IDENTIFY_WITH_TEMPLATE,
    CMD_HID_ASYNC_MATCH_WITH_TEMPLATE,
    CMD_HID_ASYNC_MATCH_WITH_CAPTURED,
    CMD_HID_ASYNC_VERIFY_WITH_CAPTURED,
    CMD_HID_ASYNC_IDENTIFY_WITH_CAPTURED,
    CMD_HID_ASYNC_VERIFY_WITH_TEMPLATE,
    CMD_HID_GET_INTERMEDIATE_RES,
    CMD_HID_GET_FINAL_RES,
    CMD_HID_PARSE_RES_INT,
    CMD_HID_PARSE_RES_DOUBLE,
    CMD_HID_PARSE_RES_DATA,
    CMD_HID_PARSE_RES_POINT,
    CMD_HID_PARSE_RES_IMAGE,
    CMD_HID_PARSE_MATCH_GALLERY,
    CMD_HID_DB_ADD_RECORD_WITH_CAPTURED,
    CMD_HID_DB_ADD_RECORD_WITH_TEMPLATE,
    CMD_HID_DB_GET_RECORD,
    CMD_HID_DB_LIST_RECORDS,
    CMD_HID_DB_DEL_RECORD,

    CMD_HID_FW_UPDATE = 0x0126,

    /**** ERROR ****/
    CMD_ERROR                          = 0xE00,   // Error has occurred.

    /**** END ****/
    CMD_LAST                           = 0xFFF   // Not a Command
}   _V100_COMMAND_SET;


/******************************************************************************
**            I  N  T  E  R  F  A  C  E      T  Y  P  E  S
******************************************************************************/

/******************************************************************************
**
**    Configuration Structure
**    Controlled Interface Host-V100
**    READ-ONLY
**
******************************************************************************/
typedef struct {

     unsigned short
    /*** REVISION/CONFIGURATION INFORMATION ***/
    Vendor_Id,                  // Vendor Identification
    Product_Id,                 // Product Identification
    Device_Serial_Number,       // Unique Device Serial Number
    Hardware_Rev,               // HW Revision Number             (xxx.xxx.xxx)
    Firmware_Rev,               // FW Revision Number             (xxx.xxx.xxx)
    Spoof_Rev,                  // Spoof Revision Number        (xxx.xxx.xxx)
    PreProc_Rev,                // PreProcessor Revision Number (xxx.xxx.xxx)
    Feature_Extractor_Rev,      // Feature Extractor Revision Number (xxx.xxx.xxx)
    Matcher_Rev,                // Matcher Revision Number         (xxx.xxx.xxx)
    ID_Rev,                     // Identifier Revision Number     (xxx.xxx.xxx)
    Imager_Chip_Version,        // Imager Chip Silicon Version
    Number_Image_Planes,        // Number of Image Planes in Native Image Stack
    Native_Image_Size_X,        // Number of Pixels in Column (Native)
    Native_Image_Size_Y,        // Number Pixels in Rows (Native)
    Native_DPI,                 // Native Resolution Pixel Dots/Inch
    Composite_Image_Size_X,     // Pixels in Col. Composite Processed Image (500 dpi)
    Composite_Image_Size_Y,     // Pixels in Row Composite Processed Image (500 dpi)
    Composite_DPI,              // Composite Image Resolution
    Image_Format,               // Format of Native Images (_V100_IMAGE_FORMAT_TYPE)
    Boresight_X,                // Pixel Units wrt Native Coords
    Boresight_Y,                // Pixel Units wrt Native Coords
    LED_Wavelength1,            // Nanometers (State Ordered)
    LED_Wavelength2,            // Nanometers (State Ordered)
    LED_Wavelength3,            // Nanometers (State Ordered)
    LED_Wavelength4,            // Nanometers (State Ordered)
    LED_Type1,                  // (_V100_LED_TYPE)
    LED_Type2,                  // (_V100_LED_TYPE)
    LED_Type3,                  // (_V100_LED_TYPE)
    LED_Type4,                  // (_V100_LED_TYPE)
    State1_Exposure,            // Current Exp Value from Last Image Acquisition
    State2_Exposure,
    State3_Exposure,
    State4_Exposure,
    State1_Dark_Exposure,       // Current Exp Value from Dark Frame Acquisition
    State2_Dark_Exposure,
    State3_Dark_Exposure,
    State4_Dark_Exposure,

    /*** SERVICE AVAILABILITY BOOLEANS ***/
    Phy_Interface_Available,    // Physical Interfaces Available (_V100_PHY_INTERFACE_TYPE)

    /*** SERVICE AVAILABILITY BIT FIELSDS ***/
    PreProc_Available,          // PreProcessors Available (_V100_PREPROC_TYPE)
    Feature_Extract_Available,  // Feature Extractors Available (_V100_FE_TYPE)
    Template_Match_Available,   // Template Matchers Available (_V100_FM_TYPE)
    Template_Format_Available,  // Template Formats Available _V100_TEMPLATE_FORMAT_TYPE)
    Template_ID_Available,      // Template Identification Available ( _V100_ID_TYPE)
    Presence_Detect_Available,  // Presence Detection Available (_V100_PRESENCE_DETECTION_TYPE)
    FW_Flash_Available,         // In-circuit Flash Capability Available (_V100_FLASH_TYPE)
    Spoof_Available,            // Spoof Algorithms Available (_V100_SPOOF_TYPE)
    Struct_Size;                // Size in Bytes of This Structure

    unsigned int
    pImageBuffer,               // Image Buffer Location in BF Heap
    pPDBuffer,                  // Presence Detection Buffer
    pStaticMask,                // Visible Aperture Mask
    pDarkBuffer,
    pCompositeBuffer;

     unsigned short
    Device_Serial_Number_Ex,    // Extended Serial Number data (Upper Word of SN on device - M31x and later devices only. For M30x with firmware 16440 or later)
    MfgStateFlag,               // Manufacturing State Flag (May not be in early M31x units)
    Device_Cfg_Type,            // FW configuration type
    /*** RESERVED EXPANSION ***/

    RESERVED_3,                    // RESERVED for Future Use
    RESERVED_4,                    // RESERVED for Future Use
    RESERVED_5;                    // RESERVED for Future Use

    /*** PADDING ***/
    //unsigned short

} _V100_INTERFACE_CONFIGURATION_TYPE;

/******************************************************************************
**
**    Command Structure
**    Controlled Interface btw Host-V100
**    READ/WRITE
**
******************************************************************************/
typedef struct
{
    /*** USER COMMANDS ***/
    unsigned short
    Trigger_Delay,              // (msec) Delay btw Pres. Detect and Acq
    TimeOut,                    // (sec) Timeout on Blocking Ops
    Override_Trigger,           // if TRUE, override presence detection
    Override_HeartBeat_Display, // if TRUE, Turns off HeartBeat
    Override_Default_LED,       // if TRUE, Turns off default LED behavior
    Match_Threshold,            // Threshold Score for Matching
    Set_Exposure_Mode,          // Manual or Auto Exposure Mode
    Select_PreProc,             // Bitfield to Select PreProcessor
    Select_Identifier,          // Bitfield to Select Identifier
    Select_Matcher,             // Bitfield to Select Matcher
    Select_Extractor,           // Bitfield to Select Extractor
    Select_Spoof_Model,         // Bitfield to Select Spoof Model
    Select_Template_Format,     // Template Format
    Exposure1,                  // Exposure Override for ACQ State 1 (Lines)
    Exposure2,                  // Exposure Override for ACQ State 2 (Lines)
    Exposure3,                  // Exposure Override for ACQ State 3 (Lines)
    Exposure4,                  // Exposure Override for ACQ State 4 (Lines)
    Gain1,                      // Analog Gain Override for ACQ State
    Gain2,                      // Legal values 16-64 dec
    Gain3,                      //
    Gain4,                      //
    Struct_Size,                // Size in Bytes of This Structure
    Mfg_Hook,                   //
    Select_Crop_Level,          //
    Calculate_Quality,
    FW_Class;
/*** RESERVED EXPANSION ***/
    unsigned short
    PAD_0,
    PAD_1,
    PAD_2,
    PAD_3,
    PAD_4,
    PAD_5;
} _V100_INTERFACE_COMMAND_TYPE;


/******************************************************************************
**
**    Status Structure
**    Controlled Interface btw Host-V100
**    READ ONLY
**
******************************************************************************/
typedef struct {
    unsigned short
    General_Error,                // cast as _V100_General_Error_Codes
    Service_Error,                // cast as _V100_Service_Error_Codes
    BIT_Error,                    // cast as _V100_BIT_ERROR_CODES
    COM_Error,                    // Number of I2C Interface Errors
    USB_Error,                    // Number of USB Data Validation Errors
    Flash_Updater_Status,         // Busy Indicator
    Struct_Size,                  // Size in Bytes of This Structure
    Latent_Detected,              //
    Boot_Error,                   // Boot errors detected in Boot::Run()
    /*** RESERVED EXPANSION ***/
    RESERVED_2,                   // RESERVED for Future Use
    RESERVED_3,                   // RESERVED for Future Use

    /*** PADDING ***/
    PAD_0,
    PAD_1,
    PAD_2,
    PAD_3,
    PAD_4;

} _V100_INTERFACE_STATUS_TYPE;

// Serial Protocol Definitions

#define PROTOCOL_VCOM_SIMPLE     0x80000000
#define MAX_SIZE_META_DATA       255
#define MAX_PACKET_SIZE          ((1024+256)*1024)
#define MAX_378_TEMPLATE_SIZE    2048

typedef unsigned int     uint;
typedef unsigned char    uchar;
typedef void*            HANDLE;

typedef struct {
    // USB Specific
    uint             DeviceNumber;        // Device Number 0=default
    HANDLE           hRead;               // USB device read handle
    HANDLE           hWrite;              // USB device write handle
    // Baud Rate Specific
    uint             nCOMIndex;
    uint             nBaudRate;
    // Other
    void*            hInstance;
}   V100_DEVICE_TRANSPORT_INFO;

typedef enum {
    ACQ_DONE,
    ACQ_PROCESSING,
    ACQ_BUSY,
    ACQ_TIMEOUT,
    ACQ_NO_FINGER_PRESENT,
    ACQ_MOVE_FINGER_UP,
    ACQ_MOVE_FINGER_DOWN,
    ACQ_MOVE_FINGER_LEFT,
    ACQ_MOVE_FINGER_RIGHT,
    ACQ_FINGER_POSITION_OK,
    ACQ_CANCELLED_BY_USER,
    ACQ_LATENT_DETECTED,
    ACQ_FINGER_PRESENT,
    ACQ_SPOOF_DETECTED,
    ACQ_ERROR_MATCH,
    ACQ_ERROR_IMAGE,
    ACQ_ERROR_SYSTEM,
    ACQ_ERROR_PARAMETER,
    ACQ_NOOP = 99,                        // No status to report.
} _V100_ACQ_STATUS_TYPE;

typedef enum {
    STATUS_OK                        = 0x00,
    ERROR_UID_EXISTS                = 0x01,
    ERROR_ENROLLMENT_QUALIFICATION  = 0x02,
    ERROR_UID_DOES_NOT_EXIST        = 0x03,
    ERROR_DB_FULL                    = 0x04,
    ERROR_QUALIFICATION                = 0x05,
    ERROR_DEV_TIMEOUT                = 0x06,
    ERROR_USER_CANCELLED            = 0x07,
    ERROR_SPOOF_DETECTED            = 0x08,
    ERROR_DB_EXISTS                    = 0x09,
    ERROR_DB_DOES_NOT_EXIST            = 0x0A,
    /* Identification Codes */
    ERROR_ID_DB_TOO_LARGE            = 0x10,
    ERROR_ID_DB_EXISTS                = 0x11,
    ERROR_ID_USER_EXISTS            = 0x12,
    ERROR_ID_USER_NOT_FOUND            = 0x13,
    STATUS_ID_MATCH                    = 0x14,
    STATUS_ID_NO_MATCH                = 0x15,
    ERROR_ID_PARAMETER                = 0x16,
    ERROR_ID_GENERAL                = 0x17,
    ERROR_ID_FILE                    = 0x18,
    ERROR_ID_NOT_INITIALIZED        = 0x19,
    ERROR_ID_DB_FULL                = 0x1A,
    ERROR_ID_DB_DOESNT_EXIST        = 0x1B,
    ERROR_ID_DB_NOT_LOADED            = 0x1C,
    ERROR_ID_RECORD_NOT_FOUND        = 0x1D,
    ERROR_ID_FS                        = 0x1E,
    ERROR_ID_CREATE_FAIL            = 0x1F,
    ERROR_ID_INTERNAL                = 0x20,
    ERROR_ID_MEM                    = 0x21,
    STATUS_ID_USER_FOUND            = 0x22,
    STATUS_ID_USER_NOT_FOUND        = 0x23,
    ERROR_ID_USER_FINGERS_FULL        = 0x24,
    ERROR_ID_USER_MULTI_FINGERS_NOT_FOUND     = 0x25,
    ERROR_ID_USERS_FULL                        = 0x26,
    ERROR_ID_OPERATION_NOT_SUPPORTED         = 0x27,
    ERROR_ID_NOT_ENOUGH_SPACE                 = 0x28,
    ERROR_ID_DUPLICATE                         = 0x29,
    /* Capture */
    ERROR_CAPTURE_TIMEOUT                    = 0x30,
    ERROR_CAPTURE_LATENT                    = 0x31,
    ERROR_CAPTURE_CANCELLED                    = 0x32,
    ERROR_CAPTURE_INTERNAL                    = 0x33,
    /* Update Firmware */
    ERROR_UPDATE_MEMORY_ERROR                 = 0x40,
    ERROR_UPDATE_DECRYPTION_FAIL            = 0x41,
    ERROR_UPDATE_FIRMWARE_VERSION_ERROR     = 0x42,
    ERROR_UPDATE_FLASH_WRITE_ERROR            = 0x43,
    ERROR_UPDATE_INVALID_TYPE                = 0x44,
    ERROR_UPDATE_FORMAT_ERROR                = 0x45,
    ERROR_UPDATE_FIRMWARE_SIZE_ERROR        = 0x46,
    ERROR_UPDATE_RESTORE_FAIL                = 0x47,
    ERROR_UPDATE_FIRMWARE_INVALID            = 0x48,
    ERROR_UPDATE_SPOOF_WRITE_FAIL            = 0x49,
    ERROR_UPDATE_SE_FAIL                    = 0x4A,
    ERROR_UPDATE_OTP_INVALID                = 0x4B,
    ERROR_UPDATE_TX_KEY_FAIL                = 0x4C,
    ERROR_UPDATE_CALIBRATION_WRITE_FAIL        = 0x4D,
    ERROR_UPDATE_FS_RESTORE_FAIL             = 0x4E,
    ERROR_UPDATE_SYSTEM_INFO_WRITE_FAIL        = 0x4F,
    /* Generic Crypto Error */
    ERROR_CRYPTO_ERROR                         = 0x50,
    /* NOOP */
    STATUS_NO_OP                            = 0xFF,
} _V100_OP_ERROR;

typedef enum
{
    NO_OP,
    OP_IN_PROGRESS,
    OP_ERROR,
    OP_COMPLETE,
} _V100_OP_MODE;

typedef enum
{
    V100_MATCH_RESULT_BEGIN        = 0x00,
    MATCH                        = 0x01,
    NO_MATCH                    = 0x02,
    V100_MATCH_RESULT_END        = 0x03,
} _V100_MATCH_RESULT;

typedef struct
{
    _V100_COMMAND_SET        eMacroCommand;
    _V100_OP_MODE            nMode;
    unsigned int            nParameter;
    _V100_ACQ_STATUS_TYPE    eAcqStatus;
} _V100_OP_STATUS;

/*
** DB Initialization structure
**
**    uint nGroupID                - Which group to use
**    uint nUsers                    - Maximum Users this database can store
**    uint nFingersPerUser        - Fingers per User in this database
**    uint nInstancesPerFinger    - Impressions per finger in the database
**    uint nFlags                    - Flags for DB  Init
**    uint nReserved_0            - Reserved for future use
**    uint nReserved_1            - Reserved for future use
**    uint nReserved_2            - Reserved for future use
**
*/

/*
**  Flags for the "nFlags" member of _MX00_DB_INIT_STRUCT
*/

#define        DB_INIT_FLAG_ID_DEFAULT            0x0001        // default behavior. Identification and Verification DB
#define        DB_INIT_FLAG_VERIFY_ONLY        0x0004        // Verify-only database.
#define        DB_INIT_FLAG_ID_ENHANCED        0x0010        // Enhanced templates with 1:N search capabilies.. If used this should be ORed with one of the above two flags

typedef struct
{
    uint nGroupID;
    uint nUsers;
    uint nFingersPerUser;
    uint nInstancesPerFinger;
    uint nFlags;
    uint nReserved_0;
    uint nReserved_1;
    uint nReserved_2;
} _MX00_DB_INIT_STRUCT;

typedef struct
{
    uint nGroupID;
    uint nMaxUsers;
    uint nFingersPerUser;
    uint nInstancesPerFinger;
    int  nFlags;
    int  nCurEnrolledUserFingers;
    int  nCurEnrolledUsers;
} _MX00_DB_METRICS;

/*
** User Record Structure
**
** Each user record corresponds to a User and finger which
** enrolled in a system.  Each _MX00_ID_USER_RECORD is followed
** by _MX00_ID_USER_RECORD.nInstances number of _MX00_TEMPLATE_INSTANCE
** records which contain the template instance information.
**
**
*/

typedef struct
{
    uint nGroupID;
    char szUserID[32];
    uint nFinger;
    uint nInstances;
} _MX00_ID_USER_RECORD;


typedef struct
{
    uchar    p378Template[MAX_378_TEMPLATE_SIZE];
} _MX00_TEMPLATE_INSTANCE;

typedef struct
{
    _V100_OP_ERROR    eLastStatus;
    char            szUserID[32];
    uint            nFinger;
    int                nIDScore;
    int                nM1Score;
    int                nM2Score;
    int                nSpoofScore;
    int                nIDTime;
    int                nC1Time;
    int                nC2Time;
} _MX00_ID_RESULT;

typedef struct
{
    uint nGroupID;
    uint nUsers;
    uint nFingersPerUser;
    uint nInstancesPerFinger;
    uint nNumUsersPopulated;
    uint nBytesDBSize;
} _MX00_DB_INFO;


typedef struct
{
    uint nSecurityLevel;
    uint nFlags;
    uint nParm2;
    uint nParm3;
    uint nParm4;
} _MX00_ID_PARAMETERS;

// Flags for _MX00_ID_PARAMETERS and _V100_VERIFICATION_RULES "nFlags" member

#define FLAG_OVERWRITE_EXISTING_RECORDS        0x0001    // If Enroll, overwrite records. Supported for _V100_VERIFICATION_RULES only
#define FLAG_FAIL_ENROLL_ON_SPOOF            0x0002    // Fail Enroll, if Spoof is detected.
#define FLAG_FAIL_VERIFY_ON_SPOOF            0x0004    // Fail Verify, if Spoof is detected.
#define FLAG_FAIL_IDENTIFY_ON_SPOOF            0x0100    // Fail Identify, if Spoof is detected.
#define FLAG_ENROLLMENT_QUALIFICATION        0x0008    // Perform inter-enrollment matching.
#define FLAG_VERIFICATION_MUST_MATCH_ONE    0x0010    // Must only match one enrolled template to verify.
#define FLAG_VERIFICATION_MUST_MATCH_ALL    0x0020    // Must match all enrolled templates to verify.
#define FLAG_ENROLLMENT_APPEND_RECORD        0x0040    // Appends enrollment to UID given. Supported for _V100_VERIFICATION_RULES only
#define FLAG_FAIL_ENROLL_ON_DUPLICATE        0x0080    // Fail Enroll, if duplicate is detected.

/*
** User Record Structure.
*/

typedef struct
{
    unsigned int  UID;                                // User ID
    unsigned int  nRecords;                            // Number of Records
    unsigned int  nSizeRecord;                        // Size of Record Data
    unsigned int  nSizeMetaData;                    // Size of Meta Data
    unsigned char MetaData[MAX_SIZE_META_DATA];        // Meta data
} _V100_USER_RECORD;

/*
** Save Debug Structure
*/

typedef struct
{

} _MX00_SAVE_CAPTURE;


/*
** Verification/Enrollment settings structure.
*/
// New Rules
//
// 1. IF FLAG_ENROLLMENT_QUALIFICATION is on, and nNumberOfImpressions = 1, an additional capture is done, and verified
//      against enrollment template, and if succesful, the enrollment template is stored.   If nNumberOfImpressions > 1, then the subsequent template(s) are compared
//    to first.
// 2. If nInterEnrollmentThreshold or nVerificationThreshold are set to 0, return score instead of Match/NoMatch
// 3. Add functionality to DB_METRICS to record last spoof score, last match score
//

typedef struct
{
// Number of Impressions to take per enrollment
    int nNumberOfImpressions;
// Maxmimum Template Size, per impression
    int nMaxTemplateSizePerImpression;
// Inter-Enrollment Threshold
    int nInterEnrollmentThreshold;
// Verification Threshold
    int nVerificationThreshhold;
// Spoof Threshold
    int nSpoofThreshold;
// Inter-Impression Time-Out
    int nInterImpressionTimeOutSeconds;
// Overwrite Existing Records?
    int nFlags;
} _V100_VERIFICATION_RULES;


/*
** Database Metrics Structure.
*/

typedef struct
{
    int nNumUserRecordsOnDrive;            // Number of User Records on Drive
    int nNumUserRecordsFreeEstimate;    // Estimate on number of user records that can still be enrolled.
    int nLastMatchScore;                // Last Match Score
    int nLastSpoofScore;                // Last Spoof Score
} _V100_DB_METRICS;

/*
**    Types of MACRO CMD Input Arguements
*/
typedef enum
{
    CMD_EXPOSURE_NONE = 0x0000,
    CMD_AUTO_EXPOSURE,
    CMD_MANUAL_EXPOSURE,
    CMD_EXPOSURE_LAST
} _V100_EXPOSURE_MODES_TYPE;

/*
**    Types of Record
*/

typedef enum
{
    RECORD_TYPE_NONE        = 0x0000,
    RECORD_TYPE_FLASH        = 0x0001,
    RECORD_TYPE_USER_ID        = 0x0002,
    RECORD_TYPE_CONFIG        = 0x0004,
    RECORD_TYPE_CALIBRATION = 0x0008,
} _V100_RECORD_TYPE;

/*
**    Types of MACRO CMD Input Arguements
*/

typedef unsigned short _V100_MACRO_ARGUMENT_TYPE;

#define    MACRO_NONE                      0x0000
#define    MACRO_TEMPLATE                 0x0001
#define    MACRO_COMPOSITE_IMAGE        0x0002
#define    MACRO_USER_ID                0x0010

/*
**    Types of Image Preprocessing Available
*/
typedef enum
{
    PP_NONE    = 0x0000,
    PP_A3,
    PP_B7,
    PP_B9,
    PP_LAST
} _V100_PREPROC_TYPE;

/*
**    Trigger modes
*/
typedef enum
{
    TRIGGER_OFF = 0x00,
    TRIGGER_ON  = 0x01,
    TRIGGER_MOTION_ON = 0x02,
    TRIGGER_MOTION_OFF = 0x03,
    TRIGGER_FINGER_DETECT = 0x04,
    TRIGGER_BARCODE_ON = 0x05,
    TRIGGER_HYBRID_ON = 0x06,
    CANCEL_VERIFICATION = 0x99,
    TRIGGER_LAST,
} _V100_TRIGGER_MODE;

/*
**    Types of Image Feature Extraction Available
*/
typedef enum
{
    FE_NONE    = 0x0000,
    FE_LDI,
    FE_NEC,
    FE_378,
    FE_LAST
} _V100_FE_TYPE;

/*
**    Types of Image Feature Matchers Available
*/
typedef enum
{
    FM_NONE    = 0x0000,
    FM_LDI,
    FM_NEC,
    FM_378,
    FM_LAST
} _V100_FM_TYPE;

/*
**    Types of Template Identification Algs Available
*/
typedef enum
{
    ID_NONE    = 0x0000,
    ID_LDI,
    ID_LAST
} _V100_ID_TYPE;

/*
**    Types of Flash Firmware Available
*/
typedef enum
{
    FLASH_NONE = 0x0000,        // not available
    FLASH_VIA_USB,                // update via usb port
    FLASH_VIA_COM,                // update via com port
    FLASH_LAST
} _V100_FLASH_TYPE;

/*
**    Types of Spoof Algs Available
*/
typedef enum
{
    SPOOF_NONE    = 0x0000,    // not available
    SPOOF_UNIVERSAL,        // only Universal spoof
    SPOOF_ENROLLMENT,        // only Enrollment spoof
    SPOOF_FULL,                // both Universal and Enrollment spoof
    SPOOF_LAST
} _V100_SPOOF_TYPE;


/*
**    Types of Spoof Models Available
*/
typedef enum
{
    SPOOF_MODEL_NONE = 0x0000,    // not available
    SPOOF_MODEL_001,            // current spoof model
    //SPOOF_MODEL_002,            // next spoof model example
    SPOOF_MODEL_LAST
} _V100_SPOOF_MODEL;


/*
**    Types of template modes available.
**    Selected using CMD_SET_OPTION
*/

typedef enum
{
    TEMPLATE_NONE                    = 0x0000,
    TEMPLATE_ANSI_378              = 0x0001,
    TEMPLATE_ISO_NORMAL            = 0x0002,
    TEMPLATE_PROP                = 0x0003,
} _V100_TEMPLATE_MODE;

/*
**    LED CONTROL
*/

typedef enum
{
    ALL_OFF  =            0x0000,
    GREEN_ON =            0x0001,
    GREEN_OFF=            0x0002,
    RED_ON   =            0x0004,
    RED_OFF  =            0x0008,
    GREEN_CYCLE_ON =    0x0010, // legacy
    GREEN_CYCLE_OFF =    0x0020, // legacy
    RED_CYCLE_ON =        0x0040, // legacy
    RED_CYCLE_OFF =        0x0080, // legacy
    PD_ON    =            0x0100, // legacy
    PD_OFF    =            0x0200    // legacy
} _V100_LED_CONTROL;

/*
**    Types of Images Available
*/

typedef enum
{
    IMAGE_NONE    = 0x0000,        // not available
    IMAGE_NATIVE_1,                // 1
    IMAGE_NATIVE_2,                // 2
    IMAGE_NATIVE_3,                // 3
    IMAGE_NATIVE_4,                // 4
    IMAGE_NATIVE_MASK,            // 5
    IMAGE_PD,                    // 6
    IMAGE_DARK_FRAME,            // 7
    IMAGE_SCALED_1,                // 8
    IMAGE_SCALED_2,                // 9
    IMAGE_SCALED_3,                // 10
    IMAGE_SCALED_4,                // 11
    IMAGE_SCALED_MASK,            // 12
    IMAGE_COMPOSITE,            // 13
    IMAGE_VID_STREAM,            // 14
    IMAGE_DARK_FRAME_STATE_1,    // 15
    IMAGE_DARK_FRAME_STATE_2,    // 16
    IMAGE_QUALITY,                // 17
    IMAGE_WSQ,                    // 18

    IMAGE_LAST
} _V100_IMAGE_TYPE;


/*
**    Types of Template Formats Available
*/
typedef enum
{
    TEMPLATE_FORMAT_NONE = 0x0000,    // None Available
    TEMPLATE_FORMAT_LDI,            // LDI (Compatible with WDW Format)
    TEMPLATE_FORMAT_NEC,            // NEC Native Format
    TEMPLATE_FORMAT_ANSI,            // ANSI-378 Format
    TEMPLATE_FORMAT_LAST
} _V100_TEMPLATE_FORMAT_TYPE;

typedef enum
{
    SPOOF_TEMPLATE_UNIVERSAL_V0  = 0x0000,
    SPOOF_TEMPLATE_ENROLLMENT_V0 = 0x0001,
    SPOOF_TEMPLATE_LAST
} _V100_SPOOF_TEMPLATE_FORMAT_TYPE;

/*
**    Types of FIR Record
*/

typedef enum
{
    LUMI_FIR_ISO        = 0x0000,    // ISO/IEC 19794-4:2005
    LUMI_FIR_ANSI        = 0x0001,    // ANSI/INCITS 381-2004
    LUMI_FIR_LAST        = 0x00FF
} _V100_FIR_RECORD_TYPE;

#define ISO_19794_4_HDR_SIZE 46        // FIR_ISO header size
#define ANSI_381_HDR_SIZE    50        // FIR_ANSI header size

/*
** Finger/Palm Position to use with FIR record
*/
typedef enum
{
    LUMI_FINGER_POSITION_UNKNOWN_FINGER             = 0,
    LUMI_FINGER_POSITION_RIGHT_THUMB                = 1,
    LUMI_FINGER_POSITION_RIGHT_INDEX_FINGER         = 2,
    LUMI_FINGER_POSITION_RIGHT_MIDDLE_FINGER        = 3,
    LUMI_FINGER_POSITION_RIGHT_RING_FINGER          = 4,
    LUMI_FINGER_POSITION_RIGHT_LITTLE_FINGER        = 5,
    LUMI_FINGER_POSITION_LEFT_THUMB                 = 6,
    LUMI_FINGER_POSITION_LEFT_INDEX_FINGER          = 7,
    LUMI_FINGER_POSITION_LEFT_MIDDLE_FINGER         = 8,
    LUMI_FINGER_POSITION_LEFT_RING_FINGER           = 9,
    LUMI_FINGER_POSITION_LEFT_LITTLE_FINGER         = 10,
    LUMI_FINGER_POSITION_PLAIN_RIGHT_FOUR_FINGERS   = 13,
    LUMI_FINGER_POSITION_PLAIN_LEFT_FOUR_FINGERS    = 14,
    LUMI_FINGER_POSITION_PLAIN_THUMBS               = 15,
    LUMI_PALM_POSITION_UNKNOWN_PALM                 = 20,
    LUMI_PALM_POSITION_RIGHT_FULL_PALM              = 21,
    LUMI_PALM_POSITION_RIGHT_WRITERS_PALM           = 22,
    LUMI_PALM_POSITION_LEFT_FULL_PALM               = 23,
    LUMI_PALM_POSITION_LEFT_WRITERS_PALM            = 24,
    LUMI_PALM_POSITION_RIGHT_LOWER_PALM             = 25,
    LUMI_PALM_POSITION_RIGHT_UPPER_PALM             = 26,
    LUMI_PALM_POSITION_LEFT_LOWER_PALM              = 27,
    LUMI_PALM_POSITION_LEFT_UPPER_PALM              = 28,
    LUMI_PALM_POSITION_RIGHT_OTHER                  = 29,
    LUMI_PALM_POSITION_LEFT_OTHER                   = 30,
    LUMI_PALM_POSITION_RIGHT_INTERDIGITAL           = 31,
    LUMI_PALM_POSITION_RIGHT_THENAR                 = 32,
    LUMI_PALM_POSITION_RIGHT_HYPOTHENAR             = 33,
    LUMI_PALM_POSITION_LEFT_INTERDIGITAL            = 34,
    LUMI_PALM_POSITION_LEFT_THENAR                  = 35,
    LUMI_PALM_POSITION_LEFT_HYPOTHENAR              = 36,
    LUMI_FINGER_PALM_POSITION_LAST                  = 37
}   _V100_FINGER_PALM_POSITION;

/*
**    Types of Finger Presence Detection
*/
typedef enum {
    PD_NONE = 0x0000,
    PD_SW_PRESENCE_DETECTION,
    PD_HW_PRESENCE_DETECTION,
    PD_SW_WITH_QUALITY_CHECK,
    PD_SW_WITH_QC_AND_SPOOF,
    PD_SENSITIVITY_LOW,
    PD_SENSITIVITY_MEDIUM,
    PD_SENSITIVITY_HIGH,
    PD_LAST
} _V100_PRESENCE_DETECTION_TYPE;

/*
**    V100 Physical Interfaces Available
*/
typedef enum {
    PHY_NONE                = 0x0000,    // None
    PHY_USB_AVAILABLE        = 0x0001,    // USB Interface Available
    PHY_ETHERNET_AVAILABLE  = 0x0002,    // Ethernet Interface Available
    PHY_COM232_AVAILABLE      = 0x0004,    // RS-232 Interface Available
    PHY_Weigand_AVAILABLE      = 0x0008,    // Weigand Interface Available
    PHY_COM485_AVAILABLE      = 0x0010,    // RS-485 Interface Available
    PHY_LAST
} _V100_PHY_INTERFACE_TYPE;

/*
**    Multi-Spectral LED Identifiers
*/
typedef enum {
    MS_LED_TIR    = 0x0001,
    MS_LED_NONP_1,
    MS_LED_NONP_2,
    MS_LED_POL_1,
    MS_LED_DARK,
    MS_LED_NONE
} _V100_LED_TYPE;

/*
**    AEC/AGC Type Selections
*/
typedef enum
{
    AEC_AGC_OFF = 0x000,
    AEC_ENABLE  = 0x0001,
    AGC_ENABLE  = 0x0002
} _V100_GAIN_TYPES;

/*
**    Image Format
*/
typedef enum
{
    IMAGE_FORMAT_NONE = 0x0000,
    IMAGE_FORMAT_BAYER,
    IMAGE_FORMAT_BW,
    IMAGE_FORMAT_RGB,
    IMAGE_FORMAT_COMPOSITE,
    IMAGE_FORMAT_LAST
} _V100_IMAGE_FORMAT_TYPE;


#define GPIO_0                        0x01
#define GPIO_1                        0x02
#define GPIO_2                        0x04
#define GPIO_3                        0x08

/*
**    COM ERROR CODES
*/
typedef enum
{
    COM_OK                             = 0x0000,
    COM_NOT_SUPPORTED                 = 0x0001,
    COM_FEATURE_LOCKED                = 0x0002,
    COM_SOH_ERROR                    = 0x0004,
    COM_NUM_BYTES_ERROR               = 0x0008,
    COM_CMD_ERROR                   = 0x0010,
    COM_INVALID_OPAQUE_DATA_FIELD    = 0x0020,
    COM_INVALID_CRC                    = 0x0040,
    COM_MESSAGE_FRAGMENT            = 0x0080,
    COM_TIMEOUT                        = 0x0100,
    COM_FINGER_QUALITY_FAIL            = 0x0200,
    COM_FINGER_SPOOF_FAIL            = 0x0400,
    COM_USERID_NOT_AVAILABLE        = 0x0800,
    COM_LAST
} _V100_COM_ERROR_CODES;



/*
**    Status Structure Error Codes
*/
typedef enum
{
    SRV_OK                  = 0x0000,
    SRV_IM_ERROR            = 0x0001,    // Interrupt Manager
    SRV_EBIU_ERROR          = 0x0002,    // EBIU
    SRV_PM_ERROR            = 0x0004,    // Power Manager
    SRV_DCB_ERROR           = 0x0008,    // Deferred Callback Manager
    SRV_DMA_ERROR           = 0x0010,    // DMA Manager
    SRV_INT_ERROR           = 0x0020,    // Interrupt Hook
    SRV_DEV_ERROR           = 0x0040,    // Device Driver
    SRV_PF_ERROR            = 0x0080,    // Program Flag Service
    SRV_CAM_INIT_ERROR      = 0x0100,    // Camera Comm
    SRV_UART_ERROR          = 0x0200,    // UART Error
    SRV_USB_ERROR           = 0x0400,    // USB Error
    SRV_PAD_ERROR           = 0x0800,    // Data Struct Pad MisAlignment
    SRV_CRYPTO_ERROR        = 0x1000,    // Cant Validate Encryption
    SRV_FLASH_ERROR         = 0x2000,    // Flash Driver Error
    SRV_MEM_ALLOC_ERROR     = 0x4000,    // malloc Failure
    SRV_NXP_ERROR           = 0x8000,    // NXP Chipset Failure
    SRV_LAST
} _V100_SERVICE_ERROR_CODES;

/******************************************************************************
**
**    Status Structure Boot Error Codes
**
******************************************************************************/
typedef enum
{
     STATUS_BOOT_ERROR_NONE              = 0x0000,
     STATUS_BOOT_ERROR_BSP               = 0x0001,
     STATUS_BOOT_ERROR_DM                = 0x0002,
     STATUS_BOOT_ERROR_CAMERA            = 0x0004,
     STATUS_BOOT_ERROR_PM                = 0x0008,
     STATUS_BOOT_ERROR_VDK               = 0x0010,
     STATUS_BOOT_ERROR_TAMPER            = 0x0020,
     STATUS_BOOT_ERROR_OTP               = 0x0040,
     STATUS_BOOT_ERROR_CONFIG_INVALID   = 0x0080,
     STATUS_BOOT_ERROR_FS_INIT          = 0x0100,
     STATUS_BOOT_ERROR_CM                  = 0x0200,
     STATUS_BOOT_ERROR_FUTURE_4          = 0x0400,
     STATUS_BOOT_ERROR_FUTURE_5          = 0x0800,
     STATUS_BOOT_ERROR_FUTURE_6          = 0x1000,
     STATUS_BOOT_ERROR_FUTURE_7          = 0x2000,
     STATUS_BOOT_ERROR_FUTURE_8          = 0x4000,
     STATUS_BOOT_ERROR_FUTURE_9          = 0x8000,

} _V100_BOOT_ERROR_CODES;

/*
**    General Error Codes
*/
typedef enum {
    GEN_OK                          = 0x0000,
    GEN_ENCRYPTION_FAIL             = 0x0001,    // Encryption Fail
    GEN_DECRYPTION_FAIL             = 0x0002,    // Decryption Fail
    GET_PD_INIT_FAIL                = 0x0004,    // Presence Detect Fail
    PD_HISTOGRAM_FAIL               = 0x0008,    // Histogram Fail
    INVALID_ACQ_MODE                = 0x0010,    // Mode Fail
    BURNIN_THREAD_FAIL              = 0x0020,    // thread creation error
    UPDATER_THREAD_FAIL             = 0x0040,    // thread creation error
    GEN_ERROR_CONNECTION_REFUSED    = 0x0079,    // Embedded USB Driver supports only one client app connection
    GEN_ERROR_START                 = 0x0080,
    GEN_ERROR_ACCESS_DENIED,
    GEN_ERROR_KEY_NOT_FOUND,
    GEN_ERROR_PROCESSING,
    GEN_ERROR_VERIFY,
    GEN_ERROR_MATCH,
    GEN_ERROR_INTERNAL,
    GEN_ERROR_INVALID_CMD,
    GEN_ERROR_PARAMETER,
    GEN_NOT_SUPPORTED,
    GEN_INVALID_ARGUEMENT,
    GEN_ERROR_TIMEOUT,
    GEN_ERROR_LICENSE,
    GEN_ERROR_COMM_TIMEOUT,
    GEN_FS_ERR_CD,
    GEN_FS_ERR_DELETE,
    GEN_FS_ERR_FIND,
    GEN_FS_ERR_WRITE,
    GEN_FS_ERR_READ,
    GEN_FS_ERR_FORMAT,
    GEN_ERROR_MEMORY,
    GEN_ERROR_RECORD_NOT_FOUND,
    GEN_VER_INVALID_RECORD_FORMAT,
    GEN_ERROR_DB_FULL,
    GEN_ERROR_INVALID_SIZE,
    GEN_ERROR_TAG_NOT_FOUND,
    GEN_ERROR_APP_BUSY,
    GEN_ERROR_DEVICE_UNCONFIGURED,
    GEN_ERROR_TIMEOUT_LATENT,
    GEN_ERROR_DB_NOT_LOADED,
    GEN_ERROR_DB_DOESNOT_EXIST,
    GEN_ERROR_ENROLLMENTS_DO_NOT_MATCH,
    GEN_ERROR_USER_NOT_FOUND,
    GEN_ERROR_DB_USER_FINGERS_FULL,
    GEN_ERROR_DB_USERS_FULL,
    GEN_ERROR_USER_EXISTS,
    GEN_ERROR_DEVICE_NOT_FOUND,
    GEN_ERROR_DEVICE_NOT_READY,
    GEN_ERROR_PIPE_READ,
    GEN_ERROR_PIPE_WRITE,
    GEN_ERROR_SENGINE_SHUTTING_DOWN,
    GEN_ERROR_SPOOF_DETECTED,
    GEN_ERROR_DATA_UNAVAILABLE,
    GEN_ERROR_CRYPTO_FAIL,
    GEN_ERROR_CAPTURE_CANCELLED,
    GEN_ERROR_ENROLLMENT_INCOMPLETE,
    GEN_ERROR_JAG_GENERAL,
    GEN_ERROR_JAG_BUSY,
    GEN_ERROR_JAG_NOT_FOUND,
    GEN_ERROR_JAG_ALREADY_PRESENT,
    GEN_ERROR_JAG_NOT_INITIALIZED,
    GEN_ERROR_JAG_ALREADY_INITIALIZED,
    GEN_ERROR_JAG_ARGUMENT_NULL,
    GEN_ERROR_JAG_ARGUMENT_INVALID,
    GEN_ERROR_JAG_NOT_IMPLEMENTED,
    GEN_ERROR_JAG_ACCESS_DENIED,
    GEN_ERROR_JAG_HW_ERROR,
    GEN_ERROR_JAG_COMM_ERROR,
    GEN_ERROR_JAG_PREREQUISITES_NOT_MET,
    GEN_ERROR_JAG_TIMEOUT,
    GEN_ERROR_JAG_ABORTED,
    GEN_ERROR_JAG_UNKNOWN_USER,
    GEN_ERROR_JAG_ALLOCATION_FAILED,
    GEN_ERROR_FWUPDATE_GENERAL   = 191,
    GEN_ERROR_FWUPDATE_FILEOPEN  = 192,
    GEN_ERROR_FWUPDATE_FILEWRITE = 193,
    GEN_LAST
}   _V100_GENERAL_ERROR_CODES;

typedef _V100_GENERAL_ERROR_CODES V100_ERROR_CODE;


/*
**    Built-In-Test Result Error Codes
*/
typedef enum
{
    BIT_OK            = 0x0000,        // Ok
    BIT_CAMERA_FAIL    = 0x0001,        // Camera Test Fail
    BIT_MEM_FAIL    = 0x0002,        // Memeory Test Fail
    BIT_LAST
} _V100_BIT_ERROR_CODES;



/*
**    Flash Update Status Codes
*/
typedef enum
{
    FLASH_READY,
    FLASH_BUSY,
    FLASH_COMPLETE,
    FLASH_ERROR
} _V100_FLASH_STATUS_TYPE;

/*
**    Download Data Record
**  FORMAT <Header> <Payload>
*/
typedef enum
{
    DATA_RECORD_OK = 0x0000,
    DATA_RECORD_ERROR_UNSUPPORTED,
    DATA_RECORD_ERROR_CRC,
    DATA_RECORD_ERROR_GENERAL,
    DATA_RECORD_THREAD_ERROR,
    DATA_RECORD_LAST
} _V100_DATA_RECORD_ERROR_TYPE;

/*
**    Record Type for Firmware
**
*/

typedef enum
{
    DATA_RECORD_TYPE_FIRMWARE = 0x10,
    DATA_RECORD_TYPE_DATABASE = 0x20,
    DATA_RECORD_TYPE_CALFILE  = 0x40
} _V100_DATA_RECORD_TYPE;

/*
**    Record header for CMD_SET_RECORD
*/

typedef struct
{
    unsigned int
    RecType,                    // _V100_DATA_RECORD_TYPE
    RecSize,                    // Sizeof RECORD in Bytes
    RecCRC;                       // CRC-32
} _V100_RECORD_HEADER;

/*
**    Mode for CMD_VID_STREAM
*/

typedef enum
{
    VID_STREAM_OFF = 0x0,
    VID_STREAM_ON  = 0x1,
} _V100_VID_STREAM_MODE;

/*
**    Latent detection mode.
**    Selected using CMD_SET_OPTION
*/
typedef enum
{
    LATENT_DETECTION_OFF = 0x0,
    LATENT_DETECTION_ON  = 0x1,
} _V100_LATENT_DETECTION_MODE;

/*
** Types of finger lift modes available
**    Selected using CMD_SET_OPTION for M30x and V30x(Firmware> 9538) sensors
*/

typedef enum
{
    FORCE_FINGER_LIFT_NONE                = 0x0000,
    FORCE_FINGER_LIFT_MODE_ON          = 0x0001, // User required to lift finger after acquisition
    FORCE_FINGER_LIFT_MODE_OFF        = 0x0002  // default option for M30x and V30x sensors
} _V100_FORCE_FINGER_LIFT_MODE;

/*
** Options
*/

typedef enum
{
    OPTION_PD_LEVEL                            = 0x01,
    OPTION_SET_TWO                            = 0x02,
    OPTION_SET_THREE                        = 0x03,
    OPTION_SET_TEMPLATE_MODE                = 0x04,
    OPTION_SET_WSQ_COMPRESSION_RATIO        = 0x05,
    OPTION_SET_LATENT_DETECTION_MODE        = 0x1001,
    OPTION_SET_FORCE_FINGER_LIFT_MODE        = 0x2002,    // For M30x and V30x (Firmware> 9538)sensors to turn on/off figer lift mode during enrollment
    OPTION_SET_RESTORE_FILESYSTEM            = 0x5000    // M32X and M30X firmware only
} _V100_OPTION_TYPE;

#define MAX_WSQ_COMPRESSION_RATIO 20        // Max WSQ compression ratio user can set

typedef enum
{
    CROP_NONE      = 0x00,
    CROP_304x400 = 0x01,
    CROP_288x432 = 0x02,
    CROP_272x400 = 0x03,
    CROP_LAST     = 0x04,
} _V100_CROP_LEVEL;

typedef unsigned short ushort;

/**** Barcode Details ****/
#define MAX_BARCODE_DETAILS_SIZE    8192 // Client needs to allocate buffer of this size for V100_Get_Barcode_details call

#define MAX_QRCODE_X        57
#define MAX_QRCODE_Y        57
#define MAX_QRCODE_BYTES    3264   //((MAX_BARCODE_X)*(MAX_BARCODE_Y)+Padding )

typedef enum
{
    QR_MODE_NONE                    = 0x00,
    QR_MODE_NUMERIC                    = 0x01,
    QR_MODE_ALPHANUMERIC            = 0x02,
    QR_MODE_STRUCTURED_APPEND        = 0x03, // not really supported. If this mode is included in the QRCode we ignore the seq number, parity and continue to read the QRCode
    QR_MODE_BYTE                    = 0x04, // Interpretation of Byte character is subject to ECI, which is assumed to be ISO8859-1 if no ECI is defined in the QRCode
    QR_MODE_FNC1_FIRST_POSITION        = 0x05,
    QR_MODE_ECI                        = 0x07, // Client needs to interpret the byte character according to ECI value
    QR_MODE_KANJI                    = 0x08,
    QR_MODE_FNC1_SECOND_POSITION    = 0x09,
    QR_MODE_HANZI                    = 0x0D,
}_V100_QRCODE_ENCODE_MODE;

// Error correction level
typedef enum {
    QR_ECLEVEL_L = 0x00,     // lowest
    QR_ECLEVEL_M = 0x01,
    QR_ECLEVEL_Q = 0x02,
    QR_ECLEVEL_H = 0x03,     // highest
} _V100_QRCODE_ECLevel;

typedef enum{
    QR_NONE = 0x0,
    QR_OK ,                // detected and decoded successfully
    QR_NOT_DETECTED,    // not detected
    QR_NOT_DECODED,     // detected but can't decode
}_V100_QRCODE_STATUS;

#pragma pack(push)
#pragma pack(1)
typedef struct {
    //***// Fields Populated if QRCode is detected
    // Centers of QRCode fixed Patterns in the image
    float                          fTopLeftX;
    float                          fTopLeftY;
    float                          fTopRightX;
    float                          fTopRightY;
    float                          fBottomLeftX;
    float                          fBottomLeftY;
    float                          fBottomRightX;
    float                          fBottomRightY;
    float                        moduleSize;                    // Estimated module size in the image
    // QR Symbol
    unsigned char                  QRCode[MAX_QRCODE_BYTES];    // QRCode Symbol
    short                          nQRSizeX;                     // QR X size
    short                          nQRSizeY;                     // QR Y size
    //***//
    //***// Fields populated if QRCode is decoded successfully
    unsigned int                ECI;                         // Extended Channel Interpretation defined in the QRCode.  = 0 if not defined or default encoding used. Default ECI assumed to be ISO8859-1. Client needs to interpret the byte character according to ECI value.
    _V100_QRCODE_ENCODE_MODE    mode;                        // Encode mode in the QRCode. If multiple modes present in QRCode it represent the last mode
    _V100_QRCODE_ECLevel        ECLevel;                    // Error Correction Level in QRCode.
    unsigned char                 rawbytes[512];                // Data extracted from QR Code
    unsigned int                  rawBytesSize;                // Data size
    //***//
    _V100_QRCODE_STATUS            status;                        // Status
    short                          nDetectTime;                 // QR detect timing
    short                          nDecodeTime;                  // QR decode timing
} _V100_QRCODE_DETAILS;

#pragma pack(pop)

typedef enum
{
    BARCODE_NONE = 0x00,
    QRCODE  =    0x01,
    BARCODE_LAST
}_V100_BARCODE_TYPE;


/* Flow Control    */


#define SOHV                        0x560D
#define MAX_SIZE_SPOOF_TEMPLATE        512
#define MAX_FW_DATA_SIZE            (1024*1024*10)
#define MAX_USER_RECORD_SIZE        10240


#define MAX_FILE_NAME_LENGTH            64

typedef enum
{
     FS_FILE_TYPE_UNKNOWN = 0x00,
     FS_FILE_TYPE_FILE      = 0x01,
     FS_FILE_TYPE_DIR     = 0x02,
} _V100_FILE_ATTR;


typedef struct
{
    uint        FileSize;        //  [In/Out] Size of File
    uint        TimeStamp;        //  [In/Out] (optional) Time stamp.
    uint        Permissions;    //  [In/Out] (optional) Permissions
    uint        FileType;        //  [In/Out] FileType (0 = ASCII, 1=BINARY)
    uint        Hash;            //  [Out]       Hash Code
    _V100_FILE_ATTR    Attribute;        //  [Out]    Is File?  Is Dir?
} _V100_FILE;


/******************************************************************************
**
**    Policy Structure
**
**    READ/WRITE
**
******************************************************************************/

typedef struct
{
    unsigned short
    // Raw Images
    Get_Raw_Image,
    Set_Raw_Image,
    Get_Scaled_Image,
    Set_Scaled_Image,
    Get_Composite_Image,
    Set_Composite_Image,
    Get_Template,
    Set_Template,
    Get_Cal,
    Set_Cal,
    Get_Config,
    Set_Config,
    Get_Cmd,
    Set_Cmd,
    Set_Record,
    Get_Record,
    Write_Flash,
    Set_LED,
    Set_License_Key,
    Get_License_Key,
    Set_Enrollment_Rules,
    Calculate_Match,
    Calculate_Extract,
    Calculate_Identify,
    Calculate_Verify,
    Set_Spoof_Info,
    Calculate_Spoof_1,
    Calculate_Spoof_2,
    Calculate_Spoof_3,
    System_Diagnostics,
    Vid_Stream;
    unsigned short
    RESERVED_0,
    RESERVED_1,
    RESERVED_2,
    RESERVED_3,
    RESERVED_4,
    RESERVED_5,
    RESERVED_6,
    RESERVED_7,
    RESERVED_8,
    RESERVED_9;
} _V100_DEVICE_POLICY;


extern "C" _V100_DEVICE_POLICY POLICY_ALL;
extern "C" _V100_DEVICE_POLICY POLICY_LEVEL_1;
extern "C" _V100_DEVICE_POLICY POLICY_LEVEL_2;
extern "C" _V100_DEVICE_POLICY POLICY_LEVEL_3;

typedef struct
{
    uint
    /* System spec information */
    System_Heap_Peak,
    Algo_Heap_Peak,
    Topmain_Stack_Peak,
    ACQ_Stack_Peak,
    USB_Stack_Peak,
    BIT_Stack_Peak,
    PD_Stack_Peak,
    updater_Stack_Peak,
    Burnin_Stack_Peak,
    UART_Synch_Stack_Peak,
    Process_Stack_Peak,
    Mem_0_Alloc,
    Mem_1_Alloc,
    Mem_2_Alloc,
    Mem_3_Alloc;
    /* Timing */
    short PDTime;
    short AcqTime;
    short FF_CC_Time;
    short PreProcTime;
    short ExtractionTime;
    short MatchingTime;
    /* RS-232 Issues*/
    uint  Reserved_0;
    uint  Reserved_1;
    uint  Reserved_2;
    uint  Reserved_3;
} _V100_SYSTEM_DIAGNOSTICS;

typedef enum
{
    UNKNOWN_LUMI_DEVICE = 0,
    VENUS_V30X            = 1,    // V30x
    MERCURY_M30X        = 2,    // M30x
    MERCURY_M31X        = 3,    // M31x
    VENUS_V31X            = 4,    // V31x
    VENUS_V371            = 5,    // V371
    VENUS_V40X            = 6,    // V40x
    VENUS_V42X            = 7,    // V42x
    MERCURY_M32X        = 8,    // M32x
    MERCURY_M42X        = 9,    // M42X
    MERCURY_M21X        = 10,   // M21x
    VENUS_V310_10        = 11,   // V31x-10
} _V100_SENSOR_TYPE;

// Lumidigm Vender IDs
#define LUMI_VID_V_SERIES_EMBEDDED        0x0525        // V-series Embedded (V30x, V4xx)
#define LUMI_VID_NON_V_SERIES_EMBEDDED    0x1fae        // Everything other than V-series Embedded

// Lumidigm Product IDs
#define LUMI_PID_V_SERIES_EMBEDDED        0x3424
#define LUMI_PID_M_SERIES_EMBEDED        0x212c
#define LUMI_PID_V_SERIES_STREAMING        0x0021
#define LUMI_PID_M_SERIES_STREAMING        0x0041
#define LUMI_PID_M210_SERIES_STREAMING  0x0011
#define LUMI_PID_V310_10_SERIES_STREAMING  0x0013
#endif
