/*************************************************************************************************************************
**                                                                                                                      **
** Copyright 2019 HID Global Corporation/ASSA ABLOY AB. ALL RIGHTS RESERVED.                                            **
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
#include <windows.h>
#include <setupapi.h>
#include <guiddef.h>

#include <usbdriver.h>

#include <TransportUSB.h>
#include <Device.h>
#include <ldiguid.h>
#include <usbcmdset.h>

#include <application/types_c.h>
#include <application/StreamPrefix.h>

#ifdef MSEC_TIMEOUT
#undef MSEC_TIMEOUT
#endif 

#define MSEC_TIMEOUT        (15 * 1000)


TransportUSB::TransportUSB(void) {
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

uint TransportUSB::Initialize(V100_DEVICE_TRANSPORT_INFO* pTransport) {
    int nNumDev = 0;
    
    if (GEN_OK != V100_Get_Num_USB_Devices(&nNumDev))
        return USB_INVALID_DEVICE;

    if (nNumDev == 0)
        return USB_NO_DEVICES_FOUND;

    Device* device = reinterpret_cast<Device*>( pTransport->hInstance );

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
    if( pTransport->hWrite == INVALID_HANDLE_VALUE)
        return USB_WRITE_PIPE_ERROR;

    // open read handle
    pTransport->hRead  = pUSB->OpenDeviceHandle((LPGUID)&GUID_BULKLDI, 
                                                READ_PIPE, 
                                                device->m_nMemberIndex,//pTransport->DeviceNumber, 
                                                TRUE,
                                                hPhysicalDeviceID);
    
    if (pTransport->hRead == INVALID_HANDLE_VALUE)
        return USB_READ_PIPE_ERROR;

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

uint TransportUSB::Close(V100_DEVICE_TRANSPORT_INFO* pTransport)
{
    CloseHandle(pTransport->hWrite);
    CloseHandle(pTransport->hRead);
    CloseHandle(hMutexID);
    return 0;
}

bool TransportUSB::GetDeviceId(V100_DEVICE_TRANSPORT_INFO* pTransport, char* szDeviceId, uint& nLength)
{
    nLength = 0;
    strcpy(szDeviceId, hPhysicalDeviceID);
    nLength = (uint) strlen(szDeviceId) + 1;
    return true;
}

uint TransportUSB::TransmitCommand(V100_DEVICE_TRANSPORT_INFO* pTransport, int route_flag, const uchar* myPacket, uint nTxSize, uchar* pResponse, uint& nRxSize) {

    HANDLE hEvent = GetMutexId();
    // Wait for it
    if ( WAIT_TIMEOUT == WaitForSingleObject(hEvent, MutexTimeout) ) {
        fprintf(stdout, "\nMutex timed-out");
        return 0;
    }

    nRxSize = 0;

    hid::stream::params_t params;
    hid::types::storage_t out_hdr;
    hid::types::storage_t inp_hdr;

    params.command = hid::stream::cmd_t::STREAM_CMD_REQUEST;
    params.code    = 0;
    params.len     = nTxSize;
    hid::stream::Prefix::SetParams (params, out_hdr);

    // Send Challenge Header Packet
    if ( ! WriteBytes(pTransport->hWrite, out_hdr.data(), out_hdr.size(), MSEC_TIMEOUT ) ) {
        ReleaseMutex(hEvent);
        return 0;
    }

    // Send Challenge Data Packet
    if ( ! WriteBytes(pTransport->hWrite, myPacket, nTxSize, MSEC_TIMEOUT) ) {
        ReleaseMutex(hEvent);
        return 0;
    }

    // Read Response Header Packet
    inp_hdr.resize ( hid::stream::Prefix::PrefixSize() );
    if ( ! ReadBytes(pTransport->hRead, inp_hdr.data(), inp_hdr.size(), MSEC_TIMEOUT)) {
        ReleaseMutex(hEvent);
        return 0;
    }

    hid::stream::Prefix::GetParams( inp_hdr, params );

    // Read Response Data Packet
    if ( ! ReadBytes(pTransport->hRead, pResponse, params.len, MSEC_TIMEOUT) ) {
        ReleaseMutex(hEvent);
        return 0;
    }

    nRxSize = params.len;

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
    int nIterations  = nPacketSize/MAX_DATA_BYTES_EZEXTENDER;
    int nRemainder   = nPacketSize%MAX_DATA_BYTES_EZEXTENDER;

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

        DWORD Now = GetTickCount();
        if ((Now - Start) >= mSecTimeout)
        {
            return false;
        }
    }

    // completed?
    if (nRxSize != (DWORD)nRemainder)
    {
        return false;
    }

    return true;
}
