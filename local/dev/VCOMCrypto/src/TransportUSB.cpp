/***************************************************************************************/
// �Copyright 2020 HID Global Corporation/ASSA ABLOY AB. ALL RIGHTS RESERVED.
//
// For a list of applicable patents and patents pending, visit www.hidglobal.com/patents/
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
// FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
// COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
// IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
// CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//
//
/***************************************************************************************/
#include "windows.h"
#include <stdio.h>
#include <vector>

#include "TransportUSB.h"
#include "VCOMBase.h"
#include "V100DeviceHandler.h"
#include "Device.h"

#undef UNICODE
#include <setupapi.h>
#include <usbdi.h>
#include "ldiguid.h"
#include "usbcmdset.h"
#include "usbdriver.h"

typedef std::vector<unsigned char>  raw_data_t;
typedef std::vector<raw_data_t>     raw_set_t;

#define SHOW_DEBUG_OUTPUT 0        // Set this to non-zero to see debug output

int WriteBytesEx( HANDLE hDev, const void* ptr, unsigned int len, int unused ) {

    unsigned char* out_stream = (unsigned char*)ptr;

    printf("[ ");
    for (unsigned int i = 0; i < len; i++) {
        printf("0x%.2X", out_stream[i]);
        if (i < len-1) {
            printf(", ");
        }
    }
    printf("]\r\n");

    return 1;
}

int ReadBytesEx(HANDLE hDev, void* ptr, unsigned int len, int unused ) {

    static int pos = 0;

    bool cont = false;

    raw_data_t read_stream_0001 = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x12, 0x00, 0x00, 0x00 };
    raw_data_t read_stream_0002 = { 0x0D, 0x56, 0x49, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00 };

    raw_set_t  read_set = { read_stream_0001, read_stream_0002 };

    while ( !cont ) {
        Sleep(1000);
    }

    if (pos != -1) {

        raw_data_t tmp = read_set[pos];
        unsigned char* dst = (unsigned char*)ptr;

        for (unsigned int i = 0; i < tmp.size(); i++) {
            dst[i] = tmp[i];
        }

    }

    pos++;
    return 1;
}



TransportUSB::TransportUSB(void)
{
    memset(hPhysicalDeviceID, 0, 2048);
    handleToClose = 0;
    hMutexID = NULL;
    pUSB = new USBDriverInterface();
}

TransportUSB::~TransportUSB(void)
{
    delete pUSB;
    pUSB = NULL;
}

// pure virtual derived of ITransport
uint TransportUSB::Initialize(V100_DEVICE_TRANSPORT_INFO* pTransport)
{
    int nNumDev = 0;

    if (GEN_OK != V100_Get_Num_USB_Devices(&nNumDev))
        return USB_INVALID_DEVICE;

    if (nNumDev == 0)
        return USB_NO_DEVICES_FOUND;

    Device* device = reinterpret_cast<Device*>(pTransport->hInstance);

    if (NULL == device)
        return USB_INVALID_DEVICE;

    if (device->m_nMemberIndex >= uint(nNumDev))
        return USB_INVALID_DEVICE;

    // open write handle
    pTransport->hWrite = pUSB->OpenDeviceHandle((LPGUID)&GUID_BULKLDI,
        WRITE_PIPE,
        device->m_nMemberIndex,//pTransport->DeviceNumber,
        TRUE,
        hPhysicalDeviceID);        // USE ASYNC
    if (pTransport->hWrite == INVALID_HANDLE_VALUE)
    {
        if (TRUE == pUSB->WasConnectionRefused())
        {
            return USB_CONNECTION_REFUSED;
        }
        else
        {
            return USB_WRITE_PIPE_ERROR;
        }
    }


    // open read handle
    pTransport->hRead = pUSB->OpenDeviceHandle((LPGUID)&GUID_BULKLDI,
        READ_PIPE,
        device->m_nMemberIndex,//pTransport->DeviceNumber,
        TRUE,
        hPhysicalDeviceID);

    if (pTransport->hWrite == INVALID_HANDLE_VALUE)
    {
        if (TRUE == pUSB->WasConnectionRefused())
        {
            return USB_CONNECTION_REFUSED;
        }
        else
        {
            return USB_READ_PIPE_ERROR;
        }
    }

    // Must copy over the first 4 characters to 1) avoid collisions,
    // and 2) Make sure that you can use the name as a mutex handle.
    strncpy(hPhysicalDeviceID, "LUMI", 4);

    return USB_OK;
}

HANDLE TransportUSB::GetMutexId()
{
    if (strcmp(hPhysicalDeviceID, "") == 0)
        return NULL;

    if(hMutexID == NULL)
        hMutexID = CreateMutexA(NULL, FALSE, hPhysicalDeviceID);

    return hMutexID;
}

// pure virtual derived of ITransport
uint TransportUSB::Close(V100_DEVICE_TRANSPORT_INFO* pTransport)
{
    CloseHandle(pTransport->hWrite);
    CloseHandle(pTransport->hRead);
    CloseHandle(hMutexID);
    return 0;
}

// pure virtual derived of ITransport
bool TransportUSB::GetDeviceId(V100_DEVICE_TRANSPORT_INFO* pTransport, char* szDeviceId, uint& nLength)
{
    nLength = 0;
    strcpy(szDeviceId, hPhysicalDeviceID);
    nLength = (uint) strlen(szDeviceId) + 1;
    return true;
}

/*
** pure virtual derived of ITransport
** performs the four-step transmit/receive sequence for sending a command and getting the result.
** returns: positive byte count success
**          0 failure/timeout
*/
#ifndef MSEC_TIMEOUT
#define MSEC_TIMEOUT 15000
#endif
uint TransportUSB::TransmitCommand(V100_DEVICE_TRANSPORT_INFO* pTransport, const uchar* myPacket, uint nTxSize, uchar* pResponse, uint& nRxSize)
{
    HANDLE hEvent = GetMutexId();
    // Wait for it
    if(WAIT_TIMEOUT == WaitForSingleObject(hEvent, MutexTimeout))
    {
        fprintf(stdout, "\nMutex timed-out");
        return 0;
    }
    // Fill out simple header
    USBCB usbcb;                                // command block
    usbcb.ulCommand = m_mode;                    // command    0 = HOST_VCOM (plaintext) IENVELOPE::XPRT_VCOM_ENCAPSULATED (Transport Security)
    usbcb.ulCount    = nTxSize;                    // not used
    usbcb.ulData    = 0x0;                        // not used
    nRxSize = 0;

    // Send Challenge Header Packet
    if (!WriteBytesEx(pTransport->hWrite, &usbcb, sizeof(usbcb), MSEC_TIMEOUT))
    {
        ReleaseMutex(hEvent);
        if(SHOW_DEBUG_OUTPUT != 0)
        {
            fprintf(stdout, "\n (%s,%d) - Failed write (usbcb).", __FILE__, __LINE__);
            fflush(stdout);
        }
        return 0;
    }
    // Send Challenge Data Packet
    if (!WriteBytesEx(pTransport->hWrite, myPacket, usbcb.ulCount, MSEC_TIMEOUT))
    {
        ReleaseMutex(hEvent);
        if(SHOW_DEBUG_OUTPUT != 0)
        {
            fprintf(stdout, "\n (%s,%d) - Failed write (Packet).", __FILE__, __LINE__);
            fflush(stdout);
        }
        return 0;
    }
    // Read Response Header Packet
    if (!ReadBytesEx(pTransport->hRead, &usbcb, sizeof(usbcb), MSEC_TIMEOUT))
    {
        ReleaseMutex(hEvent);
        if(SHOW_DEBUG_OUTPUT != 0)
        {
            fprintf(stdout, "\n (%s,%d) - Failed read (usbcb).", __FILE__, __LINE__);
            fflush(stdout);
        }
        return 0;
    }
    // Read Response Data Packet
    if (!ReadBytesEx(pTransport->hRead, pResponse, usbcb.ulCount, MSEC_TIMEOUT))
    {
        ReleaseMutex(hEvent);
        if(SHOW_DEBUG_OUTPUT != 0)
        {
            fprintf(stdout, "\n (%s,%d) - Failed read (packet).", __FILE__, __LINE__);
            fflush(stdout);
        }
        return 0;
    }
    nRxSize = usbcb.ulCount;
    ReleaseMutex(hEvent);
    return 1;
}

/*
** Writes bytes, returns false on timeout or other error
*/
bool TransportUSB::WriteBytes(HANDLE hWrite, const void* pPacket, uint nPacketSize, uint mSecTimeout)
{
    // send X number of 64K packets
    int nIterations    = nPacketSize/MAX_DATA_BYTES_EZEXTENDER;
    int nRemainder    = nPacketSize%MAX_DATA_BYTES_EZEXTENDER;
    DWORD ulActualBytes = 0;

    // already max-packetized?
    if (nPacketSize == MAX_DATA_BYTES_EZEXTENDER)
    {
        nIterations    = 0;
        nRemainder    = MAX_DATA_BYTES_EZEXTENDER;
    }

    // loopy-write the data
    for (int ii = 0 ; ii < nIterations ; ii++)
    {
        char* pData = (char*)pPacket + ii*MAX_DATA_BYTES_EZEXTENDER;
        uint nElementCount = MAX_DATA_BYTES_EZEXTENDER;
        if (!WriteBytes(hWrite, pData, nElementCount, mSecTimeout) )
        {
            if(SHOW_DEBUG_OUTPUT != 0)
            {
                fprintf(stdout, "TransportUSB::WriteBytes: recursive call for full size failed iter: %d\n", ii);
                fflush(stdout);
            }
            return false;
        }

        // Always add a 10 ms delay unless we are the last itteration of writting the data and there is
        // no remainder to be written after all of the recursion completes
        if( !(ii == nIterations-1 && nRemainder == 0) )
        {
            Sleep(10);
        }
    }

    if(nRemainder > 0)
    {
        // write non-max-packetized remainder
        char* pData = (char*)pPacket + nIterations*MAX_DATA_BYTES_EZEXTENDER;
        uint nElementCount = nRemainder;
        //bool  bTimeOut = false;
        //DWORD Start = GetTickCount();
        if (!pUSB->WritePipe(hWrite, pData, nElementCount, &ulActualBytes))
        {
            if(SHOW_DEBUG_OUTPUT != 0)
            {
                fprintf(stdout, "TransportUSB::WriteBytes: WritePipe %d bytes failed\n", nElementCount);
                fflush(stdout);
            }
            return false;
        }
    }
    return true;
}

/*
** Reads bytes, returns false on timeout or other error
*/
bool TransportUSB::ReadBytes(HANDLE hRead, void* pPacket, uint nPacketSize, uint mSecTimeout)
{
    // read X number of 64K packets
    int nIterations    = nPacketSize/MAX_DATA_BYTES_EZEXTENDER;
    int nRemainder    = nPacketSize%MAX_DATA_BYTES_EZEXTENDER;
    unsigned char * pRxBuffer = (unsigned char *)pPacket;

    // already max-packetized?
    if (nPacketSize == MAX_DATA_BYTES_EZEXTENDER)
    {
        nIterations    = 0;
        nRemainder    = MAX_DATA_BYTES_EZEXTENDER;
    }

    // loopy-read the data
    for (int ii = 0 ; ii < nIterations ; ii++)
    {
        pRxBuffer = (unsigned char *)pPacket + ii*MAX_DATA_BYTES_EZEXTENDER;
        if(false == ReadBytes(hRead, pRxBuffer, MAX_DATA_BYTES_EZEXTENDER, mSecTimeout))
        {
            if(SHOW_DEBUG_OUTPUT != 0)
            {
                fprintf(stdout, "TransportUSB::ReadBytes: recursive call for full size failed iter: %d\n", ii);
                fflush(stdout);
            }
            return false;
        }
    }

    // read non-max-packetized remainder
    //bool bTimeOut = false;
    pRxBuffer = (unsigned char *)pPacket + nIterations*MAX_DATA_BYTES_EZEXTENDER;
    DWORD Start = GetTickCount();
    DWORD nRxSize = 0;
    while (1)
    {
        if (pUSB->ReadPipe(hRead, pRxBuffer, nRemainder, (LPDWORD)&nRxSize))
            break;

        if(SHOW_DEBUG_OUTPUT != 0)
        {
            fprintf(stdout, "TransportUSB::ReadBytes: ReadPipe %d bytes failed\n", nRemainder);
            fflush(stdout);
        }

        DWORD Now = GetTickCount();
        if ((Now - Start) >= mSecTimeout)
        {
            if(SHOW_DEBUG_OUTPUT != 0)
            {
                fprintf(stdout, "TransportUSB::ReadBytes: bytes %d Now: %u Start: %u TIMING OUT!\n", nRemainder, Now, Start);
                fflush(stdout);
            }
            return false;
        }
    }

    // completed?
    if (nRxSize != (DWORD)nRemainder)
    {
        if(SHOW_DEBUG_OUTPUT != 0)
        {
            fprintf(stdout, "TransportUSB::ReadBytes: incomplete read: want: %d got: %d\n", nRemainder, nRxSize);
            fflush(stdout);
        }
        return false;
    }

    return true;
}
