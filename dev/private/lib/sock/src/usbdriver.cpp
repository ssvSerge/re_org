#if 1
// #ifdef __SUPPORT_WIN32_USB__

#include <windows.h>
#include <stdio.h>
#include <setupapi.h>
#include <usbdi.h>

#include <ldiguid.h>
#include <usbcmdset.h>
#include <usbdriver.h>

HANDLE USBDriverInterface::OpenOneDevice ( IN HDEVINFO HardwareDeviceInfo, IN PSP_INTERFACE_DEVICE_DATA DeviceInfoData, IN char *devName ) {

    PSP_INTERFACE_DEVICE_DETAIL_DATA  functionClassDeviceData = NULL;
    ULONG                             predictedLength = 0;
    ULONG                             requiredLength = 0;
    HANDLE                            hOut = INVALID_HANDLE_VALUE;

    SetupDiGetInterfaceDeviceDetail ( HardwareDeviceInfo, DeviceInfoData, NULL, 0, &requiredLength, NULL); 

    predictedLength = requiredLength;

    functionClassDeviceData = (PSP_INTERFACE_DEVICE_DETAIL_DATA) malloc (predictedLength);
    if (NULL == functionClassDeviceData) {
        return INVALID_HANDLE_VALUE;
    }
    functionClassDeviceData->cbSize = sizeof (SP_INTERFACE_DEVICE_DETAIL_DATA);

    if ( ! SetupDiGetInterfaceDeviceDetail ( HardwareDeviceInfo, DeviceInfoData, functionClassDeviceData, predictedLength, &requiredLength, NULL)) {
        free( functionClassDeviceData );
        return INVALID_HANDLE_VALUE;
    }

    strcpy_s( devName, 128, functionClassDeviceData->DevicePath) ; // added strcpy_s SPC

    hOut = CreateFile ( functionClassDeviceData->DevicePath, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);

    if (INVALID_HANDLE_VALUE == hOut) {
        // printf( "FAILED to open %s\n", devName );
    }

    free( functionClassDeviceData );
    return hOut;
}

HANDLE USBDriverInterface::OpenUsbDevice( LPGUID  pGuid, char *outNameBuf) {

   ULONG NumberDevices;
   HANDLE hOut = INVALID_HANDLE_VALUE;
   HDEVINFO                 hardwareDeviceInfo;
   SP_INTERFACE_DEVICE_DATA deviceInfoData;
   ULONG                    i;
   BOOLEAN                  done;
   PUSB_DEVICE_DESCRIPTOR   usbDeviceInst;
   PUSB_DEVICE_DESCRIPTOR   *UsbDevices = &usbDeviceInst;
   PUSB_DEVICE_DESCRIPTOR   tempDevDesc;

   *UsbDevices = NULL;
   tempDevDesc = NULL;
   NumberDevices = 0;

   hardwareDeviceInfo = SetupDiGetClassDevs ( pGuid, NULL, NULL, (DIGCF_PRESENT | DIGCF_INTERFACEDEVICE) );

   NumberDevices = 4;
   done = FALSE;
   deviceInfoData.cbSize = sizeof (SP_INTERFACE_DEVICE_DATA);

   i=0;

   while ( ! done ) {

      NumberDevices *= 2;

      if (*UsbDevices) {
            tempDevDesc = (PUSB_DEVICE_DESCRIPTOR)realloc (*UsbDevices, (NumberDevices * sizeof (USB_DEVICE_DESCRIPTOR)));
            if(tempDevDesc) {
                *UsbDevices = tempDevDesc;
                tempDevDesc = NULL;
            } else {
                free(*UsbDevices);
                *UsbDevices = NULL;
            }
      } else {
         *UsbDevices = (PUSB_DEVICE_DESCRIPTOR)calloc (NumberDevices, sizeof (USB_DEVICE_DESCRIPTOR));
      }

      if (NULL == *UsbDevices) {
         SetupDiDestroyDeviceInfoList (hardwareDeviceInfo);
         return INVALID_HANDLE_VALUE;
      }

      usbDeviceInst = *UsbDevices + i;

      for (; i < NumberDevices; i++) {

         if (SetupDiEnumDeviceInterfaces (hardwareDeviceInfo, 0, pGuid, i, &deviceInfoData)) {

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

   SetupDiDestroyDeviceInfoList (hardwareDeviceInfo);
   free ( *UsbDevices );
   return hOut;
}

BOOL USBDriverInterface::Close() {

    return TRUE;
}

BOOL USBDriverInterface::GetUsbDeviceFileName( LPGUID  pGuid, char *outNameBuf) {
    HANDLE hDev = OpenUsbDevice( pGuid, outNameBuf );
    if ( hDev != INVALID_HANDLE_VALUE )
    {
        CloseHandle( hDev );
        return TRUE;
    }
    return FALSE;

}

DWORD USBDriverInterface::QueryNumDevices( LPGUID pGuid ) {

    DWORD nDevices = 0;

    HDEVINFO hdInfo = SetupDiGetClassDevs( pGuid, NULL, NULL, DIGCF_PRESENT | DIGCF_DEVICEINTERFACE );

    if ( hdInfo == INVALID_HANDLE_VALUE ) {
        return (DWORD) (-1);
    }

    for ( nDevices=0; ; nDevices++ ) {
        SP_INTERFACE_DEVICE_DATA ifData;
        ifData.cbSize = sizeof(ifData);

        if ( !SetupDiEnumInterfaceDevice( hdInfo, NULL, pGuid, nDevices, &ifData ) ) {
            if ( GetLastError() == ERROR_NO_MORE_ITEMS ) {
                break;
            } else {
                SetupDiDestroyDeviceInfoList( hdInfo );
                return (DWORD) (-1);
            }
        }
    }

    SetupDiDestroyDeviceInfoList( hdInfo );

    return( nDevices );
}

DWORD USBDriverInterface::QueryNumNonStrDevices( LPGUID pGuid ) {

    DWORD nDevices = 0;

    HDEVINFO hdInfo = SetupDiGetClassDevs( pGuid, NULL, NULL, DIGCF_PRESENT | DIGCF_DEVICEINTERFACE );

    if ( hdInfo == INVALID_HANDLE_VALUE ) {
        return (DWORD) (-1);
    }

    for ( nDevices=0; ; nDevices++ ) {
        SP_INTERFACE_DEVICE_DATA ifData;
        ifData.cbSize = sizeof(ifData);

        if( !SetupDiEnumInterfaceDevice( hdInfo, NULL, pGuid, nDevices, &ifData ) ) {
            if( GetLastError() == ERROR_NO_MORE_ITEMS )
                break;
            else {
                SetupDiDestroyDeviceInfoList( hdInfo );
                return (DWORD) (-1);
            }
        }
    }

    SetupDiDestroyDeviceInfoList( hdInfo );

    return( nDevices );
}

HANDLE USBDriverInterface::OpenDeviceHandle( LPGUID pGuid, _PIPE_TYPE nPipeType, DWORD dwDeviceNumber, BOOL bUseAsyncIo , char* pPhysicalDeviceName) {

    // DWORD dwError = ERROR_SUCCESS;
    HANDLE hDev = INVALID_HANDLE_VALUE;
    g_bUseAsyncIo = bUseAsyncIo;

    HDEVINFO hdInfo = SetupDiGetClassDevs( pGuid, NULL, NULL, DIGCF_PRESENT | DIGCF_DEVICEINTERFACE );

    if ( hdInfo == INVALID_HANDLE_VALUE ) {
        return INVALID_HANDLE_VALUE;
    }

    SP_INTERFACE_DEVICE_DATA ifData;
    ifData.cbSize = sizeof(ifData);

    if ( ! SetupDiEnumInterfaceDevice( hdInfo, NULL, pGuid, dwDeviceNumber, &ifData ) ) {
        SetupDiDestroyDeviceInfoList( hdInfo );
        return INVALID_HANDLE_VALUE;
    }

    DWORD dwNameLength = 0;

    SetupDiGetInterfaceDeviceDetail( hdInfo, &ifData, NULL, 0, &dwNameLength, NULL );

    dwNameLength += 32;

    PSP_INTERFACE_DEVICE_DETAIL_DATA detail = (PSP_INTERFACE_DEVICE_DETAIL_DATA)malloc( dwNameLength );

    if ( !detail ) {
        SetupDiDestroyDeviceInfoList( hdInfo );
        return INVALID_HANDLE_VALUE;
    }

    detail->cbSize = sizeof( SP_INTERFACE_DEVICE_DETAIL_DATA );

    // get the DeviceName
    if( !SetupDiGetInterfaceDeviceDetail( hdInfo, &ifData, detail, dwNameLength, NULL, NULL ) ) {
        free( (PVOID)detail );
        SetupDiDestroyDeviceInfoList( hdInfo );
        return INVALID_HANDLE_VALUE;
    }

    char szDeviceName[MAX_PATH];
    strncpy_s( szDeviceName, sizeof(szDeviceName), detail->DevicePath, sizeof(szDeviceName) ); //spc
    free( (PVOID) detail );
    SetupDiDestroyDeviceInfoList( hdInfo );

    strcpy(pPhysicalDeviceName, szDeviceName);

    if ( GUID_BULKLDI == *pGuid ) {
        strcat_s (szDeviceName, sizeof(szDeviceName), "\\" );  // spc convert to safe

        if ( READ_PIPE == nPipeType ) {
            if (g_hReadEvent) {
                CloseHandle(g_hReadEvent);
            }

            if ( bUseAsyncIo ) {
                g_hReadEvent = CreateEvent( NULL, TRUE, TRUE, NULL);
                g_ReadOverlapped.hEvent = g_hReadEvent;
            }

            strcat_s (szDeviceName,sizeof(szDeviceName),"PIPE_0x00" );
        } else 
        if ( WRITE_PIPE == nPipeType ) {
            if ( g_hWriteEvent ) {
                CloseHandle(g_hWriteEvent);
            }
            if ( bUseAsyncIo ) {
                g_hWriteEvent = CreateEvent(NULL, TRUE, TRUE, NULL);
                g_WriteOverlapped.hEvent = g_hWriteEvent;
            }

            strcat_s (szDeviceName, sizeof(szDeviceName), "PIPE_0x01" );        // spc
        } else {
            return INVALID_HANDLE_VALUE;
        }
    } else {
        return INVALID_HANDLE_VALUE;
    }

    DWORD dwAttribs;

    if ( bUseAsyncIo ) {
        dwAttribs = FILE_FLAG_OVERLAPPED;
    } else {
        dwAttribs = FILE_ATTRIBUTE_NORMAL;
    }

    hDev = CreateFile( szDeviceName, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, dwAttribs, NULL);

    if (hDev == INVALID_HANDLE_VALUE) {
        // NOISY("Failed to open device");
    } else {
        // NOISY("Opened successfully.\n");
    }

    return hDev;
}

BOOL USBDriverInterface::ReadPipe(HANDLE hRead, LPVOID lpBuffer, DWORD nNumberOfBytesToRead, LPDWORD lpNumberOfBytesRead) {

    BOOL status = TRUE;

    if ( g_bUseAsyncIo ) {

        ReadFile(hRead, lpBuffer, nNumberOfBytesToRead, lpNumberOfBytesRead, &g_ReadOverlapped);

        if ( WaitForSingleObject(g_hReadEvent, MAX_IO_WAIT) != WAIT_OBJECT_0) {
            status = FALSE;
        }

        if ( ! GetOverlappedResult(hRead, &g_ReadOverlapped, lpNumberOfBytesRead, FALSE) ) {
            // DWORD result = GetLastError();
            status = FALSE;
        }

        if ( ! status ) {
            CancelIo(hRead);
        }

        if ( (*lpNumberOfBytesRead > 0) && (*lpNumberOfBytesRead != nNumberOfBytesToRead) ) {
            status = FALSE;
        }

    } else {
        status = ReadFile(hRead, lpBuffer, nNumberOfBytesToRead, lpNumberOfBytesRead, NULL);
    }

    return status;
}

BOOL USBDriverInterface::WritePipe(HANDLE hWrite, LPCVOID lpBuffer, DWORD nNumberOfBytesToWrite, LPDWORD lpNumberOfBytesWritten) {

    BOOL status = TRUE;

    if ( g_bUseAsyncIo ) {

        WriteFile(hWrite, lpBuffer, nNumberOfBytesToWrite, lpNumberOfBytesWritten, &g_WriteOverlapped);

        if (WaitForSingleObject(g_hWriteEvent, MAX_IO_WAIT) != WAIT_OBJECT_0) {
            status = FALSE;
        }

        if (!GetOverlappedResult(hWrite, &g_WriteOverlapped, lpNumberOfBytesWritten, FALSE)) {
            status = FALSE;
        }

        if (!status) {
            CancelIo(hWrite);
        }

        if (*lpNumberOfBytesWritten != nNumberOfBytesToWrite) {
            status = FALSE;
        }

    } else {
        status = WriteFile(hWrite, lpBuffer, nNumberOfBytesToWrite, lpNumberOfBytesWritten, NULL);
    }

    return status;
}

USBDriverInterface::USBDriverInterface() {
    g_bUseAsyncIo = TRUE;                   // flag indicating if we're using ASYNC (TRUE) or SYNC (FALSE) IO
    g_hReadEvent = INVALID_HANDLE_VALUE;    // read event handle for ASYNC IO
    g_hWriteEvent = INVALID_HANDLE_VALUE;   // write event handle for ASYNC IO
    memset(&g_ReadOverlapped, 0, sizeof(g_ReadOverlapped));
    memset(&g_WriteOverlapped, 0, sizeof(g_WriteOverlapped));
}

USBDriverInterface::~USBDriverInterface() {
    if( g_hWriteEvent != INVALID_HANDLE_VALUE) CloseHandle(g_hWriteEvent);
    if( g_hReadEvent  != INVALID_HANDLE_VALUE) CloseHandle(g_hReadEvent);
}

#endif

