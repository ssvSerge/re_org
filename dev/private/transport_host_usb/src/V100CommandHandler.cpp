/***************************************************************************************/
// ©Copyright 2020 HID Global Corporation/ASSA ABLOY AB. ALL RIGHTS RESERVED.
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

#include "stdio.h"
#include "V100CommandHandler.h"
#include "V100Cmd.h"
#include "V100EncCmd.h"
#include "ITransport.h"
#include "IMemMgr.h"
#include "Device.h"
#include <mutex>
#include <memory>

#ifdef __GNUC__
    #include "windows_conv.h"
    #include "TransportLibUSB.h"
#else
    #include "windows.h"
    #include "TransportUSB.h"
#endif

#include <map>

typedef std::pair<HANDLE, V100CommandHandler*> CmdHdlPair;
typedef std::map<HANDLE, V100CommandHandler*> CommandHandlerArray;
typedef std::map<HANDLE, std::shared_ptr<std::recursive_mutex>> CommandHandlerLock;

CommandHandlerArray cmdArr;
CommandHandlerLock  cmdLock;

static std::mutex io_mutex;
#define LOCK_FUNC()  std::lock_guard<std::mutex> guard_lock(io_mutex);


static void _log_msg (const char* const Prefix, const uchar* const myPacket, uint nSize) {

    printf("%s: [ ", Prefix);

    for (uint i = 0; i < 12; i++) {
        printf("%.2x", myPacket[i]);
        printf(" ");
    }

    printf("] [ ");

    for (uint i = 12; i < nSize; i++) {
        printf("%.2x", myPacket[i]);
        printf(" ");
    }
    printf("] \r\n");
}

static void _log_tx ( const uchar* const myPacket, uint nSize ) {
    _log_msg( "TX", myPacket, nSize );
}

static void _log_rx (const uchar* const myPacket, uint nSize ) {
    _log_msg( "RX", myPacket, nSize );
}





V100CommandHandler* V100CommandHandler::GetCommandHandler(const V100_DEVICE_TRANSPORT_INFO* pID)
{
    LOCK_FUNC();

    // Search for ID, if it doesnt exist, create a new one, and return it.
    CommandHandlerArray::iterator it = cmdArr.find(pID->hInstance);
    if (it != cmdArr.end())
    {
        return (it->second);
    }
    // If we can't find it, create a new one, and throw it into the DEVICE_TRANSPORT_INFO
    // During call to initialize, we will add it to the command map.  If initialize is never
    // called, we have a memory leak.
    V100CommandHandler* pCmdHandler = new V100CommandHandler();


    return pCmdHandler;
}


bool V100CommandHandler::Lock_Handle(const V100_DEVICE_TRANSPORT_INFO* pDev)
{
    LOCK_FUNC();

    CommandHandlerLock::iterator it = cmdLock.find(pDev->hInstance);
    if (it == cmdLock.end())
    {
        return false;
    }

    std::unique_lock<std::recursive_mutex> func_lock(*it->second, std::defer_lock);
    try
    {
        if (!func_lock.try_lock())
        {
            return false;
        }
    }
    catch (const std::system_error&)
    {
        return false;
    }
    return true;
}


uint32_t V100CommandHandler::GetNumOpenHandles()
{
    LOCK_FUNC();
    uint32_t nHandles = (uint32_t)cmdArr.size();

    return nHandles;
}

V100CommandHandler::V100CommandHandler(void)
{

}

V100CommandHandler::~V100CommandHandler(void)
{

}
/* Initialize only gets called once. Per Instance. Add myself to the Cmd Queue */

uint  V100CommandHandler::Initialize(V100_DEVICE_TRANSPORT_INFO* pTransport)
{
    ITransport* pTr = NULL;

    // USB Only for now
    pTr = new TransportUSB();

    uint rc = 0;
    rc = pTr->Initialize(pTransport);
    if(0 != rc)
    {
        if(pTr)
        {
            delete pTr;
            pTr = NULL;
            pTransport->hInstance = NULL;
        }

        // TransportSE will return the app busy error if the pipe is already connected. Other
        // transport code will return a range of non-zero values for errors, all these will continue
        // to be marshalled into GEN_ERROR_START, except for USB_CONNECTION_REFUSED on Windows only.
        if (rc == GEN_ERROR_APP_BUSY)
        {
            return GEN_ERROR_APP_BUSY;
        }
#ifndef __GNUC__
        if (rc == USB_CONNECTION_REFUSED)
        {
            return GEN_ERROR_CONNECTION_REFUSED;
        }
#endif
        return GEN_ERROR_START;
    }

    // clear out the device pointer
    pTransport->hInstance = NULL;
    // Add it to command map.
    pTransport->hInstance = dynamic_cast<ITransport*>(pTr);

    LOCK_FUNC();

    cmdArr.insert(CmdHdlPair(pTransport->hInstance, this));
    // - AAL - add new mutex for locking this handle, specifically.
    auto pMutex = std::make_shared<std::recursive_mutex>();
    cmdLock.insert({ pTransport->hInstance, pMutex });

    return 0;
}
void  V100CommandHandler::Close(V100_DEVICE_TRANSPORT_INFO* pDev)
{
    ITransport* pTransportLayer = reinterpret_cast<ITransport*>(pDev->hInstance);
    if(pTransportLayer == NULL) return;
    // Remove yourself
    CommandHandlerArray::iterator it = cmdArr.find(pDev->hInstance);
    if(it != cmdArr.end())
    {
        pTransportLayer->Close(pDev);
        delete pTransportLayer;
        cmdArr.erase(it);
    }

    pDev->hInstance = NULL;
}
ICmd* V100CommandHandler::IssueCommand(V100_DEVICE_TRANSPORT_INFO* pDev, ICmd* pChallenge)
{
    // If you can't do it, return
    if(pChallenge == NULL)
    {
        return NULL;
    }
    // Transmit it.
    ICmd* pResponse = TransmitCommand(pDev, pChallenge);
    // Delete the Challenge.
    delete pChallenge;
    pChallenge = NULL;
    // Return the response.
    return pResponse;
}
ICmd* V100CommandHandler::GetResponse(V100_DEVICE_TRANSPORT_INFO* pDev, uchar* pResponseBytes, uint nSize)
{
    _V100_COMMAND_SET cmd;
    memcpy(&cmd,pResponseBytes+2,sizeof(cmd));
    ICmd* pCmd = CreateCommand(cmd);
    if(pCmd == NULL)
    {
        return NULL;
    }
    if(false == pCmd->UnpackResponse(pResponseBytes,nSize))
    {
        return NULL;
    }
    return pCmd;
}

#define MAXBUFFERSIZE    1024*1024*2
ICmd* V100CommandHandler::TransmitCommand(V100_DEVICE_TRANSPORT_INFO* pDev, ICmd* pCommand)
{
    uchar* myPacket = NULL;
    uint  nSize = 0;
    uint  nRxBufferSize = MAXBUFFERSIZE;
    ICmd*  pRcv = NULL;
    ITransport* pTransportLayer = reinterpret_cast<ITransport*>(pDev->hInstance);
    // Pack Challenge
    if(false == pCommand->PackChallenge(&myPacket, nSize))
    {
        return NULL;
    }
    // Allocate space for Response
    uchar* pRxBuffer = (uchar*)MALLOC(nRxBufferSize);
    memset(pRxBuffer, 0, nRxBufferSize);
    // Transmit Challenge, Get Response
    if(false == pTransportLayer->TransmitCommand(pDev, myPacket, nSize, pRxBuffer, nRxBufferSize))
    {
        FREE(pRxBuffer);
        return NULL;
    }
    // Create Response Cmd
    pRcv = GetResponse(pDev, pRxBuffer, nRxBufferSize);

    _log_tx(myPacket, nSize);
    _log_rx(pRxBuffer, nRxBufferSize);

    // Free Response Buffer
    FREE(pRxBuffer);
    return pRcv;
}

bool  V100CommandHandler::GetDeviceId(V100_DEVICE_TRANSPORT_INFO* pDev, char* pStr, uint& nLength)
{
    ITransport* pTransportLayer = reinterpret_cast<ITransport*>(pDev->hInstance);
    if(false == pTransportLayer->GetDeviceId(pDev, pStr, nLength))
    {
        return false;
    }
    return true;
}
