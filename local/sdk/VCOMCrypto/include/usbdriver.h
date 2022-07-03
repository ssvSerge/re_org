/*********************************************************************************

Copyright(c) 2005 Analog Devices, Inc. All Rights Reserved.

This software is proprietary and confidential.  By using this software you agree
to the terms of the associated Analog Devices License Agreement.

*********************************************************************************/

#ifndef _USBDRIVER_H_
#define _USBDRIVER_H_


#define MAX_IO_WAIT                    6000        // timeout for USB transfers in msecs

// pipe types
enum _PIPE_TYPE
{
    READ_PIPE,
    WRITE_PIPE
};

#if 1
HANDLE OpenOneDevice( HDEVINFO, PSP_INTERFACE_DEVICE_DATA, char *pPhysicalDeviceName );
HANDLE OpenUsbDevice( LPGUID, char * );
BOOL GetUsbDeviceFileName( LPGUID, char * );
DWORD QueryNumDevices( LPGUID pGuid );
HANDLE OpenDeviceHandle( LPGUID pGuid, _PIPE_TYPE nPipeType, DWORD dwDeviceNumber, BOOL bUseAsyncIo /*, char* pDeviceName*/);
BOOL ReadPipe(HANDLE hRead, LPVOID lpBuffer, DWORD nNumberOfBytesToRead, LPDWORD lpNumberOfBytesRead);
BOOL WritePipe(HANDLE hWrite, LPCVOID lpBuffer, DWORD nNumberOfBytesToWrite, LPDWORD lpNumberOfBytesWritten);


class USBDriverInterface
{
public:
    USBDriverInterface();
    ~USBDriverInterface();

    HANDLE OpenOneDevice( HDEVINFO, PSP_INTERFACE_DEVICE_DATA, char *pPhysicalDeviceName );
    HANDLE OpenUsbDevice( LPGUID, char * );
    BOOL GetUsbDeviceFileName( LPGUID, char * );
    HANDLE OpenDeviceHandle( LPGUID pGuid, _PIPE_TYPE nPipeType, DWORD dwDeviceNumber, BOOL bUseAsyncIo , char* pDeviceName);
    BOOL ReadPipe(HANDLE hRead, LPVOID lpBuffer, DWORD nNumberOfBytesToRead, LPDWORD lpNumberOfBytesRead);
    BOOL WritePipe(HANDLE hWrite, LPCVOID lpBuffer, DWORD nNumberOfBytesToWrite, LPDWORD lpNumberOfBytesWritten);
    static DWORD QueryNumDevices( LPGUID pGuid );
    static DWORD QueryNumNonStrDevices( LPGUID pGuid );
    BOOL Close();
private:
    BOOL g_bUseAsyncIo;                        // flag indicating if we're using ASYNC (TRUE) or SYNC (FALSE) IO
    HANDLE g_hReadEvent;                    // read event handle for ASYNC IO
    HANDLE g_hWriteEvent;                    // write event handle for ASYNC IO
    OVERLAPPED g_ReadOverlapped;            // for asynchronous IO reads for ASYNC IO
    OVERLAPPED g_WriteOverlapped;            // for asynchronous IO writes for ASYNC IO
};


#endif

#endif // _USBDRIVER_H_

