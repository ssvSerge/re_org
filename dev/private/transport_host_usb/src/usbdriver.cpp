/*********************************************************************************

Copyright(c) 2005 Analog Devices, Inc. All Rights Reserved.

This software is proprietary and confidential.  By using this software you agree
to the terms of the associated Analog Devices License Agreement.

*********************************************************************************/

#ifdef __SUPPORT_WIN32_USB__

#include "stdafx.h"
#include <windows.h>
#include <stdio.h>
#include <setupapi.h>
#include <usbdi.h>
#include "ldiguid.h"
#include "usbcmdset.h"
#include "usbdriver.h"

#if 0
// choose whether to display (1) or supress (0) printf debug information in this file
#if 0
    #define NOISY(_x_) printf _x_ ;
#else
    #define NOISY(_x_) ;
#endif

BOOL g_bUseAsyncIo = TRUE;                        // flag indicating if we're using ASYNC (TRUE) or SYNC (FALSE) IO
HANDLE g_hReadEvent = INVALID_HANDLE_VALUE;        // read event handle for ASYNC IO
HANDLE g_hWriteEvent = INVALID_HANDLE_VALUE;    // write event handle for ASYNC IO
OVERLAPPED g_ReadOverlapped;                    // for asynchronous IO reads for ASYNC IO
OVERLAPPED g_WriteOverlapped;                    // for asynchronous IO writes for ASYNC IO



/******************************************************************************
Routine Description:

    Given the HardwareDeviceInfo, representing a handle to the plug and
    play information, and deviceInfoData, representing a specific usb device,
    open that device and fill in all the relevant information in the given
    USB_DEVICE_DESCRIPTOR structure.

Arguments:

    HardwareDeviceInfo:  handle to info obtained from Pnp mgr via SetupDiGetClassDevs()
    DeviceInfoData:      ptr to info obtained via SetupDiEnumInterfaceDevice()

Return Value:

    return HANDLE if the open and initialization was successfull,
    else INVLAID_HANDLE_VALUE.

******************************************************************************/
HANDLE OpenOneDevice (
    IN       HDEVINFO                    HardwareDeviceInfo,
    IN       PSP_INTERFACE_DEVICE_DATA   DeviceInfoData,
    IN         char *devName
    )
{
    PSP_INTERFACE_DEVICE_DETAIL_DATA     functionClassDeviceData = NULL;
    ULONG                                predictedLength = 0;
    ULONG                                requiredLength = 0;
    HANDLE                                 hOut = INVALID_HANDLE_VALUE;

    //
    // allocate a function class device data structure to receive the
    // goods about this particular device.
    //
    SetupDiGetInterfaceDeviceDetail (
            HardwareDeviceInfo,
            DeviceInfoData,
            NULL,            // probing so no output buffer yet
            0,                // probing so output buffer length of zero
            &requiredLength,
            NULL);            // not interested in the specific dev-node


    predictedLength = requiredLength;

    functionClassDeviceData = (_SP_DEVICE_INTERFACE_DETAIL_DATA_A * )malloc (predictedLength);
    if(NULL == functionClassDeviceData) {
        return INVALID_HANDLE_VALUE;
    }
    functionClassDeviceData->cbSize = sizeof (SP_INTERFACE_DEVICE_DETAIL_DATA);

    //
    // Retrieve the information from Plug and Play.
    //
    if (! SetupDiGetInterfaceDeviceDetail (
               HardwareDeviceInfo,
               DeviceInfoData,
               functionClassDeviceData,
               predictedLength,
               &requiredLength,
               NULL)) {
        free( functionClassDeviceData );
        return INVALID_HANDLE_VALUE;
    }

    strcpy_s( devName, 128, functionClassDeviceData->DevicePath) ; // added strcpy_s SPC
    NOISY(( "Attempting to open %s\n", devName ));

    hOut = CreateFile (
                  functionClassDeviceData->DevicePath,
                  GENERIC_READ | GENERIC_WRITE,
                  FILE_SHARE_READ | FILE_SHARE_WRITE,
                  NULL,                // no SECURITY_ATTRIBUTES structure
                  OPEN_EXISTING,    // No special create flags
                  0,                // No special attributes
                  NULL);            // No template file

    if (INVALID_HANDLE_VALUE == hOut) {
        printf( "FAILED to open %s\n", devName );
    }
    free( functionClassDeviceData );
    return hOut;
}


/******************************************************************************
Routine Description:

   Do the required PnP things in order to find
   the next available proper device in the system at this time.

Arguments:

    pGuid:      ptr to GUID registered by the driver itself
    outNameBuf: the generated name for this device

Return Value:

    return HANDLE if the open and initialization was successful,
    else INVLAID_HANDLE_VALUE.
******************************************************************************/
HANDLE OpenUsbDevice( LPGUID  pGuid, char *outNameBuf)
{
   ULONG NumberDevices;
   HANDLE hOut = INVALID_HANDLE_VALUE;
   HDEVINFO                 hardwareDeviceInfo;
   SP_INTERFACE_DEVICE_DATA deviceInfoData;
   ULONG                    i;
   BOOLEAN                  done;
   PUSB_DEVICE_DESCRIPTOR   usbDeviceInst;
   PUSB_DEVICE_DESCRIPTOR    *UsbDevices = &usbDeviceInst;
   PUSB_DEVICE_DESCRIPTOR   tempDevDesc;

   *UsbDevices = NULL;
   tempDevDesc = NULL;
   NumberDevices = 0;

   //
   // Open a handle to the plug and play dev node.
   // SetupDiGetClassDevs() returns a device information set that contains info on all
   // installed devices of a specified class.
   //
   hardwareDeviceInfo = SetupDiGetClassDevs (
                           pGuid,
                           NULL,                        // Define no enumerator (global)
                           NULL,                        // Define no
                           (DIGCF_PRESENT |                // Only Devices present
                            DIGCF_INTERFACEDEVICE));    // Function class devices.

   //
   // Take a wild guess at the number of devices we have;
   // Be prepared to realloc and retry if there are more than we guessed
   //
   NumberDevices = 4;
   done = FALSE;
   deviceInfoData.cbSize = sizeof (SP_INTERFACE_DEVICE_DATA);

   i=0;
   while (!done) {
      NumberDevices *= 2;

      if (*UsbDevices) {
            tempDevDesc =
               (PUSB_DEVICE_DESCRIPTOR)realloc (*UsbDevices, (NumberDevices * sizeof (USB_DEVICE_DESCRIPTOR)));
            if(tempDevDesc) {
                *UsbDevices = tempDevDesc;
                tempDevDesc = NULL;
            }
            else {
                free(*UsbDevices);
                *UsbDevices = NULL;
            }
      } else {
         *UsbDevices = (PUSB_DEVICE_DESCRIPTOR)calloc (NumberDevices, sizeof (USB_DEVICE_DESCRIPTOR));
      }

      if (NULL == *UsbDevices) {

         // SetupDiDestroyDeviceInfoList destroys a device information set
         // and frees all associated memory.

         SetupDiDestroyDeviceInfoList (hardwareDeviceInfo);
         return INVALID_HANDLE_VALUE;
      }

      usbDeviceInst = *UsbDevices + i;

      for (; i < NumberDevices; i++) {

         // SetupDiEnumDeviceInterfaces() returns information about device interfaces
         // exposed by one or more devices. Each call returns information about one interface;
         // the routine can be called repeatedly to get information about several interfaces
         // exposed by one or more devices.

         if (SetupDiEnumDeviceInterfaces (hardwareDeviceInfo,
                                         0, // We don't care about specific PDOs
                                                                                 pGuid,
                                         i,
                                         &deviceInfoData)) {

            hOut = OpenOneDevice (hardwareDeviceInfo, &deviceInfoData, outNameBuf);
                        if ( hOut != INVALID_HANDLE_VALUE ) {
               done = TRUE;
               break;
                        }
         } else {
            if (ERROR_NO_MORE_ITEMS == GetLastError()) {
               done = TRUE;
               break;
            }
         }
      }
   }

   NumberDevices = i;

   // SetupDiDestroyDeviceInfoList() destroys a device information set
   // and frees all associated memory.

   SetupDiDestroyDeviceInfoList (hardwareDeviceInfo);
   free ( *UsbDevices );
   return hOut;
}


/******************************************************************************
Routine Description:

    Given a ptr to a driver-registered GUID, give us a string with the device name
    that can be used in a CreateFile() call.
    Actually briefly opens and closes the device and sets outBuf if successfull;
    returns FALSE if not

Arguments:

    pGuid:      ptr to GUID registered by the driver itself
    outNameBuf: the generated zero-terminated name for this device

Return Value:

    TRUE on success else FALSE

******************************************************************************/
BOOL GetUsbDeviceFileName( LPGUID  pGuid, char *outNameBuf)
{
    HANDLE hDev = OpenUsbDevice( pGuid, outNameBuf );
    if ( hDev != INVALID_HANDLE_VALUE )
    {
        CloseHandle( hDev );
        return TRUE;
    }
    return FALSE;

}


/******************************************************************************
Routine Description:

    Returns the number of devices attached to the system based on supplied GUID.

Arguments:

    pClassGuid:    ptr to GUID registered by the driver itself

Return Value:

    Number of devices attached to system

******************************************************************************/
DWORD QueryNumDevices( LPGUID pGuid )
{
    DWORD nDevices = 0;

    HDEVINFO hdInfo = SetupDiGetClassDevs( pGuid, NULL, NULL,
                                         DIGCF_PRESENT | DIGCF_DEVICEINTERFACE );

    if( hdInfo == INVALID_HANDLE_VALUE ) return( -1 );

    for( nDevices=0; ; nDevices++ )
    {
        SP_INTERFACE_DEVICE_DATA ifData;
        ifData.cbSize = sizeof(ifData);

        if( !SetupDiEnumInterfaceDevice( hdInfo, NULL, pGuid, nDevices, &ifData ) )
        {
            if( GetLastError() == ERROR_NO_MORE_ITEMS )
                break;
            else
            {
                SetupDiDestroyDeviceInfoList( hdInfo );
                return( -1 );
            }
        }
    }

    SetupDiDestroyDeviceInfoList( hdInfo );

    return( nDevices );
}


/******************************************************************************
Routine Description:

    Opens a pipe handle based on the GUID and device number.

Arguments:

    pGuid:            points to the GUID that identifies the interface class
    nPipeType:        indicates which pipe we want to open
    dwDeviceNumber:    specifies which instance of the enumerated devices to open
    bUseAsyncIo:    specifies if we should use ASYNC (TRUE) or SYNC (FALSE) IO

Return Value:

    Device handle on success else NULL

******************************************************************************/
HANDLE OpenDeviceHandle( LPGUID pGuid, _PIPE_TYPE nPipeType, DWORD dwDeviceNumber, BOOL bUseAsyncIo /*, char* pPhysicalDeviceName*/)
{
    char pPhysicalDeviceName[1024];
    DWORD dwError = ERROR_SUCCESS;
    HANDLE hDev = INVALID_HANDLE_VALUE;
    g_bUseAsyncIo = bUseAsyncIo;

    // see if there are of these attached to the system
    HDEVINFO hdInfo = SetupDiGetClassDevs( pGuid, NULL, NULL,
                                           DIGCF_PRESENT | DIGCF_DEVICEINTERFACE );

    if( hdInfo == INVALID_HANDLE_VALUE )
    {
        NOISY(("Failed to OpenDeviceHandle\n", GetLastError()));
        return INVALID_HANDLE_VALUE;
    }

    SP_INTERFACE_DEVICE_DATA ifData;
    ifData.cbSize = sizeof(ifData);

    // see if the device with the corresponding DeviceNumber is present
    if( !SetupDiEnumInterfaceDevice( hdInfo, NULL, pGuid, dwDeviceNumber, &ifData ) )
    {
        NOISY(("Failed to OpenDeviceHandle\n", GetLastError()));
        SetupDiDestroyDeviceInfoList( hdInfo );
        return INVALID_HANDLE_VALUE;
    }

    DWORD dwNameLength = 0;

    // find out how many bytes to malloc for the DeviceName.
    SetupDiGetInterfaceDeviceDetail( hdInfo, &ifData, NULL, 0, &dwNameLength, NULL );

    // we need to account for the pipe name so add to the length here
    dwNameLength += 32;

    PSP_INTERFACE_DEVICE_DETAIL_DATA detail = (PSP_INTERFACE_DEVICE_DETAIL_DATA)malloc( dwNameLength );

    if( !detail )
    {
        NOISY(("Failed to OpenDeviceHandle\n", GetLastError()));
        SetupDiDestroyDeviceInfoList( hdInfo );
        return INVALID_HANDLE_VALUE;
    }

    detail->cbSize = sizeof( SP_INTERFACE_DEVICE_DETAIL_DATA );

    // get the DeviceName
    if( !SetupDiGetInterfaceDeviceDetail( hdInfo, &ifData, detail, dwNameLength, NULL, NULL ) )
    {
        NOISY(("Failed to OpenDeviceHandle\n", GetLastError()));
        free( (PVOID)detail );
        SetupDiDestroyDeviceInfoList( hdInfo );
        return INVALID_HANDLE_VALUE;
    }

    char szDeviceName[MAX_PATH];
    strncpy_s( szDeviceName, sizeof(szDeviceName), detail->DevicePath, sizeof(szDeviceName) ); //spc
    free( (PVOID) detail );
    SetupDiDestroyDeviceInfoList( hdInfo );
    strcpy(pPhysicalDeviceName, szDeviceName);
    // check the GUID
    if ( GUID_BULKLDI == *pGuid )
    {
        strcat_s (szDeviceName, sizeof(szDeviceName), "\\" );  // spc convert to safe

        // if caller wants the read pipe
        if ( READ_PIPE == nPipeType )
        {
            if ( bUseAsyncIo )
            {
                g_hReadEvent = CreateEvent(    NULL,    // no security attribute
                                            TRUE,    // manual-reset event
                                            TRUE,    // initial state = signaled
                                            NULL);   // unnamed event object

                g_ReadOverlapped.hEvent = g_hReadEvent;
            }

            strcat_s (szDeviceName,sizeof(szDeviceName),"PIPE_0x00" );        // spc
        }

        // else if they want the write pipe
        else if ( WRITE_PIPE == nPipeType )
        {
            if ( bUseAsyncIo )
            {
                g_hWriteEvent = CreateEvent(NULL,    // no security attribute
                                            TRUE,    // manual-reset event
                                            TRUE,    // initial state = signaled
                                            NULL);   // unnamed event object

                g_WriteOverlapped.hEvent = g_hWriteEvent;
            }


            strcat_s (szDeviceName, sizeof(szDeviceName), "PIPE_0x01" );        // spc
        }

        // else unsupported pipe
        else
        {
            NOISY("Unsupported pipe\n");
            return INVALID_HANDLE_VALUE;
        }
    }

    // else unsupported device
    else
    {
        NOISY("Unsupported device\n");
        return INVALID_HANDLE_VALUE;
    }

    // determine the attributes
    DWORD dwAttribs;
    if ( bUseAsyncIo )
        dwAttribs = FILE_FLAG_OVERLAPPED;
    else
        dwAttribs = FILE_ATTRIBUTE_NORMAL;

    // open the driver with this DeviceName
    hDev = CreateFile(    szDeviceName,
                        GENERIC_READ | GENERIC_WRITE,
                        FILE_SHARE_READ | FILE_SHARE_WRITE,
                        NULL,
                        OPEN_EXISTING,
                        dwAttribs,
                        NULL);

    if (hDev == INVALID_HANDLE_VALUE)
    {
        NOISY("Failed to open device");
    }
    else
    {
        NOISY("Opened successfully.\n");
    }

    // return handle
    return hDev;
}


/******************************************************************************
Routine Description:

    Reads data from the pipe

Arguments:

    hRead:                    handle to read pipe
    lpBuffer:                pointer to buffer for data
    nNumberOfBytesToRead:    number of bytes we want to read
    lpNumberOfBytesRead:    number of bytes we read

Return Value:

    Status of operation

******************************************************************************/
BOOL ReadPipe(HANDLE hRead, LPVOID lpBuffer, DWORD nNumberOfBytesToRead, LPDWORD lpNumberOfBytesRead)
{
    BOOL status = TRUE;

    // if ASYNC
    if ( g_bUseAsyncIo )
    {
        // read file
        ReadFile(hRead, lpBuffer, nNumberOfBytesToRead, lpNumberOfBytesRead, &g_ReadOverlapped);

        // wait for the event
        if (WaitForSingleObject(g_hReadEvent, MAX_IO_WAIT) != WAIT_OBJECT_0)
        {
            status = FALSE;
        }

        // get the result of the operation
        if (!GetOverlappedResult(hRead, &g_ReadOverlapped, lpNumberOfBytesRead, FALSE))
        {
            DWORD result = GetLastError();

            status = FALSE;
        }

        // if we failed, cancel all I/O for this handle in this thread
        if (!status)
            CancelIo(hRead);

        // make sure we read all the data we wanted to
        if ((*lpNumberOfBytesRead > 0) && (*lpNumberOfBytesRead != nNumberOfBytesToRead))
            status = FALSE;
    }

    // else SYNC
    else
    {
        // read file
        status = ReadFile(hRead, lpBuffer, nNumberOfBytesToRead, lpNumberOfBytesRead, NULL);
    }

    return status;
}


/******************************************************************************
Routine Description:

    Writes data to the pipe

Arguments:

    hWrite:                    handle to write pipe
    lpBuffer:                pointer to buffer for data
    nNumberOfBytesToWrite:    number of bytes we want to write
    lpNumberOfBytesWritten:    number of bytes we wrote

Return Value:

    Status of operation

******************************************************************************/
BOOL WritePipe(HANDLE hWrite, LPCVOID lpBuffer, DWORD nNumberOfBytesToWrite, LPDWORD lpNumberOfBytesWritten)
{
    BOOL status = TRUE;

    // if ASYNC
    if ( g_bUseAsyncIo )
    {
        // write file
        WriteFile(hWrite, lpBuffer, nNumberOfBytesToWrite, lpNumberOfBytesWritten, &g_WriteOverlapped);

        // wait for the event
        if (WaitForSingleObject(g_hWriteEvent, MAX_IO_WAIT) != WAIT_OBJECT_0)
        {
            status = FALSE;
        }

        // get the result of the operation
        if (!GetOverlappedResult(hWrite, &g_WriteOverlapped, lpNumberOfBytesWritten, FALSE))
        {
            status = FALSE;
        }

        // if we failed, cancel all I/O for this handle in this thread
        if (!status)
            CancelIo(hWrite);

        // make sure we wrote all the data we wanted to
        if (*lpNumberOfBytesWritten != nNumberOfBytesToWrite)
            status = FALSE;
    }

    // else SYNC
    else
    {
        // write file
        status = WriteFile(hWrite, lpBuffer, nNumberOfBytesToWrite, lpNumberOfBytesWritten, NULL);
    }

    return status;
}
#endif


#if 0
    #define NOISY(_x_) printf _x_ ;
#else
    #define NOISY(_x_) ;
#endif

/******************************************************************************
Routine Description:

    Given the HardwareDeviceInfo, representing a handle to the plug and
    play information, and deviceInfoData, representing a specific usb device,
    open that device and fill in all the relevant information in the given
    USB_DEVICE_DESCRIPTOR structure.

Arguments:

    HardwareDeviceInfo:  handle to info obtained from Pnp mgr via SetupDiGetClassDevs()
    DeviceInfoData:      ptr to info obtained via SetupDiEnumInterfaceDevice()

Return Value:

    return HANDLE if the open and initialization was successfull,
    else INVLAID_HANDLE_VALUE.

******************************************************************************/
HANDLE USBDriverInterface::OpenOneDevice (
    IN       HDEVINFO                    HardwareDeviceInfo,
    IN       PSP_INTERFACE_DEVICE_DATA   DeviceInfoData,
    IN         char *devName
    )
{
    PSP_INTERFACE_DEVICE_DETAIL_DATA     functionClassDeviceData = NULL;
    ULONG                                predictedLength = 0;
    ULONG                                requiredLength = 0;
    HANDLE                                 hOut = INVALID_HANDLE_VALUE;

    m_bWasConnectionRefused = FALSE;

    //
    // allocate a function class device data structure to receive the
    // goods about this particular device.
    //
    SetupDiGetInterfaceDeviceDetail (
            HardwareDeviceInfo,
            DeviceInfoData,
            NULL,            // probing so no output buffer yet
            0,                // probing so output buffer length of zero
            &requiredLength,
            NULL);            // not interested in the specific dev-node


    predictedLength = requiredLength;

    functionClassDeviceData = (_SP_DEVICE_INTERFACE_DETAIL_DATA_A * )malloc (predictedLength);
    if(NULL == functionClassDeviceData) {
        return INVALID_HANDLE_VALUE;
    }
    functionClassDeviceData->cbSize = sizeof (SP_INTERFACE_DEVICE_DETAIL_DATA);

    //
    // Retrieve the information from Plug and Play.
    //
    if (! SetupDiGetInterfaceDeviceDetail (
               HardwareDeviceInfo,
               DeviceInfoData,
               functionClassDeviceData,
               predictedLength,
               &requiredLength,
               NULL)) {
        free( functionClassDeviceData );
        return INVALID_HANDLE_VALUE;
    }

    strcpy_s( devName, 128, functionClassDeviceData->DevicePath) ; // added strcpy_s SPC
    NOISY(( "Attempting to open %s\n", devName ));

    hOut = CreateFile (
                  functionClassDeviceData->DevicePath,
                  GENERIC_READ | GENERIC_WRITE,
                  FILE_SHARE_READ | FILE_SHARE_WRITE,
                  NULL,                // no SECURITY_ATTRIBUTES structure
                  OPEN_EXISTING,    // No special create flags
                  0,                // No special attributes
                  NULL);            // No template file

    if (INVALID_HANDLE_VALUE == hOut)
    {
        DWORD err = GetLastError();
        if (err == ERROR_CONNECTION_REFUSED)
        {
            m_bWasConnectionRefused = TRUE;
        }
        printf( "FAILED to open %s, last error = %d\n", devName, err );
    }
    free( functionClassDeviceData );
    return hOut;
}


/******************************************************************************
Routine Description:

   Do the required PnP things in order to find
   the next available proper device in the system at this time.

Arguments:

    pGuid:      ptr to GUID registered by the driver itself
    outNameBuf: the generated name for this device

Return Value:

    return HANDLE if the open and initialization was successful,
    else INVLAID_HANDLE_VALUE.
******************************************************************************/
HANDLE USBDriverInterface::OpenUsbDevice( LPGUID  pGuid, char *outNameBuf)
{
   ULONG NumberDevices;
   HANDLE hOut = INVALID_HANDLE_VALUE;
   HDEVINFO                 hardwareDeviceInfo;
   SP_INTERFACE_DEVICE_DATA deviceInfoData;
   ULONG                    i;
   BOOLEAN                  done;
   PUSB_DEVICE_DESCRIPTOR   usbDeviceInst;
   PUSB_DEVICE_DESCRIPTOR    *UsbDevices = &usbDeviceInst;
   PUSB_DEVICE_DESCRIPTOR   tempDevDesc;

   *UsbDevices = NULL;
   tempDevDesc = NULL;
   NumberDevices = 0;

   //
   // Open a handle to the plug and play dev node.
   // SetupDiGetClassDevs() returns a device information set that contains info on all
   // installed devices of a specified class.
   //
   hardwareDeviceInfo = SetupDiGetClassDevs (
                           pGuid,
                           NULL,                        // Define no enumerator (global)
                           NULL,                        // Define no
                           (DIGCF_PRESENT |                // Only Devices present
                            DIGCF_INTERFACEDEVICE));    // Function class devices.

   //
   // Take a wild guess at the number of devices we have;
   // Be prepared to realloc and retry if there are more than we guessed
   //
   NumberDevices = 4;
   done = FALSE;
   deviceInfoData.cbSize = sizeof (SP_INTERFACE_DEVICE_DATA);

   i=0;
   while (!done) {
      NumberDevices *= 2;

      if (*UsbDevices) {
            tempDevDesc =
               (PUSB_DEVICE_DESCRIPTOR)realloc (*UsbDevices, (NumberDevices * sizeof (USB_DEVICE_DESCRIPTOR)));
            if(tempDevDesc) {
                *UsbDevices = tempDevDesc;
                tempDevDesc = NULL;
            }
            else {
                free(*UsbDevices);
                *UsbDevices = NULL;
            }
      } else {
         *UsbDevices = (PUSB_DEVICE_DESCRIPTOR)calloc (NumberDevices, sizeof (USB_DEVICE_DESCRIPTOR));
      }

      if (NULL == *UsbDevices) {

         // SetupDiDestroyDeviceInfoList destroys a device information set
         // and frees all associated memory.

         SetupDiDestroyDeviceInfoList (hardwareDeviceInfo);
         return INVALID_HANDLE_VALUE;
      }

      usbDeviceInst = *UsbDevices + i;

      for (; i < NumberDevices; i++) {

         // SetupDiEnumDeviceInterfaces() returns information about device interfaces
         // exposed by one or more devices. Each call returns information about one interface;
         // the routine can be called repeatedly to get information about several interfaces
         // exposed by one or more devices.

         if (SetupDiEnumDeviceInterfaces (hardwareDeviceInfo,
                                         0, // We don't care about specific PDOs
                                                                                 pGuid,
                                         i,
                                         &deviceInfoData)) {

            hOut = OpenOneDevice (hardwareDeviceInfo, &deviceInfoData, outNameBuf);
                        if ( hOut != INVALID_HANDLE_VALUE ) {
               done = TRUE;
               break;
                        }
         } else {
            if (ERROR_NO_MORE_ITEMS == GetLastError()) {
               done = TRUE;
               break;
            }
         }
      }
   }

   NumberDevices = i;

   // SetupDiDestroyDeviceInfoList() destroys a device information set
   // and frees all associated memory.

   SetupDiDestroyDeviceInfoList (hardwareDeviceInfo);
   free ( *UsbDevices );
   return hOut;
}
BOOL USBDriverInterface::Close()
{

    return TRUE;
}


/******************************************************************************
Routine Description:

    Given a ptr to a driver-registered GUID, give us a string with the device name
    that can be used in a CreateFile() call.
    Actually briefly opens and closes the device and sets outBuf if successfull;
    returns FALSE if not

Arguments:

    pGuid:      ptr to GUID registered by the driver itself
    outNameBuf: the generated zero-terminated name for this device

Return Value:

    TRUE on success else FALSE

******************************************************************************/
BOOL USBDriverInterface::GetUsbDeviceFileName( LPGUID  pGuid, char *outNameBuf)
{
    HANDLE hDev = OpenUsbDevice( pGuid, outNameBuf );
    if ( hDev != INVALID_HANDLE_VALUE )
    {
        CloseHandle( hDev );
        return TRUE;
    }
    return FALSE;

}


/******************************************************************************
Routine Description:

    Returns the number of devices attached to the system based on supplied GUID.

Arguments:

    pClassGuid:    ptr to GUID registered by the driver itself

Return Value:

    Number of devices attached to system

******************************************************************************/
DWORD USBDriverInterface::QueryNumDevices( LPGUID pGuid )
{
    DWORD nDevices = 0;

    HDEVINFO hdInfo = SetupDiGetClassDevs( pGuid, NULL, NULL,
                                         DIGCF_PRESENT | DIGCF_DEVICEINTERFACE );

    if( hdInfo == INVALID_HANDLE_VALUE ) return( -1 );

    for( nDevices=0; ; nDevices++ )
    {
        SP_INTERFACE_DEVICE_DATA ifData;
        ifData.cbSize = sizeof(ifData);

        if( !SetupDiEnumInterfaceDevice( hdInfo, NULL, pGuid, nDevices, &ifData ) )
        {
            if( GetLastError() == ERROR_NO_MORE_ITEMS )
                break;
            else
            {
                SetupDiDestroyDeviceInfoList( hdInfo );
                return( -1 );
            }
        }
    }

    SetupDiDestroyDeviceInfoList( hdInfo );

    return( nDevices );
}

DWORD USBDriverInterface::QueryNumNonStrDevices( LPGUID pGuid )
{
    DWORD nDevices = 0;

    HDEVINFO hdInfo = SetupDiGetClassDevs( pGuid, NULL, NULL,
                                         DIGCF_PRESENT | DIGCF_DEVICEINTERFACE );

    if( hdInfo == INVALID_HANDLE_VALUE ) return( -1 );

    for( nDevices=0; ; nDevices++ )
    {
        SP_INTERFACE_DEVICE_DATA ifData;
        ifData.cbSize = sizeof(ifData);

        if( !SetupDiEnumInterfaceDevice( hdInfo, NULL, pGuid, nDevices, &ifData ) )
        {
            if( GetLastError() == ERROR_NO_MORE_ITEMS )
                break;
            else
            {
                SetupDiDestroyDeviceInfoList( hdInfo );
                return( -1 );
            }
        }
    }

    SetupDiDestroyDeviceInfoList( hdInfo );

    return( nDevices );
}


/******************************************************************************
Routine Description:

    Opens a pipe handle based on the GUID and device number.

Arguments:

    pGuid:            points to the GUID that identifies the interface class
    nPipeType:        indicates which pipe we want to open
    dwDeviceNumber:    specifies which instance of the enumerated devices to open
    bUseAsyncIo:    specifies if we should use ASYNC (TRUE) or SYNC (FALSE) IO

Return Value:

    Device handle on success else NULL

******************************************************************************/
HANDLE USBDriverInterface::OpenDeviceHandle( LPGUID pGuid, _PIPE_TYPE nPipeType, DWORD dwDeviceNumber, BOOL bUseAsyncIo , char* pPhysicalDeviceName)
{
    DWORD dwError = ERROR_SUCCESS;
    HANDLE hDev = INVALID_HANDLE_VALUE;
    m_bWasConnectionRefused = FALSE;
    g_bUseAsyncIo = bUseAsyncIo;

    // see if there are of these attached to the system
    HDEVINFO hdInfo = SetupDiGetClassDevs( pGuid, NULL, NULL,
                                           DIGCF_PRESENT | DIGCF_DEVICEINTERFACE );

    if( hdInfo == INVALID_HANDLE_VALUE )
    {
        NOISY(("Failed to OpenDeviceHandle\n", GetLastError()));
        return INVALID_HANDLE_VALUE;
    }

    SP_INTERFACE_DEVICE_DATA ifData;
    ifData.cbSize = sizeof(ifData);

    // see if the device with the corresponding DeviceNumber is present
    if( !SetupDiEnumInterfaceDevice( hdInfo, NULL, pGuid, dwDeviceNumber, &ifData ) )
    {
        NOISY(("Failed to OpenDeviceHandle\n", GetLastError()));
        SetupDiDestroyDeviceInfoList( hdInfo );
        return INVALID_HANDLE_VALUE;
    }

    DWORD dwNameLength = 0;

    // find out how many bytes to malloc for the DeviceName.
    SetupDiGetInterfaceDeviceDetail( hdInfo, &ifData, NULL, 0, &dwNameLength, NULL );

    // we need to account for the pipe name so add to the length here
    dwNameLength += 32;

    PSP_INTERFACE_DEVICE_DETAIL_DATA detail = (PSP_INTERFACE_DEVICE_DETAIL_DATA)malloc( dwNameLength );

    if( !detail )
    {
        NOISY(("Failed to OpenDeviceHandle\n", GetLastError()));
        SetupDiDestroyDeviceInfoList( hdInfo );
        return INVALID_HANDLE_VALUE;
    }

    detail->cbSize = sizeof( SP_INTERFACE_DEVICE_DETAIL_DATA );

    // get the DeviceName
    if( !SetupDiGetInterfaceDeviceDetail( hdInfo, &ifData, detail, dwNameLength, NULL, NULL ) )
    {
        NOISY(("Failed to OpenDeviceHandle\n", GetLastError()));
        free( (PVOID)detail );
        SetupDiDestroyDeviceInfoList( hdInfo );
        return INVALID_HANDLE_VALUE;
    }

    char szDeviceName[MAX_PATH];
    strncpy_s( szDeviceName, sizeof(szDeviceName), detail->DevicePath, sizeof(szDeviceName) ); //spc
    free( (PVOID) detail );
    BOOL BRC = SetupDiDestroyDeviceInfoList( hdInfo );

    // check the GUID
    strcpy(pPhysicalDeviceName, szDeviceName);

    if ( GUID_BULKLDI == *pGuid )
    {
        strcat_s (szDeviceName, sizeof(szDeviceName), "\\" );  // spc convert to safe

        // if caller wants the read pipe
        if ( READ_PIPE == nPipeType )
        {
            if(g_hReadEvent)
            {
                CloseHandle(g_hReadEvent);
            }
            if ( bUseAsyncIo )
            {
                g_hReadEvent = CreateEvent(    NULL,    // no security attribute
                                            TRUE,    // manual-reset event
                                            TRUE,    // initial state = signaled
                                            NULL);   // unnamed event object

                g_ReadOverlapped.hEvent = g_hReadEvent;
            }

            strcat_s (szDeviceName,sizeof(szDeviceName),"PIPE_0x00" );        // spc
        }

        // else if they want the write pipe
        else if ( WRITE_PIPE == nPipeType )
        {
            if(g_hWriteEvent)
            {
                CloseHandle(g_hWriteEvent);
            }
            if ( bUseAsyncIo )
            {
                g_hWriteEvent = CreateEvent(NULL,    // no security attribute
                                            TRUE,    // manual-reset event
                                            TRUE,    // initial state = signaled
                                            NULL);   // unnamed event object

                g_WriteOverlapped.hEvent = g_hWriteEvent;
            }

            strcat_s (szDeviceName, sizeof(szDeviceName), "PIPE_0x01" );        // spc
        }

        // else unsupported pipe
        else
        {
            NOISY("Unsupported pipe\n");
            return INVALID_HANDLE_VALUE;
        }
    }

    // else unsupported device
    else
    {
        NOISY("Unsupported device\n");
        return INVALID_HANDLE_VALUE;
    }

    // determine the attributes
    DWORD dwAttribs;
    if ( bUseAsyncIo )
        dwAttribs = FILE_FLAG_OVERLAPPED;
    else
        dwAttribs = FILE_ATTRIBUTE_NORMAL;

    // open the driver with this DeviceName
    hDev = CreateFile(    szDeviceName,
                        GENERIC_READ | GENERIC_WRITE,
                        FILE_SHARE_READ | FILE_SHARE_WRITE,
                        NULL,
                        OPEN_EXISTING,
                        dwAttribs,
                        NULL);

    if (hDev == INVALID_HANDLE_VALUE)
    {
        DWORD err = GetLastError();
        if (err == ERROR_CONNECTION_REFUSED)
        {
            m_bWasConnectionRefused = TRUE;
        }
        NOISY("Failed to open device");
    }
    else
    {
        NOISY("Opened successfully.\n");
    }
    // return handle
    return hDev;
}


/******************************************************************************
Routine Description:

    Reads data from the pipe

Arguments:

    hRead:                    handle to read pipe
    lpBuffer:                pointer to buffer for data
    nNumberOfBytesToRead:    number of bytes we want to read
    lpNumberOfBytesRead:    number of bytes we read

Return Value:

    Status of operation

******************************************************************************/
BOOL USBDriverInterface::ReadPipe(HANDLE hRead, LPVOID lpBuffer, DWORD nNumberOfBytesToRead, LPDWORD lpNumberOfBytesRead)
{
    BOOL status = TRUE;

    // if ASYNC
    if ( g_bUseAsyncIo )
    {
        // read file
        ReadFile(hRead, lpBuffer, nNumberOfBytesToRead, lpNumberOfBytesRead, &g_ReadOverlapped);

        // wait for the event
        if (WaitForSingleObject(g_hReadEvent, MAX_IO_WAIT) != WAIT_OBJECT_0)
        {
            status = FALSE;
        }

        // get the result of the operation
        if (!GetOverlappedResult(hRead, &g_ReadOverlapped, lpNumberOfBytesRead, FALSE))
        {
            DWORD result = GetLastError();

            status = FALSE;
        }

        // if we failed, cancel all I/O for this handle in this thread
        if (!status)
            CancelIo(hRead);

        // make sure we read all the data we wanted to
        if ((*lpNumberOfBytesRead > 0) && (*lpNumberOfBytesRead != nNumberOfBytesToRead))
            status = FALSE;
    }

    // else SYNC
    else
    {
        // read file
        status = ReadFile(hRead, lpBuffer, nNumberOfBytesToRead, lpNumberOfBytesRead, NULL);
    }

    return status;
}


/******************************************************************************
Routine Description:

    Writes data to the pipe

Arguments:

    hWrite:                    handle to write pipe
    lpBuffer:                pointer to buffer for data
    nNumberOfBytesToWrite:    number of bytes we want to write
    lpNumberOfBytesWritten:    number of bytes we wrote

Return Value:

    Status of operation

******************************************************************************/
BOOL USBDriverInterface::WritePipe(HANDLE hWrite, LPCVOID lpBuffer, DWORD nNumberOfBytesToWrite, LPDWORD lpNumberOfBytesWritten)
{
    BOOL status = TRUE;

    // if ASYNC
    if ( g_bUseAsyncIo )
    {
        // write file
        WriteFile(hWrite, lpBuffer, nNumberOfBytesToWrite, lpNumberOfBytesWritten, &g_WriteOverlapped);

        // wait for the event
        if (WaitForSingleObject(g_hWriteEvent, MAX_IO_WAIT) != WAIT_OBJECT_0)
        {
            status = FALSE;
        }

        // get the result of the operation
        if (!GetOverlappedResult(hWrite, &g_WriteOverlapped, lpNumberOfBytesWritten, FALSE))
        {
            status = FALSE;
        }

        // if we failed, cancel all I/O for this handle in this thread
        if (!status)
            CancelIo(hWrite);

        // make sure we wrote all the data we wanted to
        if (*lpNumberOfBytesWritten != nNumberOfBytesToWrite)
            status = FALSE;
    }

    // else SYNC
    else
    {
        // write file
        status = WriteFile(hWrite, lpBuffer, nNumberOfBytesToWrite, lpNumberOfBytesWritten, NULL);
    }

    return status;
}

BOOL USBDriverInterface::WasConnectionRefused()
{
    return m_bWasConnectionRefused;
}

USBDriverInterface::USBDriverInterface()
{
    g_bUseAsyncIo = TRUE;                        // flag indicating if we're using ASYNC (TRUE) or SYNC (FALSE) IO
    g_hReadEvent = INVALID_HANDLE_VALUE;        // read event handle for ASYNC IO
    g_hWriteEvent = INVALID_HANDLE_VALUE;    // write event handle for ASYNC IO
    m_bWasConnectionRefused = FALSE;        // flag to indicate if the connection was refused
    memset(&g_ReadOverlapped, 0, sizeof(g_ReadOverlapped));
    memset(&g_WriteOverlapped, 0, sizeof(g_WriteOverlapped));
}
USBDriverInterface::~USBDriverInterface()
{
    if( g_hWriteEvent != INVALID_HANDLE_VALUE) CloseHandle(g_hWriteEvent);
    if( g_hReadEvent  != INVALID_HANDLE_VALUE) CloseHandle(g_hReadEvent);
}

#endif