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

#include "IEncCmd.h"
#include "stdio.h"
#include "string.h"

IEncCmd::IEncCmd()
{
     // Challenge buffer size...
     m_nChallengeBufferSize = ENVELOPE_INFO_SIZE;
     m_pChallengeBuffer = NULL;
     m_nResponseBufferSize = ENVELOPE_INFO_SIZE;
     m_pResponseBuffer = NULL;

     m_pCGData = NULL;
     m_nCGDataSize = 0;
}
IEncCmd::~IEncCmd()
{
    if(m_pCGData)
    {
        FREE(m_pCGData);
        m_pCGData = NULL;
    }
}
// Called by Both sides
bool IEncCmd::SetCryptogram(const uchar* pCGPacket, uint nSize)
{
    if(m_pCGData)
    {
        FREE(m_pCGData);
        m_pCGData = NULL;
    }
    m_pCGData = (uchar*)MALLOC(nSize);
    if(m_pCGData == NULL)
    {
        return false;
    }
    memcpy(m_pCGData, pCGPacket, nSize);
    m_nCGDataSize = nSize;
    return true;
}
// Called by Both sides
bool IEncCmd::GetCryptogram(uchar** pCGPacket, uint& nSize)
{
    *pCGPacket = m_pCGData;
    nSize = m_nCGDataSize;
    return true;
}
// Takes content of Command, and packs it into pPacket
bool IEncCmd::PackChallenge(uchar** pPacket, uint& nSize)
{
    m_nChallengeBufferSize+=m_nCGDataSize;
    uchar* pPtr = GenerateChallengeHeader(m_nArg,m_nChallengeBufferSize-ENVELOPE_INFO_SIZE);
    if(NULL == pPtr) return false;
    if(m_pCGData != NULL )
    {
        pPtr = Pack(pPtr, m_pCGData, m_nCGDataSize);
    }
    // Fill in some stuff
    *pPacket = m_pChallengeBuffer;
    nSize = m_nChallengeBufferSize;
    return true;
}
// Unpacks packet passed in into internal data structure
bool IEncCmd::UnpackChallenge(const uchar* pPacket, uint nSize)
{
    uchar* pPtr = UnpackChallengeHeader(pPacket, m_nsohv, m_nArg, m_nOpaqueDataSize);
    m_pCGData = (uchar*)MALLOC(m_nOpaqueDataSize);
    pPtr = Unpack(m_pCGData, pPtr, m_nOpaqueDataSize);
    m_nCGDataSize = m_nOpaqueDataSize;

    return (pPtr!=NULL)? true:false;
}
// Takes content of Command, and packs it into pPacket
bool IEncCmd::PackResponse(uchar** pPacket, uint& nSize)
{
    // Check for error.
    if(CheckErrorCode(pPacket,nSize) == false) return true;
    // Fill in
    m_nResponseBufferSize+=m_nCGDataSize;
    uchar*pPtr = GenerateResponseHeader(0,m_nResponseBufferSize-ENVELOPE_INFO_SIZE);
    if(NULL == pPtr) return false;
    // Response
    pPtr = Pack(pPtr, m_pCGData, m_nCGDataSize);
    *pPacket = m_pResponseBuffer;
    nSize = m_nResponseBufferSize;
    return true;
}
// Unpacks packet passed in into internal data structure
bool IEncCmd::UnpackResponse(const uchar* pPacket, uint nSize)
{
    // Remember to allocate memory for composite image, and template, if requested.
    uchar* pPtr = UnpackResponseHeader(pPacket, m_nsohv, m_nArg, m_nOpaqueDataSize);
    if(NULL == pPtr) return false;
    if(m_nOpaqueDataSize == 0 ) return true;
    m_pCGData = (uchar*)MALLOC(m_nOpaqueDataSize);
    memset(m_pCGData, 0, sizeof(m_nOpaqueDataSize));
    m_nCGDataSize = m_nOpaqueDataSize;
    pPtr = Unpack(m_pCGData, pPtr, m_nCGDataSize);
    if(NULL == pPtr) return false;
    return true;
}
// How large is the Challenge packet?
int  IEncCmd::GetChallengeBufferSize()
{
    return m_nChallengeBufferSize;
}
// How large is the Response packet?
int  IEncCmd::GetResponseBufferSize()
{
    return m_nResponseBufferSize;
}