/*************************************************************************************************************************
**                                                                                                                      **
** ©Copyright 2017 HID Global Corporation/ASSA ABLOY AB. ALL RIGHTS RESERVED.                                           **
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
#pragma once

#include "V100_shared_types.h"
#include "IMemMgr.h"

typedef unsigned char uchar;
typedef unsigned int  uint;

#define ENVELOPE_INFO_SIZE    14

class ICmd : public MemoryBase
{
public:
    ICmd();
    virtual ~ICmd();
    // Takes content of Command, and packs it into pPacket
    virtual bool PackChallenge(uchar** pPacket, uint& nSize) = 0;
     // Unpacks packet passed in into internal data structure
    virtual bool UnpackChallenge(const uchar* pPacket, uint nSize) = 0;
    // Takes content of Command, and packs it into pPacket
    virtual bool PackResponse(uchar** pPacket, uint& nSize) = 0;
    // Unpacks packet passed in into internal data structure
    virtual bool UnpackResponse(const uchar* pPacket, uint nSize) = 0;
    // How large is the Challenge packet?
    virtual int  GetChallengeBufferSize() = 0;
    // How large is the Response packet?
    virtual int  GetResponseBufferSize() = 0;
    // Set Return Code.
    virtual void SetReturnCode(_V100_GENERAL_ERROR_CODES rc) { m_nRC = rc;}
    // Get Return Code
    virtual _V100_GENERAL_ERROR_CODES GetReturnCode() { return m_nRC;}
    // Set Argument
    virtual void SetArguement(short arg) { m_nArg = arg;}
    // Get Argument
    virtual void GetArguement(short& arg) { arg = m_nArg;}
    // Execute command
    virtual void  Exec() { };
    // Return command code
    virtual _V100_COMMAND_SET GetCommandCode() { return m_nCmd;}
protected:
    // Generates the Header for the packet, return Opaque data pointer
    virtual uchar* GenerateChallengeHeader(uint Arg, uint Size);
    // Unpack the Challenge header, return Opaque data pointer
    virtual uchar* UnpackChallengeHeader(const uchar* pPacket, short& sohv,short& Arg,uint& OpaqueDataSize);
    // Generates the Header for the packet.
    virtual uchar* GenerateResponseHeader(uint Arg, uint Size);
    // Unpack the Response header, return Opaque data pointer
    virtual uchar* UnpackResponseHeader(const uchar* pPacket, short& sohv,short& Arg,uint& OpaqueDataSize);
    // Generates the Footer for the packet
    virtual void   GenerateFooter(uchar* pFooterPtr);
    // Check Error Code
    virtual bool   CheckErrorCode(uchar** pPacket, uint& nSize);
    // Pack data, return pointer
    virtual uchar* Pack(uchar* pDst, void* pSrc, uint nSize);
    virtual uchar* Unpack(void* pDst, uchar* pSrc, uint nSize);
protected:
    _V100_COMMAND_SET m_nCmd;
    uchar*              m_pChallengeBuffer;
    uint              m_nChallengeBufferSize;
    uchar*              m_pResponseBuffer;
    uint              m_nResponseBufferSize;
    // Protocol Specific Stuff
    // Check integrity of packet
    short              m_nArg;
    short              m_nsohv;
    uint              m_nOpaqueDataSize;
    // Definitions
    static    uint        MAX_TEMPLATE_SIZE;
    // ;;;;;
    static  uint        MAX_IMAGE_SIZE;
    // Return Code
    _V100_GENERAL_ERROR_CODES m_nRC;
};


