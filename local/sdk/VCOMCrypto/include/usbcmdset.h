/*********************************************************************************
**
** Copyright(c) 2005 Analog Devices, Inc. All Rights Reserved.
** This software is proprietary and confidential.  By using this software you agree
** to the terms of the associated Analog Devices License Agreement.
**
**    $Id: usbcmdset.h 33412 2018-09-06 06:24:11Z cduran $
**    $Date: 2018-09-06 00:24:11 -0600 (Thu, 06 Sep 2018) $
**    $Rev: 33412 $
**    $Author: cduran $
**
*********************************************************************************/

#ifndef _USBCMD_H_
#define _USBCMD_H_

#include "lumi_stdint.h"

///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
// defines for both host and device
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////


enum _VERSION_STRINGS           // version string info
{
    FW_BUILD_DATE,              // build date of firmware
    FW_BUILD_TIME,              // build time of firmware
    FW_VERSION_NUMBER,          // version number of firmware
    FW_TARGET_PROC,             // target processor of firmware
    FW_APPLICATION_NAME,        // application name of firmware
    NUM_VERSION_STRINGS         // number of version strings
};



#define    MAX_VERSION_STRING_LEN        32
#define VERSION_STRING_BLOCK_SIZE    (NUM_VERSION_STRINGS*MAX_VERSION_STRING_LEN)

#define LOOPBACK_HEADER_BYTES        4                        // bytes in header of loopback data

#ifdef _WIN32_WCE
#define MAX_DATA_BYTES_EZEXTENDER    0x04000                    // max bytes to send
#else
#define MAX_DATA_BYTES_EZEXTENDER    0x10000                    // max bytes to send
#endif

#define MIN_DATA_BYTES_EZEXTENDER    LOOPBACK_HEADER_BYTES    // min bytes to send

#define FILE_OPEN_MODE_OFFSET        0                        // byte offset for mode for file open
#define FILE_OPEN_FILENAME_OFFSET    4                        // byte offset for filename for file open

#define USBIO_STDIN_FD     0        // file descriptor for stdin on Blackfin
#define USBIO_STDOUT_FD 1        // file descriptor for stdout on Blackfin
#define USBIO_STDERR_FD 2        // file descriptor for stderr on Blackfin

///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
// defines for host only
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////

#ifdef _HOSTAPP_

enum _ERROR_VALUES            // error values
{
    OPERATION_PASSED = 0,
    UNSUPPORTED_COMMAND,
    IO_WRITE_USBCB_FAILED,
    IO_READ_USBCB_FAILED,
    IO_READ_DATA_FAILED,
    IO_WRITE_DATA_FAILED,
    OUT_OF_MEMORY_ON_HOST,
    NO_AVAILABLE_FILE_PTRS,
    COULD_NOT_CONNECT,
};

#endif    // _HOSTAPP_


///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
// defines for device only
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////

#ifndef _HOSTAPP_

#endif // ! _HOSTAPP_


#endif // _USBCMD_H_

