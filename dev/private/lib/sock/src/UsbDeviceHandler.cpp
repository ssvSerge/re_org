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


#include <string.h>
#include <iostream>

#include <UsbCommunication.h>
#include <usbcmdset.h>

#ifndef __GNUC__

    #include <setupapi.h>
    #include <guiddef.h>
    #include <ldiguid.h>

#else
    #include "windows_conv.h"
    #include "TransportLibUSB.h"

    #define ACQUIRE_SYNC_MUTEX
    #define RELEASE_SYNC_MUTEX

#endif

typedef struct _USBCB {          // USB command block
    uint32_t   ulCommand;        // command to execute
    uint32_t   ulData;           // generic data field
    uint32_t   ulCount;          // number of bytes to transfer
}   USBCB, * PUSBCB;



static constexpr int MutexTimeout       = 1000;
static constexpr int UsbCommTimeout     = 60 * 1000;
static constexpr int UsbMtu             = MAX_DATA_BYTES_EZEXTENDER;


static uint32_t QueryNumNonStrDevices ( LPGUID pGuid ) {

    DWORD nDevices = 0;

    HDEVINFO hdInfo = SetupDiGetClassDevs ( pGuid, NULL, NULL, DIGCF_PRESENT | DIGCF_DEVICEINTERFACE );

    if ( hdInfo != INVALID_HANDLE_VALUE ) {

        for ( nDevices = 0; ; nDevices++ ) {
            SP_INTERFACE_DEVICE_DATA ifData;
            ifData.cbSize = sizeof ( ifData );
            if ( ! SetupDiEnumInterfaceDevice ( hdInfo, NULL, pGuid, nDevices, &ifData ) ) {
                if ( GetLastError () == ERROR_NO_MORE_ITEMS ) {
                    break;
                } else {
                    SetupDiDestroyDeviceInfoList ( hdInfo );
                    return (0);
                }
            }
        }

        SetupDiDestroyDeviceInfoList ( hdInfo );
    }

    return(nDevices);
}

UsbCommunication::UsbCommunication ( void ) {

}

UsbCommunication::~UsbCommunication ( void ) {

}

USB_ERROR_CODE UsbCommunication::GetDevicesCnt ( uint32_t& cnt ) {

    USB_ERROR_CODE ret_val = USB_ERROR_CODE::ERR_NONE;
    cnt = QueryNumNonStrDevices( (LPGUID) (&GUID_BULKLDI) );
    return ret_val;
}

USB_ERROR_CODE UsbCommunication::Initialize ( uint32_t device_id ) {

    USB_ERROR_CODE ret_val = USB_ERROR_CODE::ERR_GENERIC;

    uint32_t devices_cnt = 0;

    devices_cnt = QueryNumNonStrDevices ( (LPGUID) (&GUID_BULKLDI) );

    if ( devices_cnt == 0 ) {
        ret_val = USB_ERROR_CODE::USB_NO_DEVICES_FOUND;
    } else 
    if ( devices_cnt <= device_id ) {
        ret_val = USB_ERROR_CODE::USB_NO_DEVICES_FOUND;
    } else {

        hWrite = OpenDeviceHandle ( (LPGUID) &GUID_BULKLDI, PIPE_TYPE::WRITE_PIPE, device_id, TRUE, hPhysicalDeviceID );
        hRead  = OpenDeviceHandle ( (LPGUID) &GUID_BULKLDI, PIPE_TYPE::READ_PIPE,  device_id, TRUE, hPhysicalDeviceID );

        if ( hWrite == INVALID_HANDLE_VALUE ) {
            ret_val = USB_ERROR_CODE::USB_WRITE_PIPE_ERROR;
        }
        if ( hRead == INVALID_HANDLE_VALUE ) {
            ret_val = USB_ERROR_CODE::USB_READ_PIPE_ERROR;
        }

        strncpy ( hPhysicalDeviceID, "LUMI", 4 );

    }

    return ret_val;
}

HANDLE UsbCommunication::OpenDeviceHandle ( LPGUID pGuid, PIPE_TYPE nPipeType, DWORD dwDeviceNumber, BOOL bUseAsyncIo, char* pPhysicalDeviceName ) {

    HANDLE hDev     = INVALID_HANDLE_VALUE;

    g_bUseAsyncIo   = bUseAsyncIo;

    HDEVINFO hdInfo = SetupDiGetClassDevs ( pGuid, NULL, NULL, DIGCF_PRESENT | DIGCF_DEVICEINTERFACE );

    if ( hdInfo == INVALID_HANDLE_VALUE ) {
        return INVALID_HANDLE_VALUE;
    }

    SP_INTERFACE_DEVICE_DATA ifData;
    ifData.cbSize = sizeof ( ifData );

    if ( ! SetupDiEnumInterfaceDevice ( hdInfo, NULL, pGuid, dwDeviceNumber, &ifData ) ) {
        SetupDiDestroyDeviceInfoList ( hdInfo );
        return INVALID_HANDLE_VALUE;
    }

    DWORD dwNameLength = 0;

    SetupDiGetInterfaceDeviceDetail ( hdInfo, &ifData, NULL, 0, &dwNameLength, NULL );

    dwNameLength += 32;

    PSP_INTERFACE_DEVICE_DETAIL_DATA detail = (PSP_INTERFACE_DEVICE_DETAIL_DATA) malloc ( dwNameLength );

    if ( ! detail ) {
        SetupDiDestroyDeviceInfoList ( hdInfo );
        return INVALID_HANDLE_VALUE;
    }

    detail->cbSize = sizeof ( SP_INTERFACE_DEVICE_DETAIL_DATA );

    // get the DeviceName
    if ( ! SetupDiGetInterfaceDeviceDetail ( hdInfo, &ifData, detail, dwNameLength, NULL, NULL ) ) {
        free ( (PVOID) detail );
        SetupDiDestroyDeviceInfoList ( hdInfo );
        return INVALID_HANDLE_VALUE;
    }

    char szDeviceName [MAX_PATH];
    strncpy_s ( szDeviceName, sizeof ( szDeviceName ), detail->DevicePath, sizeof ( szDeviceName ) );
    free ( (PVOID) detail );

    SetupDiDestroyDeviceInfoList ( hdInfo );

    strcpy ( pPhysicalDeviceName, szDeviceName );

    if ( GUID_BULKLDI == *pGuid ) {
        strcat_s ( szDeviceName, sizeof ( szDeviceName ), "\\" );

        // if caller wants the read pipe
        if ( PIPE_TYPE::READ_PIPE == nPipeType ) {
            if ( g_hReadEvent ) {
                CloseHandle ( g_hReadEvent );
            }
            if ( bUseAsyncIo ) {
                g_hReadEvent = CreateEvent ( NULL, TRUE, TRUE, NULL );
                g_ReadOverlapped.hEvent = g_hReadEvent;
            }

            strcat_s ( szDeviceName, sizeof ( szDeviceName ), "PIPE_0x00" );
        } else
        if ( PIPE_TYPE::WRITE_PIPE == nPipeType ) {
            if ( g_hWriteEvent ) {
                CloseHandle ( g_hWriteEvent );
            }
            if ( bUseAsyncIo ) {
                g_hWriteEvent = CreateEvent ( NULL, TRUE, TRUE, NULL );
                g_WriteOverlapped.hEvent = g_hWriteEvent;
            }

            strcat_s ( szDeviceName, sizeof ( szDeviceName ), "PIPE_0x01" );
        } else {
            return INVALID_HANDLE_VALUE;
        }

    } else {
        return INVALID_HANDLE_VALUE;
    }

    // determine the attributes
    DWORD dwAttribs;
    if ( bUseAsyncIo ) {
        dwAttribs = FILE_FLAG_OVERLAPPED;
    } else {
        dwAttribs = FILE_ATTRIBUTE_NORMAL;
    }

    hDev = CreateFile ( szDeviceName, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, dwAttribs, NULL );

    // if ( hDev == INVALID_HANDLE_VALUE ) {
    //     NOISY ( "Failed to open device" );
    // } else {
    //     NOISY ( "Opened successfully.\n" );
    // }

    return hDev;
}

HANDLE UsbCommunication::GetMutexId () {

    if ( strcmp ( hPhysicalDeviceID, "" ) == 0 ) {
        return NULL;
    }

    if ( hMutexID == NULL ) {
        hMutexID = CreateMutexA ( NULL, FALSE, hPhysicalDeviceID );
    }

    return hMutexID;
}

void UsbCommunication::Close () {
    CloseHandle ( hWrite );
    CloseHandle ( hRead );
    CloseHandle ( hMutexID );
}

bool UsbCommunication::GetDeviceId ( char* szDeviceId, uint32_t& nLength ) {
    nLength = 0;
    strcpy ( szDeviceId, hPhysicalDeviceID );
    nLength = (uint32_t) strlen ( szDeviceId ) + 1;
    return true;
}

bool UsbCommunication::TransmitCommand ( int route_flag, const uint8_t* myPacket, uint32_t nTxSize, uint8_t* pResponse, uint32_t& nRxSize ) {

    HANDLE hEvent = GetMutexId ();

    if ( WAIT_TIMEOUT == WaitForSingleObject ( hEvent, MutexTimeout ) ) {
        fprintf ( stdout, "\nMutex timed-out" );
        return 0;
    }

    USBCB usbcb = { 0 };            // command block
    usbcb.ulCommand = 0;            // command    0 = HOST_VCOM (plaintext) IENVELOPE::XPRT_VCOM_ENCAPSULATED (Transport Security)
    usbcb.ulCount   = nTxSize;      // not used 
    usbcb.ulData    = route_flag;   // Who should handle command 0=JEngine / 1=UsbTransceiver.

    nRxSize = 0;

    // Send Header.
    if ( ! WriteFrame ( &usbcb, sizeof ( usbcb ), UsbCommTimeout ) ) {
        ReleaseMutex ( hEvent );
        return 0;
    }

    // Send Challenge Data Packet
    if ( ! WriteFrame ( myPacket, usbcb.ulCount, UsbCommTimeout ) ) {
        ReleaseMutex ( hEvent );
        return 0;
    }

    // Read Response Header Packet
    if ( ! ReadFrame ( &usbcb, sizeof ( usbcb ), UsbCommTimeout ) ) {
        ReleaseMutex ( hEvent );
        return 0;
    }

    // Read Response Data Packet
    if ( ! ReadFrame ( pResponse, usbcb.ulCount, UsbCommTimeout ) ) {
        ReleaseMutex ( hEvent );
        return 0;
    }

    nRxSize = usbcb.ulCount;

    ReleaseMutex ( hEvent );
    return 1;
}

bool UsbCommunication::WriteFrame ( const void* pPacket, uint32_t nPacketSize, uint32_t timeout_ms ) {

    bool ret_val = false;

    uint32_t tx_cnt  = 0;
    uint32_t tx_part = 0;
    DWORD    io_cnt  = 0;

    const uint8_t* tx_pos = static_cast<const uint8_t*> (pPacket);

    for ( ; ; ) {

        if ( tx_cnt == nPacketSize ) {
            ret_val = true;
            break;
        }

        tx_part = nPacketSize - tx_cnt;
        if ( tx_part > UsbMtu ) {
            tx_part = UsbMtu;
        }

        if ( ! WritePipe ( tx_pos+tx_cnt, tx_part, timeout_ms, io_cnt ) ) {
            return false;
        }

        tx_cnt += tx_part;

    }

    return ret_val;
}

bool UsbCommunication::ReadFrame ( void* pPacket, uint32_t nPacketSize, uint32_t timeout_ms ) {

    bool ret_val = false;

    uint32_t rx_cnt = 0;
    uint32_t rx_part = 0;
    DWORD    io_cnt = 0;

    uint8_t* rx_pos = static_cast<uint8_t*> (pPacket);

    for ( ; ; ) {

        if( rx_cnt == nPacketSize ) {
            ret_val = true;
            break;
        }

        rx_part = nPacketSize - rx_cnt;
        if ( rx_part > UsbMtu ) {
            rx_part = UsbMtu;
        }

        if ( ! WritePipe ( rx_pos+rx_cnt, rx_part, timeout_ms, io_cnt ) ) {
            return false;
        }

        rx_cnt += rx_part;

    }

    return ret_val;
}

BOOL UsbCommunication::ReadPipe ( LPVOID lpBuffer, DWORD nNumberOfBytesToRead, uint32_t timeout_ms, DWORD& io_cnt ) {

    BOOL ret_val = TRUE;

    if ( g_bUseAsyncIo ) {

        ReadFile ( hRead, lpBuffer, nNumberOfBytesToRead, &io_cnt, &g_ReadOverlapped );

        if ( WaitForSingleObject ( g_hReadEvent, timeout_ms ) != WAIT_OBJECT_0 ) {
            ret_val = FALSE;
        }

        if ( ! GetOverlappedResult ( hRead, &g_ReadOverlapped, &io_cnt, FALSE ) ) {
            ret_val = FALSE;
        }

        if ( ! ret_val ) {
            CancelIo ( hRead );
        }

        if ( io_cnt > 0  ) {
            if ( io_cnt != nNumberOfBytesToRead ) {
                ret_val = FALSE;
            }
        }

    } else {
        ret_val = ReadFile ( hRead, lpBuffer, nNumberOfBytesToRead, &io_cnt, NULL );
    }

    return ret_val;
}

BOOL UsbCommunication::WritePipe ( LPCVOID lpBuffer, DWORD nNumberOfBytesToWrite, uint32_t timeout_ms, DWORD& io_cnt ) {

    BOOL ret_val = TRUE;

    if ( g_bUseAsyncIo ) {

        WriteFile ( hWrite, lpBuffer, nNumberOfBytesToWrite, &io_cnt, &g_WriteOverlapped );

        if ( WaitForSingleObject ( g_hWriteEvent, timeout_ms ) != WAIT_OBJECT_0 ) {
            ret_val = FALSE;
        }

        if ( ! GetOverlappedResult ( hWrite, &g_WriteOverlapped, &io_cnt, FALSE ) ) {
            ret_val = FALSE;
        }

        if ( ! ret_val ) {
            CancelIo ( hWrite );
        }

        if ( io_cnt != nNumberOfBytesToWrite ) {
            ret_val = FALSE;
        }

    } else {
        ret_val = WriteFile ( hWrite, lpBuffer, nNumberOfBytesToWrite, &io_cnt, NULL );
    }

    return ret_val;
}
