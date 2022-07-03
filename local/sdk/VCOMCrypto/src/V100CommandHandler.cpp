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

#include "stdio.h"
#include "V100CommandHandler.h"
#include "V100Cmd.h"
#include "V100EncCmd.h"
#include "ITransport.h"
#include "IMemMgr.h"
#include "Device.h"

#ifdef __GNUC__
    #include "windows_conv.h"
    #include "TransportLibUSB.h"
#else
    #include "windows.h"
    #include "TransportUSB.h"
#endif

#include <map>

#define MAXBUFFERSIZE    (16 * 1024 * 1024)

CommandHandlerArray cmdArr;

#ifndef __GNUC__
HANDLE V100CommandHandler::hSyncMutex = CreateMutex(NULL, FALSE, L"CMDHdlMutex");
#endif

V100CommandHandler* V100CommandHandler::GetCommandHandler(const V100_DEVICE_TRANSPORT_INFO* pID) {

    #ifndef __GNUC__
        WaitForSingleObject(hSyncMutex, INFINITE);
    #endif

    CommandHandlerArray::iterator it = cmdArr.find(pID->hInstance);

    if(it != cmdArr.end()) {
        #ifndef __GNUC__
            ReleaseMutex(hSyncMutex);
        #endif
        return (it->second);
    }
    V100CommandHandler* pCmdHandler = new V100CommandHandler();

    #ifndef __GNUC__
        ReleaseMutex(hSyncMutex);
    #endif

    return pCmdHandler;
}

V100CommandHandler::V100CommandHandler(void) {
}

V100CommandHandler::~V100CommandHandler(void) {

}

uint V100CommandHandler::Initialize(V100_DEVICE_TRANSPORT_INFO* pTransport) {

    ITransport* pTr = NULL;

    // USB Only for now
    pTr = new TransportUSB();

    uint rc = 0;
    rc = pTr->Initialize(pTransport);
    if (0 != rc) {
        if(pTr) {
            delete pTr;
            pTr = NULL;
            pTransport->hInstance = NULL;
        }
        return rc;
    }

    // Add it to command map.
    pTransport->hInstance = NULL; // clear out the device pointer
    pTransport->hInstance = dynamic_cast<ITransport*>(pTr);

    #ifndef __GNUC__
        WaitForSingleObject(hSyncMutex, INFINITE);
    #endif

    cmdArr.insert(CmdHdlPair(pTransport->hInstance, this));

    #ifndef __GNUC__
        ReleaseMutex(hSyncMutex);
    #endif
    return 0;
}

void  V100CommandHandler::Close(V100_DEVICE_TRANSPORT_INFO* pDev) {

    ITransport* pTransportLayer = reinterpret_cast<ITransport*>(pDev->hInstance);
    if(pTransportLayer == NULL) return;
    pTransportLayer->Close(pDev);
    delete pTransportLayer;
    // Remove yourself
    CommandHandlerArray::iterator it = cmdArr.find(pDev->hInstance);
    if(it != cmdArr.end()) {
        cmdArr.erase(it);
    }

    pDev->hInstance = NULL;
}

ICmd* V100CommandHandler::IssueCommand( V100_DEVICE_TRANSPORT_INFO* pDev, int route_flag, ICmd* pChallenge) {

    // If you can't do it, return
    if(pChallenge == NULL) return NULL;
    // Transmit it.
    ICmd* pResponse = TransmitCommand( (V100_DEVICE_TRANSPORT_INFO*) pDev, route_flag, pChallenge);
    // Delete the Challenge.
    delete pChallenge;
    pChallenge = NULL;
    // Return the response.
    return pResponse;
}

ICmd* V100CommandHandler::IssueCommand2 ( const V100_DEVICE_TRANSPORT_INFO * const pDev, int route_flag, ICmd* pChallenge) {

    ICmd* retVal;

    if (pChallenge == NULL) { 
        return NULL;
    }

    retVal = TransmitCommand((V100_DEVICE_TRANSPORT_INFO*)pDev, route_flag, pChallenge);

    return retVal;
}

ICmd* V100CommandHandler::GetResponse(V100_DEVICE_TRANSPORT_INFO* pDev, uchar* pResponseBytes, uint nSize) {

    _V100_COMMAND_SET cmd;
    memcpy(&cmd,&pResponseBytes[2],sizeof(cmd));
    ICmd* pCmd = CreateCommand(cmd);
    if(pCmd == NULL) return NULL;
    pCmd->UnpackResponse(pResponseBytes,nSize);
    return pCmd;
}

ICmd* V100CommandHandler::TransmitCommand(V100_DEVICE_TRANSPORT_INFO* pDev, int route_flag, ICmd* pCommand) {

    uchar*  myPacket = nullptr;
    ICmd*   pRcv     = nullptr;
    uint    nSize    = 0;
    uint    nRxBufferSize = MAXBUFFERSIZE;
    uchar*  pRxBuffer = nullptr;
    bool    ioRes;

    ITransport* pTransportLayer = reinterpret_cast<ITransport*>(pDev->hInstance);
    if(NULL == pTransportLayer) {
        return NULL;
    }

    ioRes = pCommand->PackChallenge(&myPacket, nSize);
    if (false == ioRes) {
        return NULL;
    }

    pRxBuffer = (uchar*)MALLOC(nRxBufferSize);

    if ( pRxBuffer != nullptr ) {

        memset(pRxBuffer, 0, nRxBufferSize);

        ioRes = pTransportLayer->TransmitCommand(pDev, route_flag, myPacket, nSize, pRxBuffer, nRxBufferSize);
        if(false == ioRes) {
            FREE(pRxBuffer);
            return NULL;
        }

        // Create Response Cmd
        pRcv = GetResponse(pDev, pRxBuffer, nRxBufferSize);

        // Free Response Buffer
        FREE(pRxBuffer);
    }

    return pRcv;
}