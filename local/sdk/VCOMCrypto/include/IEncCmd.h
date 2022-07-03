
/***************************************************************************************/
// ©Copyright 2015 HID Global Corporation/ASSA ABLOY AB. ALL RIGHTS RESERVED.
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

#pragma once

#include "V100_shared_types.h"
#include "ICmd.h"
#include "IMemMgr.h"

typedef unsigned char uchar;
typedef unsigned int  uint;

#define ENVELOPE_INFO_SIZE    14

class IEncCmd : public ICmd
{
public:
    IEncCmd();
    virtual ~IEncCmd();
    // Called by Both sides
    bool SetCryptogram(const uchar* pCGPacket, uint nSize);
     // Called by Both sides
    bool GetCryptogram(uchar** pCGPacket, uint& nSize);
    //
    uchar* GetCryptogram() { return m_pCGData; }
    // overload
    uint GetCryptogramSize() { return m_nCGDataSize; }
protected:
    // ICMD
     // Takes content of Command, and packs it into pPacket
    virtual bool PackChallenge(uchar** pPacket, uint& nSize);
    // Unpacks packet passed in into interal data structure
    virtual bool UnpackChallenge(const uchar* pPacket, uint nSize);
    // Takes content of Command, and packs it into pPacket
    virtual bool PackResponse(uchar** pPacket, uint& nSize);
    // Unpacks packet passed in into interal data structure
    virtual bool UnpackResponse(const uchar* pPacket, uint nSize);
    // How large is the Challenge packet?
    virtual int  GetChallengeBufferSize();
    // How large is the Response packet?
    virtual int  GetResponseBufferSize();
protected:
    uchar*              m_pCGData;
    uint              m_nCGDataSize;
};


