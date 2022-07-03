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
#ifndef WIN32
    #include "windows_conv.h"
#else
    #include <windows.h>
#endif

#include "VCOMBase.h"
#include "V100CommandHandler.h"
#include "V100Cmd.h"
#include "EncCmd.h"
#include "IMemMgr.h"
#include <iostream>
#include "VCOMCrypto.h"
#include "EncTypes.h"


#ifndef __GNUC__
#include "V100DeviceHandler.h"
EXTERN_C const GUID GUID_BULKLDI;
#endif

#define PERIOD 1


#define __LOCK_API__
#ifdef __LOCK_API__
#include <mutex>
static std::recursive_mutex io_rec_mutex;

//
// LOCK_FUNC makes certain that a client does not call various VCOM functions
// from multiple threads, regarding the same device.
// Multi-threaded applications with multiple device handles is allowed, however.

#define LOCK_FUNC()                                            \
    if(false == V100CommandHandler::Lock_Handle(pDev) ){    \
        return GEN_ERROR_APP_BUSY;                            \
    }


// BLOCK_LOCK
#define BLOCK_LOCK() std::lock_guard<std::recursive_mutex> lg(io_rec_mutex);
#else
#define LOCK_FUNC()
#define BLOCK_LOCK()
#endif



VCOM_CORE_EXPORT V100_ERROR_CODE V100_Enc_Capture(const V100_DEVICE_TRANSPORT_INFO* pDev, _V100_ENC_CAPTURE_TYPE type, StatusCallBack callbackfunc)
{
    LOCK_FUNC();
    Atomic_Enc_Capture* pCommand = reinterpret_cast<Atomic_Enc_Capture*>(V100CommandHandler::GetCommandHandler(pDev)->CreateCommand(_V100_COMMAND_SET(CMD_ENC_CAPTURE)));
    pCommand->SetCaptureType(type);
    ICmd* pResponse = (V100CommandHandler::GetCommandHandler(pDev)->IssueCommand((V100_DEVICE_TRANSPORT_INFO*)(pDev), pCommand));
    if (pResponse == NULL) return GEN_ERROR_COMM_TIMEOUT;
    Atomic_Error* pErr = dynamic_cast<Atomic_Error*>(pResponse);
    if (pErr)  // Handle Error
    {
        short arg; pErr->GetArguement(arg); delete pErr; return (V100_ERROR_CODE)(arg);
    }
    pCommand = dynamic_cast<Atomic_Enc_Capture*>(pResponse);
    if (pCommand == NULL) { delete pResponse; return GEN_ERROR_INTERNAL; }
    delete pCommand;

    // Poll for capture finish
    return PollCaptureCompletion(pDev, callbackfunc);
}


VCOM_CORE_EXPORT V100_ERROR_CODE V100_Enc_Decrypt(const V100_DEVICE_TRANSPORT_INFO* pDev, uchar* pDataIn, int pDataInSize, u256 DigSigIn, uchar* pDataOut, int& pDataOutSize, u256 DigSigOut)
{
    LOCK_FUNC();
    Atomic_Enc_Decrypt* pCommand = reinterpret_cast<Atomic_Enc_Decrypt*>(V100CommandHandler::GetCommandHandler(pDev)->CreateCommand(_V100_COMMAND_SET(CMD_ENC_DECRYPT)));
    pCommand->SetDataBuffer(pDataIn, pDataInSize);
    pCommand->SetDigSig(DigSigIn);
    ICmd* pResponse = (V100CommandHandler::GetCommandHandler(pDev)->IssueCommand((V100_DEVICE_TRANSPORT_INFO*)(pDev), pCommand));
    if (pResponse == NULL) return GEN_ERROR_COMM_TIMEOUT;
    Atomic_Error* pErr = dynamic_cast<Atomic_Error*>(pResponse);
    if (pErr)  // Handle Error
    {
        short arg; pErr->GetArguement(arg); delete pErr; return (V100_ERROR_CODE)(arg);
    }
    pCommand = dynamic_cast<Atomic_Enc_Decrypt*>(pResponse);
    if (pCommand == NULL) { delete pResponse; return GEN_ERROR_INTERNAL; }
    uchar* pPtr = pCommand->GetDataBuffer();
    pDataOutSize = pCommand->GetDataBufferSize();
    memcpy(pDataOut, pPtr, pDataOutSize);
    memcpy(DigSigOut, pCommand->GetDigSig(), sizeof(u256));
    delete pCommand;
    return GEN_OK;
}


VCOM_CORE_EXPORT V100_ERROR_CODE V100_Enc_Enroll(const V100_DEVICE_TRANSPORT_INFO* pDev)
{
    LOCK_FUNC();
    Atomic_Enc_Enroll* pCommand = reinterpret_cast<Atomic_Enc_Enroll*>(V100CommandHandler::GetCommandHandler(pDev)->CreateCommand(_V100_COMMAND_SET(CMD_ENC_ENROLL)));

    ICmd* pResponse = (V100CommandHandler::GetCommandHandler(pDev)->IssueCommand((V100_DEVICE_TRANSPORT_INFO*)(pDev), pCommand));
    if (pResponse == NULL) return GEN_ERROR_COMM_TIMEOUT;
    Atomic_Error* pErr = dynamic_cast<Atomic_Error*>(pResponse);
    if (pErr)  // Handle Error
    {
        short arg; pErr->GetArguement(arg); delete pErr; return (V100_ERROR_CODE)(arg);
    }
    pCommand = dynamic_cast<Atomic_Enc_Enroll*>(pResponse);
    if (pCommand == NULL) { delete pResponse; return GEN_ERROR_INTERNAL; }
    delete pCommand;
    return GEN_OK;
}


VCOM_CORE_EXPORT V100_ERROR_CODE V100_Enc_Factory_Set_Key(const V100_DEVICE_TRANSPORT_INFO* pDev, u256 key, u256 digsig)
{
    LOCK_FUNC();
    Atomic_Enc_Factory_Set_Key* pCommand = reinterpret_cast<Atomic_Enc_Factory_Set_Key*>(V100CommandHandler::GetCommandHandler(pDev)->CreateCommand(_V100_COMMAND_SET(CMD_ENC_FACTORY_SET_KEY)));

    pCommand->SetKey(key);
    pCommand->SetDigSig(digsig);

    ICmd* pResponse = (V100CommandHandler::GetCommandHandler(pDev)->IssueCommand((V100_DEVICE_TRANSPORT_INFO*)(pDev), pCommand));
    if (pResponse == NULL) return GEN_ERROR_COMM_TIMEOUT;
    Atomic_Error* pErr = dynamic_cast<Atomic_Error*>(pResponse);
    if (pErr)  // Handle Error
    {
        short arg; pErr->GetArguement(arg); delete pErr; return (V100_ERROR_CODE)(arg);
    }
    pCommand = dynamic_cast<Atomic_Enc_Factory_Set_Key*>(pResponse);
    if (pCommand == NULL) { delete pResponse; return GEN_ERROR_INTERNAL; }
    delete pCommand;
    return GEN_OK;
}


VCOM_CORE_EXPORT V100_ERROR_CODE V100_Enc_Get_Rnd_Number_SK(const V100_DEVICE_TRANSPORT_INFO* pDev, u256 rndNumber)
{
    LOCK_FUNC();
    Atomic_Enc_Get_Rnd_Number* pCommand = reinterpret_cast<Atomic_Enc_Get_Rnd_Number*>(V100CommandHandler::GetCommandHandler(pDev)->CreateCommand(_V100_COMMAND_SET(CMD_ENC_GET_RND_NUMBER)));
    pCommand->SetArguement(1);
    ICmd* pResponse = (V100CommandHandler::GetCommandHandler(pDev)->IssueCommand((V100_DEVICE_TRANSPORT_INFO*)(pDev), pCommand));
    if (pResponse == NULL) return GEN_ERROR_COMM_TIMEOUT;
    Atomic_Error* pErr = dynamic_cast<Atomic_Error*>(pResponse);
    if (pErr)  // Handle Error
    {
        short arg; pErr->GetArguement(arg); delete pErr; return (V100_ERROR_CODE)(arg);
    }
    pCommand = dynamic_cast<Atomic_Enc_Get_Rnd_Number*>(pResponse);
    if (pCommand == NULL) { delete pResponse; return GEN_ERROR_INTERNAL; }
    memcpy(rndNumber, pCommand->GetRandomNumber(), sizeof(u256));
    delete pCommand;
    return GEN_OK;

}


VCOM_CORE_EXPORT V100_ERROR_CODE V100_Enc_IdentifyMatch(const V100_DEVICE_TRANSPORT_INFO* pDev,
    long int& nFMRRequested,                // FMR In/Out [ CLEAR]
    u8* pastReferenceTemplate,                // Non-Contiguous arrays, each encrypted w/ session key, each with size header.
    int nPastReferenceTemplateSize,            // Clear
    u256 puszAleatoryNumberBB,                // [ANSOL]SK
    u256 DigSigIn,                            // [[DeviceID  + ANSOL + ANBIO + nFMRRequested + TemplateData]SHA]SK
    uchar** pCGResult,                        // [ MatchScore + MatchResult + ANSOL ] SK
    int& sizeResult,                        // Clear
    int * piResult,                            // Clear
    long int * pliFMRResult,                // Clear
    u256 DigSigOut)                            // [[pliFMRResult + pIResult+ puszAleatoryNumberBB + 8 byte padding]SHA256]SK
{
    LOCK_FUNC();
    Atomic_Enc_IdentifyMatch* pCommand = reinterpret_cast<Atomic_Enc_IdentifyMatch*>(V100CommandHandler::GetCommandHandler(pDev)->CreateCommand(_V100_COMMAND_SET(CMD_ENC_IDENTIFYMATCH)));
    pCommand->SetDataBuffer(pastReferenceTemplate, nPastReferenceTemplateSize);
    pCommand->SetDigSig(DigSigIn);
    pCommand->SetFMRRequested((int)nFMRRequested);
    pCommand->SetANSOLSK(puszAleatoryNumberBB);

    ICmd* pResponse = (V100CommandHandler::GetCommandHandler(pDev)->IssueCommand((V100_DEVICE_TRANSPORT_INFO*)(pDev), pCommand));
    if (pResponse == NULL) return GEN_ERROR_COMM_TIMEOUT;
    Atomic_Error* pErr = dynamic_cast<Atomic_Error*>(pResponse);
    if (pErr)  // Handle Error
    {
        short arg; pErr->GetArguement(arg); delete pErr; return (V100_ERROR_CODE)(arg);
    }
    pCommand = dynamic_cast<Atomic_Enc_IdentifyMatch*>(pResponse);
    if (pCommand == NULL) { delete pResponse; return GEN_ERROR_INTERNAL; }
    *pCGResult = (uchar*)MALLOC(pCommand->GetDataBufferSize());
    if (*pCGResult == NULL)
    {
        return GEN_ERROR_MEMORY;
    }
    memcpy(*pCGResult, pCommand->GetDataBuffer(), pCommand->GetDataBufferSize());
    memcpy(DigSigOut, pCommand->GetDigSig(), sizeof(u256));
    sizeResult = pCommand->GetDataBufferSize();
    *piResult = pCommand->GetResult();
    *pliFMRResult = pCommand->GetFMRRequested();
    delete pCommand;
    return GEN_OK;

}


VCOM_CORE_EXPORT V100_ERROR_CODE V100_Enc_ReturnCapturedBIR(const V100_DEVICE_TRANSPORT_INFO* pDev, ST_BBXBIO_BIR_HEADER* inHdr, uchar* pCryptogramANSOL, int sizeCGANSOL, uchar** pCGBIRRecord, int& sizeBIRRecord, u256 oDigSig)
{
    LOCK_FUNC();
    Atomic_Enc_ReturnCapturedBIR* pCommand = reinterpret_cast<Atomic_Enc_ReturnCapturedBIR*>(V100CommandHandler::GetCommandHandler(pDev)->CreateCommand(_V100_COMMAND_SET(CMD_ENC_RETURNCAPTUREDBIR)));
    pCommand->SetCG(pCryptogramANSOL, sizeCGANSOL);
    pCommand->SetBIR(inHdr);
    ICmd* pResponse = (V100CommandHandler::GetCommandHandler(pDev)->IssueCommand((V100_DEVICE_TRANSPORT_INFO*)(pDev), pCommand));
    if (pResponse == NULL) return GEN_ERROR_COMM_TIMEOUT;
    Atomic_Error* pErr = dynamic_cast<Atomic_Error*>(pResponse);
    if (pErr)  // Handle Error
    {
        short arg; pErr->GetArguement(arg); delete pErr; return (V100_ERROR_CODE)(arg);
    }
    pCommand = dynamic_cast<Atomic_Enc_ReturnCapturedBIR*>(pResponse);
    if (pCommand == NULL) { delete pResponse; return GEN_ERROR_INTERNAL; }
    int nSizeReturnedCG = pCommand->GetCGSize();
    *pCGBIRRecord = (u8*)MALLOC(nSizeReturnedCG);
    memcpy(*pCGBIRRecord, pCommand->GetCG(), nSizeReturnedCG);
    sizeBIRRecord = nSizeReturnedCG;
    memcpy(oDigSig, pCommand->GetDigSig(), sizeof(u256));
    delete pCommand;
    return GEN_OK;
}


VCOM_CORE_EXPORT V100_ERROR_CODE V100_Enc_ReturnCapturedBIR_IM(const V100_DEVICE_TRANSPORT_INFO* pDev, ST_BBXBIO_BIR_HEADER* inHdr, uchar* pCryptogramANSOL, int sizeCGANSOL, uchar** pCGBIRRecord, int& sizeBIRRecord, u256 oDigSig, _V100_IMAGE_TYPE imageType, uchar** ppImage, uint& nImageSize, u256 oImageDigSig)
{
    LOCK_FUNC();
    Atomic_Enc_ReturnCapturedBIR_IM* pCommand = reinterpret_cast<Atomic_Enc_ReturnCapturedBIR_IM*>(V100CommandHandler::GetCommandHandler(pDev)->CreateCommand(_V100_COMMAND_SET(CMD_ENC_RETURNCAPTUREDBIR_IM)));
    pCommand->SetCG(pCryptogramANSOL, sizeCGANSOL);
    pCommand->SetBIR(inHdr);
    pCommand->SetImageType(imageType);

    // This function deletes pCommand
    ICmd* pResponse = (V100CommandHandler::GetCommandHandler(pDev)->IssueCommand((V100_DEVICE_TRANSPORT_INFO*)(pDev), pCommand));
    pCommand = NULL;

    if (pResponse == NULL)
    {
        return GEN_ERROR_COMM_TIMEOUT;
    }

    Atomic_Error* pErr = dynamic_cast<Atomic_Error*>(pResponse);
    if (pErr)  // Handle Error
    {
        short arg;
        pErr->GetArguement(arg);
        delete pErr;
        return (V100_ERROR_CODE)(arg);
    }

    pCommand = dynamic_cast<Atomic_Enc_ReturnCapturedBIR_IM*>(pResponse);
    int nSizeReturnedCG = pCommand->GetCGSize();
    *pCGBIRRecord = (u8*)MALLOC(nSizeReturnedCG);
    memcpy(*pCGBIRRecord, pCommand->GetCG(), nSizeReturnedCG);
    sizeBIRRecord = nSizeReturnedCG;
    memcpy(oDigSig, pCommand->GetDigSig(), sizeof(u256));

    nImageSize = pCommand->GetImageSize();
    *ppImage = (u8*)MALLOC(nImageSize);
    memcpy(*ppImage, pCommand->GetImageCG(), nImageSize);
    memcpy(oImageDigSig, pCommand->GetImageDigSig(), sizeof(u256));

    delete pCommand;
    return GEN_OK;
}


VCOM_CORE_EXPORT V100_ERROR_CODE V100_Enc_Set_Parameters(const V100_DEVICE_TRANSPORT_INFO* pDev, uchar* pSKCG, int nSKSize, uchar* pANSOL, int nANSOLSize,
    _V100_ENC_SPOOF_PROTECTION_LEVEL protLevel, int nTimeoutSeconds)
{
    LOCK_FUNC();
    Atomic_Enc_Set_Parameters* pCommand = reinterpret_cast<Atomic_Enc_Set_Parameters*>(V100CommandHandler::GetCommandHandler(pDev)->CreateCommand(_V100_COMMAND_SET(CMD_ENC_SET_PARAMETERS)));
    pCommand->SetParameters(pSKCG, nSKSize, pANSOL, nANSOLSize, protLevel, nTimeoutSeconds);
    ICmd* pResponse = (V100CommandHandler::GetCommandHandler(pDev)->IssueCommand((V100_DEVICE_TRANSPORT_INFO*)(pDev), pCommand));
    if (pResponse == NULL) return GEN_ERROR_COMM_TIMEOUT;
    Atomic_Error* pErr = dynamic_cast<Atomic_Error*>(pResponse);
    if (pErr)  // Handle Error
    {
        short arg; pErr->GetArguement(arg); delete pErr; return (V100_ERROR_CODE)(arg);
    }
    pCommand = dynamic_cast<Atomic_Enc_Set_Parameters*>(pResponse);
    if (pCommand == NULL) { delete pResponse; return GEN_ERROR_INTERNAL; }

    delete pCommand;
    return GEN_OK;
}


VCOM_CORE_EXPORT V100_ERROR_CODE V100_Enc_VerifyMatch(const V100_DEVICE_TRANSPORT_INFO* pDev,
    long int& nFMRRequested,        // FMR In/Out [ CLEAR]
    int* piResult,                // 0 No Match, >0 Match [CLEAR]
    uchar* pCGTemplateToCompare,  // [BIR]SK
    const int szCG,                // Size pCGTemplateToCompare cryptogram
    u256 puszAleatoryNumberBB,    // [ANSOL]SK
    u256 DigSigIn,                // [[device ID + ANBIO + puszAleatoryNumberBB (ANSOL) + liMaxFMRRequested + pstReferenceTemplate]SHA]SK
    uchar** pCGResult,            // [ MatchScore + MatchResult + ANSOL ] SK
    int& sizeResult)
{
    LOCK_FUNC();
    Atomic_Enc_VerifyMatch* pCommand = reinterpret_cast<Atomic_Enc_VerifyMatch*>(V100CommandHandler::GetCommandHandler(pDev)->CreateCommand(_V100_COMMAND_SET(CMD_ENC_VERIFYMATCH)));
    pCommand->SetDataBuffer(pCGTemplateToCompare, szCG);
    pCommand->SetDigSig(DigSigIn);
    pCommand->SetFMRRequested((int)nFMRRequested);
    pCommand->SetANSOLSK(puszAleatoryNumberBB);

    ICmd* pResponse = (V100CommandHandler::GetCommandHandler(pDev)->IssueCommand((V100_DEVICE_TRANSPORT_INFO*)(pDev), pCommand));
    if (pResponse == NULL) return GEN_ERROR_COMM_TIMEOUT;
    Atomic_Error* pErr = dynamic_cast<Atomic_Error*>(pResponse);
    if (pErr)  // Handle Error
    {
        short arg; pErr->GetArguement(arg); delete pErr; return (V100_ERROR_CODE)(arg);
    }
    pCommand = dynamic_cast<Atomic_Enc_VerifyMatch*>(pResponse);
    if (pCommand == NULL) { delete pResponse; return GEN_ERROR_INTERNAL; }
    *pCGResult = (uchar*)MALLOC(pCommand->GetDataBufferSize());
    if (*pCGResult == NULL)
    {
        return GEN_ERROR_MEMORY;
    }
    memcpy(*pCGResult, pCommand->GetDataBuffer(), pCommand->GetDataBufferSize());
    sizeResult = pCommand->GetDataBufferSize();
    *piResult = pCommand->GetResult();
    nFMRRequested = pCommand->GetFMRRequested();

    delete pCommand;
    return GEN_OK;

}


