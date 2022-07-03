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

#include "ICmd.h"
#include "stdio.h"
#include "string.h"

// Definitions

uint    ICmd::MAX_TEMPLATE_SIZE    = 5000;
uint    ICmd::MAX_IMAGE_SIZE    = 640*480;


ICmd::ICmd()
{
    m_nArg = 0 ;
    m_nsohv = 0;
    m_nOpaqueDataSize = 0;
    m_pChallengeBuffer = NULL;
    m_pResponseBuffer = NULL;
    m_nRC = GEN_OK;
}
ICmd::~ICmd()
{
    if(m_pChallengeBuffer)
    {
        FREE(m_pChallengeBuffer);
        m_pChallengeBuffer = NULL;
       }
    if(m_pResponseBuffer)
    {
        FREE(m_pResponseBuffer);
        m_pResponseBuffer = NULL;
       }
}
uchar* ICmd::GenerateChallengeHeader(uint Arg, uint Size)
{
    if(m_pChallengeBuffer)
    {
        FREE(m_pChallengeBuffer);
        m_pChallengeBuffer = NULL;
    }
    m_pChallengeBuffer = (uchar*)MALLOC(m_nChallengeBufferSize);
    if(m_pChallengeBuffer == NULL) return NULL;

    memset(m_pChallengeBuffer, 0, m_nChallengeBufferSize);

    short sohv = SOHV;
    // SOHV
    memcpy(m_pChallengeBuffer,&sohv,sizeof(short));
    // _V100_COMMAND_SET
    memcpy(&m_pChallengeBuffer[2], &m_nCmd, sizeof(m_nCmd));
    // Argument
    memcpy(&m_pChallengeBuffer[6], &m_nArg, sizeof(unsigned int));
    // Size
    memcpy(&m_pChallengeBuffer[8], &Size, sizeof(unsigned int));
    // Return start of Opaque Data
    return &m_pChallengeBuffer[12];
}
uchar* ICmd::UnpackChallengeHeader(const uchar* pPacket, short& sohv,short& Arg,uint& OpaqueDataSize)
{
    // SOHV
    memcpy(&sohv,pPacket, sizeof(short));
    // _V100_COMMAND_SET
    memcpy(&m_nCmd, (void*)&pPacket[2], sizeof(m_nCmd));
    // Argument
    memcpy(&m_nArg, &pPacket[6], sizeof(short));
    // Size
    memcpy(&OpaqueDataSize, &pPacket[8], sizeof(uint));
    // Return start of Opaque Data
    return (uchar*)&pPacket[12];
}

uchar* ICmd::GenerateResponseHeader(uint Arg, uint Size)
{
    if(m_pResponseBuffer)
    {
        FREE(m_pResponseBuffer);
        m_pResponseBuffer = NULL;
    }
    m_pResponseBuffer = (uchar*)MALLOC(m_nResponseBufferSize);
    memset(m_pResponseBuffer, 0, m_nResponseBufferSize);
    if(m_pResponseBuffer == NULL) return NULL;
    // I don't believe this function can fail, so no RC
    uint _Arg = m_nArg;
       short sohv = SOHV;
    // SOHV
    memcpy(m_pResponseBuffer,&sohv,sizeof(short));
    // _V100_COMMAND_SET
    memcpy(&m_pResponseBuffer[2], &m_nCmd, sizeof(m_nCmd));
    // Argument
    memcpy(&m_pResponseBuffer[6], &_Arg, sizeof(unsigned int));
    // Size
    memcpy(&m_pResponseBuffer[8], &Size, sizeof(unsigned int));
    // Return start of Opaque Data
    return &m_pResponseBuffer[12];
}
uchar* ICmd::UnpackResponseHeader(const uchar* pPacket, short& sohv,short& Arg,uint& OpaqueDataSize)
{
    // SOHV
    memcpy(&sohv,pPacket, sizeof(short));
    // _V100_COMMAND_SET
    memcpy(&m_nCmd, (void*)&pPacket[2], sizeof(m_nCmd));
    // Arguement
    memcpy(&Arg, &pPacket[6], sizeof(short));
    // Size
    memcpy(&OpaqueDataSize, &pPacket[8], sizeof(uint));
    // Return start of Opaque Data
    return (uchar*)&pPacket[12];
}
void   ICmd::GenerateFooter(uchar* pFooterPtr)
{

}
bool   ICmd::CheckErrorCode(uchar** pPacket, uint& nSize)
{
    if(m_nRC != GEN_OK)
    {
        m_nResponseBufferSize = ENVELOPE_INFO_SIZE;
        m_nArg = m_nRC;
        m_pResponseBuffer = (uchar*)MALLOC(m_nResponseBufferSize);
        GenerateResponseHeader(0,m_nResponseBufferSize-ENVELOPE_INFO_SIZE);
        *pPacket = m_pResponseBuffer;
        nSize =       m_nResponseBufferSize;
        return false;
    }
    return true;
}
uchar* ICmd::Pack(uchar* pDst, void* pSrc, uint nSize)
{
     memcpy(pDst,(uchar*)pSrc,nSize);
     return pDst+nSize;
}
uchar* ICmd::Unpack(void* pDst, uchar* pSrc, uint nSize)
{
    memcpy(pDst,(uchar*)pSrc,nSize);
     return pSrc+nSize;
}
