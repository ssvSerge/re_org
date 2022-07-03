#ifndef __V100_INTERNAL_TYPES
#define __V100_INTERNAL_TYPES


#ifdef _VDSP
#pragma diag(suppress: 301)
#endif

#include "lumi_stdint.h"

typedef unsigned char  uchar;
typedef unsigned short ushort;
typedef unsigned int   uint;


typedef struct
{
    uint    fw_id;
    uint    dev_id;
    uint    diag_level;
    uint    diag_version_num;
    uint    nImageBlobSize;
} DiagHeader;

typedef enum
{
    M300_Default      = 0x0000,        // Application code
    M300_BootManager  = 0x1000,        // Boot Manager
    M300_Diagnostic   = 0x2000,        // ECU_Test Diagnostics
    M300_Application  = 0x3000,        // Application Code
    M300_Image          = 0x4000,        // Image File
    NotUsed           = 0xffff
} Merc_Flash_Loc;


/*
**    New Mercury EEPROM to support Structured Acq
**
*/

typedef struct
{
    unsigned int Product_ID,
        Serial_Number,
        Revision_Code,
        Product_Reserved;
    unsigned short
        DateCode,
        Imager_ID,
        Imager_VER,
        Imager_MAN_Code_MSB,
        Imager_MAN_Code_LSB,
        Optical_Boresight_Row,
        Optical_Boresight_Col,
        DPI,
        Distortion_Coeff_0,
        Distortion_Coeff_1,
        Distortion_Coeff_2,
        Distortion_Coeff_3,
        PD_Brow,
        PD_Bcol,
        PD_Top_Offset,
         PD_Bot_Offset,
         PD_Seed_Exp_MSB,
         PD_Seed_Exp_LSB,
         PD_Seed_Gain,
         Latent_TL_X,
        Latent_TL_Y,
        Latent_BR_X,
        Latent_BR_Y,
        RESERVED7;

    // Presence detection and color calibration (M320)
    //
    unsigned char
        M32X_ID,            // must equal 32, or we can assume it is a legacy device without proper calibration
        Gain_PD,               // calibrated gain values for acquisitions with the sensor
        Gain_State1,
        Gain_State2;
    unsigned short
        Exp_PD,               // calibrated exposure values for acquisitions with the sensor
        Exp_State1,
        Exp_State2;
    unsigned char
        PD_Band_X_1,
        PD_Band_X_2,
        PD_Band_X_3,
        PD_Band_Y_Top_1,
        PD_Band_Y_Top_2,
        PD_Band_Y_Top_3,
        PD_Band_Y_Bot_1,
        PD_Band_Y_Bot_2,
        PD_Band_Y_Bot_3,
        PD_Band_Y_Mid,
        PD_Left,
        PD_Right,
        PD_Top,
        PD_Bot;
    float
        XStart_QVGA_UP,
        XStart_VGA_UP,        // may be needed for removing repeat movement frame in the future
        XStart_VGA_DN,        // may be needed for removing repeat movement frame in the future
        Mask_Ratio;

    unsigned char
        Red_State1,
        Green_State1,
        Blue_State1,
        Red_State2,
        Green_State2,
        Blue_State2;

} _MX00_EEPROM_DATA_M320;


/*
**    Legacy Mercury EEPROM
**
*/

typedef struct
{
    unsigned int Product_ID,
        Serial_Number,
        Revision_Code,
        Product_Reserved;
    unsigned short
        DateCode,
        Imager_ID,
        Imager_VER,
        Imager_MAN_Code_MSB,
        Imager_MAN_Code_LSB,
        Optical_Boresight_Row,
        Optical_Boresight_Col,
        DPI,
        Distortion_Coeff_0,
        Distortion_Coeff_1,
        Distortion_Coeff_2,
        Distortion_Coeff_3,
        PD_Brow,
        PD_Bcol,
        PD_Top_Offset,
        PD_Bot_Offset,
        PD_Seed_Exp_MSB,
        PD_Seed_Exp_LSB,
        PD_Seed_Gain,
        Latent_TL_X,
        Latent_TL_Y,
        Latent_BR_X,
        Latent_BR_Y,
        RESERVED4,
        RESERVED5,
        RESERVED6,
        RESERVED7;
} _MX00_EEPROM_DATA;

/*
** Data Stream Module EEPROM Settings
*/
typedef struct
{
    /*
    **    1st 8 Bytes MUST Comply with Cypress Boot Loader Format
    **    DO NOT MODIFY
    */
    unsigned char
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
    unsigned int
        Serial_Number,
        CPLD_Firmware_Revision,
        ManDateCode;
    unsigned short
        Product_ID,            // Post-Enum Product ID
        Platform_Type,
        Bx_Row,                // V31x Only
        Bx_Col,                // V31x Only
        PD_Row,                // V31x Only
        PD_Col,                // V31x Only
        DPI,                // V31x Only
        MfgStateFlag;        // Manufacturing state flag

    unsigned char
        pCalData[1520];        // V31x Only
    unsigned char
        pTagData[256];        // For V100_Set_Tag and V100_Get_Tag functionality first byte is size
} _VX00_DSM_EEPROM_DATA;


/*
**    History Hooks Structure
**  This provides history diagnostics of key metrics
*/
typedef struct
{
    unsigned short
        Num_Acq_Attempts,    // MUST be First Entry

        // SHARED TIME
        PD_Stop,            // PD latency
        SCALE_Stop,            // Scale & Correct latency
        ACQ_Stop,            // ACQ latency
        PROC_Stop,            // PROC latency
        EXTRACT_Stop,        // Extractor latency
        MATCH_Stop,            // Matcher latency
        TOTAL_Stop,            // TOTAL latency

        // BLUE ONLY
        Blue_Mask_Stop,        // Mask Creatation latency
        Blue_MaskRatio,
        Blue_MaskValid,
        Blue_MaskPosValid,
        Blue_MaskHistoValid,
        Blue_MaskMAE,
        Blue_MaskTime,
        Blue_Mask_NSat,
        Blue_Mask_Size,

        // WHITE ONLY
        White_Color_Left,
        White_Color_Right,
        White_Present_Left,
        White_Present_Right,
        White_Hist_Left,
        White_Hist_Right,
        White_Exposure,
        White_Centroid,

        // SHARED
        Finger_Quality,
        Exp1,
        Exp2,
        Exp3,
        Exp4,
        Exp1_Dark,
        Exp2_Dark,
        Exp3_Dark,
        PD_Exp,
        Color_Status,
        Hist_Status,
        Left_RMean,
        Left_NSat,
        Right_RMean,
        Right_NSat,
        BPwr1,
        BPwr2,
        BPwr3,
        BPwr4,
        Cb,
        Cr,
        Y,
        MAE,
        Cxy,
        ErrorCode,

        // LATENTS
        bColorCheck,
        bIllumCheck,
        bFreqsCheck,
        ColorScore,
        IllumScore,
        FreqScore,
        ColorEtime,
        IllumEtime,
        FreqEtime,

        // END MARK
        End_Mark;
} History_Type;

typedef enum
{
      VID_LED_1_ON   = 0x1000,
    VID_LED_2_ON   = 0x1001,
    VID_LED_3_ON   = 0x1002,
    VID_TIR_ON       = 0x1003,
    VID_DARK_ON    = 0x1004,
    VID_PD_ON       = 0x1005,
} _V100_INTERNAL_VID_STREAM_MODE;

typedef enum
{
    // These (first 6) were moved from shared types
    GREEN_CYCLE_ON            = 0x0010,    // legacy
    GREEN_CYCLE_OFF            = 0x0020,    // legacy
    RED_CYCLE_ON            = 0x0040,    // legacy
    RED_CYCLE_OFF            = 0x0080,    // legacy
    PD_ON                    = 0x0100,    // legacy
    PD_OFF                    = 0x0200,    // legacy
    // Internal LED states used for m-series
    MS_LED_PD_IR            = 2000,        // IR PD
    MS_LED_PD,
    MS_LED_OFF,
    MS_LED_NONP_1_GREEN,                // Green Only
    MS_LED_NONP_1_BLUE,                    // Blue Only
    MS_LED_NONP_2_GREEN,                // Green Only
    MS_LED_NONP_2_BLUE,                    // Blue Only
    MS_LED_VIDEO_DISPLAY,
    MS_LED_EXT_NONE,
    MS_MERC_STATE_1            = 2009,
    MS_MERC_STATE_2            = 2010,
} _V100_INTERNAL_LED_TYPE;

typedef enum
{
    /**** START ****/
    CMD_GET_MFG_STATE            = 0x10,        // get Manufacturing State
    CMD_SET_MFG_STATE            = 0x11,        // set Manufacturing State
    CMD_GET_CAL                     = 0x4B,        // INTERNAL get Calibration Struct
    CMD_SET_CAL                     = 0x4E,        // INTERNAL set Calibration Struct
    CMD_GET_RECORD                 = 0x53,        // INTERNAL Device->Xfer of "Record"
    CMD_PROCESS                     = 0x57,        // Process current Spec.
    CMD_DIAG_NONE                 = 0x90,
//    CMD_GET_SYSTEM_STATE         = 0x91,        // Get System State
    CMD_GET_LICENSE_KEY             = 0x93,
    CMD_WRITE_FILE                 = 0x94,
    CMD_READ_FILE                 = 0x95,
    CMD_GET_SPOOF_DETAILS         = 0x96,
    CMD_GET_SPOOF_DETAILS_V2     = 0x97,
    CMD_GET_EEPROM                 = 0x98,        // Get the EEPROM information from the M30x Sensor Head (Legacy Mode)
    CMD_SET_EEPROM               = 0x99,        // Set the EEPROM information from the M30x Sensor Head (Legacy Mode)
    CMD_GET_EEPROM_M320          = 0x9B,        // Get the EEPROM information from the M3xx Sensor Head with M320-SA calibration data
    CMD_SET_EEPROM_M320          = 0x9C,        // Set the EEPROM information from the M3xx Sensor Head with M320-SA calibration data
    /**** FILE MANAGEMENT ****/
    CMD_FILE_READ                =    0xB0,
    CMD_FILE_WRITE                =    0xB1,
    CMD_FILE_CD                    =    0xB2,
    CMD_FILE_GETCWD                =   0xB3,
    CMD_FILE_GET_ATTRIBUTES     =    0xB4,
//    CMD_FILE_DELETE                =    0xB5,  // Moved to Shared Types
    CMD_FILE_DIR_FINDFIRST      =    0xB6,
    CMD_FILE_DIR_FINDNEXT       =   0xB7,
    CMD_FORMAT_VOLUME            =    0xB8,
    /**** DIAGNOSTIC COMMANDS ****/
    CMD_LOOPBACK                 = 0xC0,        // Echo Back Packets
    CMD_BIT                        = 0xC3,        // Run Built-In Test
    CMD_TEST                    = 0xC4,        // Test Hook
    CMD_START_BURNIN            = 0xC5,        // Start Device Burn In Cycle
    CMD_STOP_BURNIN                = 0xC6,        // Stop Device Burn In Cycle
    CMD_BURN_CAL_IMAGES            = 0xC7,
    CMD_RUN_MAT                    = 0xC8,   // Run the MAT Tests (macro command)
    CMD_DIAGNOSTIC_TEST            = 0xC9,   // Diagnostic Test Hook (see diagnostic subcommands)

    CMD_QUERY_RECORD_ADDRESS    = 0x50,        // Return Base Address of Buffer
    CMD_QUERY_RECORD_SIZE        = 0x51,        // Return Size of Buffer
    CMD_QUERY_SUPPORT             = 0x40,        // query for support

    /*** Get and Set DSM EEPROM ***/
    CMD_GET_DSM_EEPROM          = 0x8D,    // Get the EEPROM information from the V31x DSM board
    CMD_SET_DSM_EEPROM            = 0x8E,    // Set the EEPROM information from the V31x DSM board

    /*** Deprecated in SDK8 (Moved from Shared Types) ***/
    // 1:1 Commands
} _V100_INTERNAL_COMMAND_SET;
/******************************************************************************
**
**    Extra Memory Locations to gather via Get/Set_Image
**
**
**
******************************************************************************/
typedef enum
{
    IMAGE_EX_NONE    = 0x1000,    // not available
    SCALE_IMAGE_1,                // 1   0x1001
    SCALE_IMAGE_2,                // 2   0x1002
    SCALE_IMAGE_3,                // 3   0x1003
    SYSTEM_MEMORY_MAP,            // 4   0x1004 Generated dynamically
    ALGO_MEMORY_MAP,            // 5   0x1005 Generated dynamically
    PD_HISTORY,                    // 6   0x1006 PD History
    LOGGER_XML,                    // 8   0x1007
    DIAG_DATA_5,                // 9   0x1008
    DIAG_DATA_6,                // 10  0x1009
    DIAG_DATA_7,                // 11  0x100A
    DIAG_DATA_8,                // 12  0x100B
    DIAG_DATA_9,                // 13  0x100C
    DIAG_DATA_10,                // 14  0x100D
    MEMORY_MAP_1,                // 15  0x100E
    MEMORY_MAP_2,                // 16  0x100F
    MEMORY_MAP_3,                // 17  0x1010
    MEMORY_MAP_4,                // 18  0x1011
    FEATURE_DATA,                // 18  0x1012(4114)
    IMAGE_SA,                    // 19  0x1013(4115)
    FEATURE_DATA_M,                // 20  0x1014(4116) for M320
    TIMING_DATA,                // 21  0x1015(4117)
    IMAGE_SA_METADATA,          // 22  0x1016(4118)
    IMAGE_GET_TEST_RESULTS  = 101,
    IMAGE_GET_SENSOR_CONFIG = 102,
} _V100_IMAGE_TYPE_EX;


typedef enum
{
    OPTION_SET_CONFIG               = 200,     // Temporary to get SensorConfig.json from V52x/V32x
    OPTION_SET_BORESITE              = 0x1000,
    OPTION_SET_AGC_DEFAULT             = 0x2000,
    OPTION_SET_AGC_MANUFACTURING    = 0x4000,
    OPTION_SET_LUMINANCE_TARGET        = 0x4001,// Override default 'luminance target' for auto-gain/exp on the V52X sensor
    OPTION_SET_LATENT_DETECTION        = 0x1001,
    OPTION_BURN_CAL_IMAGES            = 0x1002,
    OPTION_SET_FINGER_SAMPLING_MODE = 0x1004,
    OPTION_SAVE_DEBUG_IMAGES        = 0x1008,
    OPTION_SET_FINGER_LIFT_MODE        = 0x2001,// Internal use for ALL sensors to disable finger lift mode.
    OPTION_SET_CAL_DATA                = 0x1010,
    OPTION_SET_DPI                    = 0x1011,
    OPTION_SAVE_BORESITE            = 0x1012,
    OPTION_SET_MFG_STATE            = 0x1013,
    OPTION_SET_FLUSH_QUEUE            = 0x06,  // this is here to make sure we don't use this enum for something else. Its already used in logging firmware 16048 for mercury
    OPTION_SET_SA_STEP                = 0x3000,// This is used for setting the current acquisition step for SA process
    OPTION_SET_SA_TABLE                = 0x3001,// This is used to override the Acquisition table on sensor. Only for RD purposes.
    OPTION_SET_CAM_REG                = 0x3002,// Used by RD. Option to set a register value on camera. 3 bytes, 1st byte reg address, 2 bytes value
    OPTION_SET_EXTRACTOR_OPTIONS    = 0x3003,// Used by RD. Options to pass to Innovatrics extractor (default = 0x00000005)
    OPTION_SET_LEGACY_CAPTURE_MODE    = 0x3004,// Used in manufacturing to calibrate M300 sensor heads
    OPTION_DUMP_LOG                    = 0x3005,// Dumps timing log if SENGINE_LATENCY is defined (engineering test only)
    OPTION_CLEAR_LOG                = 0x3006,
} _V100_INTERNAL_OPTION_TYPE;

/*
**    Types of finger samplings available
**    Selected using CMD_SET_OPTION
*/

typedef enum
{
    FINGER_SAMPLING_NONE            = 0x0000,
    FINGER_SAMPLING_LOW                   = 0x0001, // low frames per sec
    FINGER_SAMPLING_HIGH            = 0x0002, // high frames per sec
    FINGER_SAMPLING_DYNAMIC            = 0x0003, // Default mode
} _V100_FINGER_SAMPLING_MODE;

/*
** Types of finger lift modes available
**    Selected using CMD_SET_OPTION
*/

typedef enum
{
    FINGER_LIFT_NONE                = 0x0000,
    FORCE_FINGER_LIFT_ON          = 0x0001, // Default mode. User required to lift finger after acquisition
    FORCE_FINGER_LIFT_OFF        = 0x0002,
} _V100_FINGER_LIFT_MODE;

/******************************************************************************
**
**    Calibration Structure
**    Platform Specific Information Stored in User FLASH
**
******************************************************************************/
typedef struct
{
    unsigned short
        Device_Serial_Number,        // Unique Device Serial Number
         Hardware_Rev,                // HW Revision Number             (xxx.xxx.xxx)
         Boresight_X,                // Imager Window Readout Offset
         Boresight_Y,                // Imager Window Readout Offset
        Struct_Size,                // Size in Bytes of This Structure
        DPI,                        // DPI
        Baud_Rate,                    // Initial Baud Rate
        Flow_Control,                // Flow Control
        COM2_Mode,                    // COM2 - Mode
        Override_Trigger,            // Override_Trigger
        Override_Heartbeat,            // Override_Heartbeat
        PD_Level,                    // Presense Detect Level
        Policy_Key,                    // Policy Key (1 = All, 2 = Gen, 3 = Image)
        Processing_Parameter1,        // Processing Paramter 1
        Processing_Parameter2,        // Processing Paramter 2
        Processing_Parameter3,        // Processing Paramter 3
        Processing_Parameter4,        // Processing Paramter 4
        Processing_Parameter5,        // Processing Paramter 5
        Processing_Parameter6,        // Processing Paramter 6
        Processing_Parameter7,        // Processing Paramter 7
        Processing_Parameter8,        // Processing Paramter 8
        Processing_Parameter9,        // Processing Paramter 9
         RESERVED_0,                    // Reserve 8 DWords
         RESERVED_1,
         RESERVED_2,
         RESERVED_3,
         RESERVED_4,
         RESERVED_5,
         RESERVED_6,
         RESERVED_7,
         RESERVED_8,
         RESERVED_9,
         RESERVED_10,
         RESERVED_11,
         RESERVED_12,
         RESERVED_13,
         RESERVED_14,
         RESERVED_15,
         RESERVED_16,
        Burnin_Mode;
} _V100_INTERFACE_CALIBRATION_HDR;


typedef unsigned char _V100_CALIBRATION_DATA[4096];        // Calibration Coeffs

typedef struct
{

     _V100_INTERFACE_CALIBRATION_HDR
         Hdr;
    _V100_CALIBRATION_DATA
        Data;

} _V100_INTERFACE_CALIBRATION_TYPE;


/******************************************************************************
**
**    LEGACY Calibration Structure
**
**
******************************************************************************/
typedef struct
{
    unsigned short
        Device_Serial_Number,        // Unique Device Serial Number
         Hardware_Rev,                // HW Revision Number             (xxx.xxx.xxx)
         Boresight_X,                // Imager Window Readout Offset
         Boresight_Y,                // Imager Window Readout Offset
        Struct_Size,                // Size in Bytes of This Structure
         RESERVED_0,
         RESERVED_1,
         RESERVED_2;

} _V100_INTERFACE_CALIBRATION_HDR_REV_0;

typedef struct
{

     _V100_INTERFACE_CALIBRATION_HDR_REV_0 Hdr;
    _V100_CALIBRATION_DATA  Data;

} _V100_INTERFACE_CALIBRATION_TYPE_REV_0;


typedef enum
{
    ePOLICY_ALL     = 0x01,
    ePOLICY_LEVEL_1 = 0x02,
    ePOLICY_LEVEL_2 = 0x03,
    ePOLICY_LEVEL_3 = 0x04,
} _V100_DEVICE_POLICY_ENUM;


typedef enum
{
    FLASH_ADDRESS_CONFIGURATION = 0x2000000,
} _V100_FLASH_LOCATION;

typedef struct
{
    uint nSOH;
    uint nFlashAddress;
    uint nFileSize;
    uint lTime;
} _V100_FILE_HEADER;

typedef struct
{
  uint nStartOfFrame;                  //0x00C0FFEE
  uint nSizeOfCalData;
  uint nLocPatch[3];                // x, y, l
  uchar SpoofCalData[48];
} _V100_SPOOF_CAL;


// Color Information from Gray Puck for E-Spoof
typedef struct
{
    uchar colorCalData[12]; // State 1-3 Center Pixel Color Average [B G G R]
    short expVals[4];        // State 1-4 Exposure values
} _V100_COLOR_INFO_CAL;

#define COLORCAL_OFFSET   1328    // COLORCAL marker location

// Card Reader Serial Number information for V371 units
#define V371_CRSN_OFFSET  1368  // CRSN marker location

#define V371_CRSN_SIZE    32


// One of each of these per first 3 planes.

// The "C" structure has a struct in a struct for portablity.
// The MATLAB structure is just for the wrapper
#ifdef _WIN32
#pragma pack(push)
#pragma pack(1)
#endif

typedef struct
{
    _V100_SPOOF_CAL spoofCalStruct;
    ushort             nGPSums[12];
    long            nSumX[12];
    long long        nSumX2[12];
    uint            nNsum;
    //
} _V100_SPOOF_METRICS;

typedef struct
{
    _V100_SPOOF_CAL spoofCalStruct;
    ushort             nGPSums[12];
    long            nSumX[48];
    long long        nSumX2[48];
    uint            nNsum[4];
    //
} _V100_SPOOF_METRICS_V2;

typedef struct
{
       uint            m_pStartMemPool;
    uint            m_pEndMemPool;
    uint            m_pCurMemStart;
    uint            m_pFirstMemoryNode;
    uint            m_nMemPoolLength;
    uint            m_nCurrentlyAllocated;
    uint            m_nPeakAllocation;
    uint            m_bInitialized;
} _V100_MEMORY_METRICS_HDR;

typedef struct
{
    uint    nSize;
    uint    pNext;
} _V100_MEMORY_METRICS_NODE;

// Moved to shared types
//typedef enum
//{
//     FS_FILE_TYPE_UNKNOWN = 0x00,
//     FS_FILE_TYPE_FILE      = 0x01,
//     FS_FILE_TYPE_DIR     = 0x02,
//} _V100_FILE_ATTR;


#ifdef _WIN32
#pragma pack(pop)
#endif



// Moved to shared types
//typedef struct
//{
//    uint        FileSize;        //  [In/Out] Size of File
//    uint        TimeStamp;        //  [In/Out] (optional) Time stamp.
//    uint        Permissions;    //  [In/Out] (optional) Permissions
//    uint        FileType;        //  [In/Out] FileType (0 = ASCII, 1=BINARY)
//    uint        Hash;            //  [Out]       Hash Code
//    _V100_FILE_ATTR    Attribute;        //  [Out]    Is File?  Is Dir?
//} _V100_FILE;


typedef struct
{
    uint NumFreeSpace;
    uint NumUsedSpace;
    uint NumFiles;
    uint NumDirectories;
} _V100_FILE_SYSTEM_ATTRIBUTE;


// The MATLAB structure is just for the wrapper

// Each Record consists of one _V100_USER_RECORD followed by
// (nSizeOpaque/sizeof(_V100_USER_RECORD_ENTRY)) _V100_USER_RECORD_ENTRYs, followed by
// nSizeMetaData MetaData.

// ESPOOF Model Header

typedef struct
{
    long            nSumX[12];            // 48 bytes
    long long        nSumX2[12];            // 96 bytes
    uint            nNsum;                // 4 bytes
    ushort             nGPSums[12];        // 24 bytes
} _V100_SPOOF_METRICS_MATLAB;

/*
**  DEFINITIONS
*/
#define MAX_FILE_NAME_LENGTH            64
#define OFFSET_BYTES_SPOOF_CAL_DATA        1260
#define MAX_FILE_SIZE                    (2*1024*1024)
#define MAX_USER_DATA_SIZE                256



typedef enum
{
    PROCESS_IMAGES = 1,
    PROCESS_IMAGES_AND_SPOOF,
    PROCESS_IMAGES_AND_SPOOF_WITH_FLAT_FIELDING,
    PROCESS_IMAGES_AND_SPOOF_M320
} _V100_PROCESS_TYPES;

// for ID == 1: pre-processing images only requires the DPI of the
// sensor on which the images were taken
typedef struct
{
    uint    nIdentifier; // == 1
    uint    native_dpi;
} _V100_PROCESS_IMAGES;

// for ID == 2: color cal data plus exposure times are needed for
// spoof calculations
typedef struct
{
    uint    nIdentifier; // == 2
    uint    native_dpi;
    uchar    color_cal_data[20];
    ushort    exposure_values[4];
} _V100_PROCESS_IMAGES_AND_SPOOF;

// for ID == 3: adds flat field coefficients (V31x and early V30x)
typedef struct
{
    uint    nIdentifier; // == 3
    uint    native_dpi;
    //uchar    color_cal_data[20];
    ushort    exposure_values[4];
    uchar    flat_field_coeffs[4096];
} _V100_PROCESS_IMAGES_AND_SPOOF_WITH_FLAT_FIELDING;

// for ID == 4: (M320 with eSpoof)
typedef struct
{
    uint    nIdentifier; // == 4
    uchar    color_cal_data[6];    // RGB State 1 / State 2
    uchar     gain_values[2];
    ushort    exposure_values[2];
    int        distortion_coefficients[2];
} _V100_PROCESS_IMAGES_AND_SPOOF_M320;


// Firmware update header for V4xx firmware files
typedef struct
{
    char    strFilename[16];    // Filename - 16 bytes
    short    version;
    short    type;                // Merc_Flash_Loc for Mercury, _V100_FIRMWARE_TYPE for V30X/V4XX
    uint    length;                // Length of decrypted data in bytes.
    short    mode;                // blackfin firmware mode. _V100_FIRMWARE_MODE;
    char    pad[6];
} _V100_FIRMWARE_UPDATE_TYPE;


// What type of firmware it is. Used in _V100_FIRMWARE_UPDATE_TYPE in header structure for type field
typedef enum
{
    FIRMWARE_TYPE_NONE            = 0x0000,
    FIRMWARE_TYPE_BLACKFIN        = 0x1000,
    FIRMWARE_TYPE_SPOOF            = 0x1001,
    FIRMWARE_TYPE_MAXQ_HEX        = 0x1002,

    FIRMWARE_TYPE_SET_TX_KEY    = 0x1005,
    FIRMWARE_TYPE_SE_OP         = 0x1006,

// The following are special commands supported only by the Intermediate
// firmware, for facilitating upgrades to Universal firmware.  These
// headers can be "stacked" in front of the one containing a typical
// payload (firmware or maxq...).  The firmware will execute the commands
// in the order given (one per header) until it encounters a typical
// header, or runs out of data.

    // No parameter is needed for this command.  Moves file system from
    // current location to the location used for Universal FW.  Also makes
    // a backup in the file cache portion of the System Info block.  Finally,
    // it moves the System Info block to the block just before the new
    // file system, leaving plenty of room for future firmware updates.
    FIRMWARE_TYPE_FORMAT_UNIVERSAL_WITH_BACKUP     = 0x1007,

    // The 'nMode' field contains block start and block count in first
    // two bytes.  This determines the size and the location where the
    // file system will start.  No other actions are taken.  To be used
    // when downgrading firmware to a legacy build.  Files must be loaded
    // from the host after using this option (and rebooting to let it take
    // effect).
    FIRMWARE_TYPE_FORMAT_LEGACY                    = 0x1008
}_V100_FIRMWARE_TYPE;

// Used in nMode field of _V100_ENC_FIRMWARE_HEADER struct for FIRMWARE_TYPE_MAXQ_HEX
typedef enum
{
   KEYMAP_RESTORE              = 0x0000,    // default behaviour
   KEYMAP_NO_RESTORE           = 0x0100
}_V100_SE_UPDATE_MODE;


typedef enum
{
    FIRMWARE_TM_LEGACY            = 0x0000,
    FIRMWARE_TM_MSK00            = 0x0100,
    FIRMWARE_TM_PKI                = 0x0200,
    FIRMWARE_TM_VEX                = 0x0300,
    FIRMWARE_TM_BASE            = 0x0400,
    FIRMWARE_TM_UNLOCKED        = 0x0500,
    FIRMWARE_TM_PKI_UNLOCKED    = 0x0600,
    FIRMWARE_TM_TECBAN            = 0x0700,
    FIRMWARE_TM_MSK01            = 0x0800,
    FIRMWARE_TM_INTERM            = 0x0900,
    FIRMWARE_TM_ALL                = 0x1000,
    FIRMWARE_TM_HYB01            = 0x1100,
    FIRMWARE_TM_RECOVERY        = 0x1200,
    FIRMWARE_TM_HYB02           = 0x1300,
    FIRMWARE_TM_CP001           = 0x1400,
    FIRMWARE_TM_ERROR           = 0xFFFF,
}_V100_FIRMWARE_CONFIG;



/******************************************************************************
**
**    Error codes for _V100_INTERFACE_STATUS_TYPE
**    (moved to internal types in SDK8)
**
******************************************************************************/

/*
**    COM Error Codes
*/
typedef enum
{
    COM_OK = 0x0000,
    COM_NOT_SUPPORTED = 0x0001,
    COM_FEATURE_LOCKED = 0x0002,
    COM_SOH_ERROR = 0x0004,
    COM_NUM_BYTES_ERROR = 0x0008,
    COM_CMD_ERROR = 0x0010,
    COM_INVALID_OPAQUE_DATA_FIELD = 0x0020,
    COM_INVALID_CRC = 0x0040,
    COM_MESSAGE_FRAGMENT = 0x0080,
    COM_TIMEOUT = 0x0100,
    COM_FINGER_QUALITY_FAIL = 0x0200,
    COM_FINGER_SPOOF_FAIL = 0x0400,
    COM_USERID_NOT_AVAILABLE = 0x0800,
    COM_LAST
} _V100_COM_ERROR_CODES;

/*
**    Service Error Codes
*/
typedef enum
{
    SRV_OK = 0x0000,
    SRV_IM_ERROR = 0x0001,    // Interrupt Manager
    SRV_EBIU_ERROR = 0x0002,    // EBIU
    SRV_PM_ERROR = 0x0004,    // Power Manager
    SRV_DCB_ERROR = 0x0008,    // Deffered Callback Manager
    SRV_DMA_ERROR = 0x0010,    // DMA Manager
    SRV_INT_ERROR = 0x0020,    // Interrupt Hook
    SRV_DEV_ERROR = 0x0040,    // Device Driver
    SRV_PF_ERROR = 0x0080,    // Program Flag Service
    SRV_CAM_INIT_ERROR = 0x0100,    // Camera Comm
    SRV_UART_ERROR = 0x0200,    // UART Error
    SRV_USB_ERROR = 0x0400,    // USB Error
    SRV_PAD_ERROR = 0x0800,    // Data Struct Pad MisAlignment
    SRV_CRYPTO_ERROR = 0x1000,    // Cant Validate Encryption
    SRV_FLASH_ERROR = 0x2000,    // Flash Driver Error
    SRV_MEM_ALLOC_ERROR = 0x4000,     // malloc Failure
    SRV_NXP_ERROR = 0x8000,    // NXP Chipset Failure
    SRV_LAST
} _V100_SERVICE_ERROR_CODES;

/*
**  Boot Error Codes
*/
typedef enum
{
    STATUS_BOOT_ERROR_NONE = 0x0000,
    STATUS_BOOT_ERROR_BSP = 0x0001,
    STATUS_BOOT_ERROR_DM = 0x0002,
    STATUS_BOOT_ERROR_CAMERA = 0x0004,
    STATUS_BOOT_ERROR_PM = 0x0008,
    STATUS_BOOT_ERROR_VDK = 0x0010,
    STATUS_BOOT_ERROR_TAMPER = 0x0020,
    STATUS_BOOT_ERROR_OTP = 0x0040,
    STATUS_BOOT_ERROR_CONFIG_INVALID = 0x0080,
    STATUS_BOOT_ERROR_FUTURE_2 = 0x0100, // -> SEngine fail to start
    STATUS_BOOT_ERROR_FUTURE_3 = 0x0200, // -> SECA fail to start
    STATUS_BOOT_ERROR_FUTURE_4 = 0x0400, // -> SHM fail (?)
    STATUS_BOOT_ERROR_FUTURE_5 = 0x0800, //
    STATUS_BOOT_ERROR_FUTURE_6 = 0x1000,
    STATUS_BOOT_ERROR_FUTURE_7 = 0x2000,
    STATUS_BOOT_ERROR_FUTURE_8 = 0x4000,
    STATUS_BOOT_ERROR_FUTURE_9 = 0x8000,

} _V100_BOOT_ERROR_CODES;

/*
**  Flash Update Status Codes
*/
typedef enum
{
    FLASH_READY,
    FLASH_BUSY,
    FLASH_COMPLETE,
    FLASH_ERROR
} _V100_FLASH_STATUS_TYPE;

/*
**    Built-In Test Result Error Codes
*/
typedef enum
{
    BIT_OK = 0x0000,        // Ok
    BIT_CAMERA_FAIL = 0x0001,        // Camera Test Fail
    BIT_MEM_FAIL = 0x0002,        // Memory Test Fail
    BIT_LAST
} _V100_BIT_ERROR_CODES;



// *** BEGIN DEPRECATED ENUMS FROM V100_Shared_Types.h (Deprecated in SDK8) *** //

typedef enum
{
    CROP_304x400 = 0x01,
    CROP_288x432 = 0x02,
}_V100_INTERNAL_CROP_LEVEL;

typedef enum
{
    TRIGGER_MOTION_ON = 0x02,
    TRIGGER_MOTION_OFF = 0x03,
    TRIGGER_BARCODE_ON = 0x05,
    TRIGGER_HYBRID_ON = 0x06,
    CANCEL_VERIFICATION = 0x99,
}_V100_INTERNAL_TRIGGER_MODE;

//typedef enum
//{
//    V100_MATCH_RESULT_BEGIN = 0x00,
//    MATCH = 0x01,
//    NO_MATCH = 0x02,
//    V100_MATCH_RESULT_END = 0x03,
//} _V100_MATCH_RESULT;


/*
**    Types of Spoof Models Available
*/
//typedef enum
//{
//    SPOOF_MODEL_NONE = 0x0000,    // not available
//    SPOOF_MODEL_001,            // current spoof model
//    //SPOOF_MODEL_002,            // next spoof model example
//    SPOOF_MODEL_LAST
//} _V100_SPOOF_MODEL;



//typedef enum
//{
//    CMD_EXPOSURE_NONE = 0x0000,
//    CMD_AUTO_EXPOSURE,
//    CMD_MANUAL_EXPOSURE,
//    CMD_EXPOSURE_LAST
//} _V100_EXPOSURE_MODES_TYPE;

//
//typedef struct
//{
//    unsigned int
//        RecType,                    // _V100_DATA_RECORD_TYPE
//        RecSize,                    // Sizeof RECORD in Bytes
//        RecCRC;                       // CRC-32
//} _V100_RECORD_HEADER;
//


//typedef enum
//{
//    DATA_RECORD_TYPE_FIRMWARE = 0x10,
//    DATA_RECORD_TYPE_DATABASE = 0x20,
//    DATA_RECORD_TYPE_CALFILE = 0x40
//} _V100_DATA_RECORD_TYPE;


//typedef unsigned short _V100_MACRO_ARGUMENT_TYPE;
//#define    MACRO_NONE                      0x0000
//#define    MACRO_TEMPLATE                 0x0001
//#define    MACRO_COMPOSITE_IMAGE        0x0002
//#define    MACRO_USER_ID                0x0010

/*
**    Types of Finger Presence Detection Available
*/
//typedef enum {
//    PD_NONE = 0x0000,
//    PD_SW_PRESENCE_DETECTION,
//    PD_HW_PRESENCE_DETECTION,
//    PD_SW_WITH_QUALITY_CHECK,
//    PD_SW_WITH_QC_AND_SPOOF,
//    PD_SENSITIVITY_LOW,
//    PD_SENSITIVITY_MEDIUM,
//    PD_SENSITIVITY_HIGH,
//    PD_LAST
//} _V100_PRESENCE_DETECTION_TYPE;


/*
**  Download Data Record Error Codes
*/
//typedef enum
//{
//    DATA_RECORD_OK = 0x0000,
//    DATA_RECORD_ERROR_UNSUPPORTED,
//    DATA_RECORD_ERROR_CRC,
//    DATA_RECORD_ERROR_GENERAL,
//    DATA_RECORD_THREAD_ERROR,
//    DATA_RECORD_LAST
//} _V100_DATA_RECORD_ERROR_TYPE;

/*
**    Multi-Spectral LED Identifiers
*/
typedef enum {
    MS_LED_TIR = 0x0001,
    MS_LED_NONP_1,
    MS_LED_NONP_2,
    MS_LED_POL_1,
    MS_LED_DARK,
    MS_LED_NONE
} _V100_LED_TYPE;
//
//
///*
//**    AEC/AGC Type Selections
//*/
//typedef enum
//{
//    AEC_AGC_OFF = 0x000,
//    AEC_ENABLE = 0x0001,
//    AGC_ENABLE = 0x0002
//} _V100_GAIN_TYPES;

// *** END OF DEPRECATED ENUMS FROM V100_Shared_Types.h (Deprecated in SDK8) *** //


struct _V100_ENC_FIRMWARE_HEADER
{
    char filename[32];
    u32 version;
    _V100_FIRMWARE_TYPE type;
    u32 length;
    u32 mode;
};

enum class PhysicalSensorTypes : int
{
    UNKNOWN_LUMI_DEVICE = 0,
    VENUS_V30X = 1,
    MERCURY_M30X = 2,
    MERCURY_M31X = 3,
    VENUS_V31X = 4,
    VENUS_V371 = 5,
    VENUS_V40X = 6,
    VENUS_V42X = 7,
    MERCURY_M32X = 8,
    MERCURY_M42X = 9,
    MERCURY_M21X = 10,
    VENUS_V31X_10 = 11,
    VENUS_V32X = 12,
    VENUS_V52X = 13,
    UNSUPPORTED_LUMI_DEVICE = 99
};

enum class TypedTransactionModels : unsigned int
{
    MSK00 = 0x0100,
    PKI = 0x0200,
    VEX = 0x0300,
    BASE = 0x0400,
    UNLOCKED = 0x0500,
    PKI_UNLOCKED = 0x0600,
    TECBAN = 0x0700,
    MSK01 = 0x0800,
    INTERM = 0x0900,
    ALL = 0x1000,
    HYB01 = 0x1100,
    RECOVERY = 0x1200,
    HYB02 = 0x1300,
    CP001 = 0x1400
};


#endif
