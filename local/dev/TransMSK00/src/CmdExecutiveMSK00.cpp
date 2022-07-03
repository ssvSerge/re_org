#include "CmdExecutiveMSK00.h"
#include "CmdExecutiveCommon.h"
#include "CmdExecutiveBase.h"
#include "IEnrollSvc.h"
#include "EnrollSvcMSK00.h"
#include "AppStateObserver.h"
#include "DataMgr.h"
#include "wsqWrapper.h"
#include "TAdapter.h"
#include "BioParameters.h"
#include "CryptoMgrMSK00.h"
#include "CfgMgr.h"
#include "AutoHeapBuffer.h"
#include "CriticalErrorLog.h"

CExecStatus        CmdExecutiveMSK00::Execute_Enc_Get_Rnd_Number(u256* pRN, short isEncryptedWithSk)
{
    CryptoMgrMSK00* pCryptMgr = CryptoMgrMSK00::GetInstance();
    uchar* pCG = nullptr;
    uint nCG = 0;
    CExecStatus bRet = CExecStatus::CMD_ERR_CRYPTO_FAIL;

    memset(pRN, 0, sizeof(u256));
    if (false == pCryptMgr->GetRandomNumber((u8*)pRN, ANBIO_SIZE))
    {
        LOGMSG("Execute_Enc_Get_Rnd_Number: Getting random number returned error. Returning error %d.", CExecStatus::CMD_ERR_CRYPTO_FAIL);
        return bRet;
    }

    switch (isEncryptedWithSk)
    {
    case 1:
    {
        if (false == pCryptMgr->Encrypt(KT_MSK_SK, (u8*)pRN, sizeof(u256), &pCG, nCG))
        {
            LOGMSG("Execute_Enc_Get_Rnd_Number: Encrypting random number with SK returned error. Returning error %d.",
                CExecStatus::CMD_ERR_CRYPTO_FAIL);
            return bRet;
        }
    } break;
    case 0:
    {
        if (false == pCryptMgr->Encrypt(KT_MSK_MKD, (u8*)pRN, sizeof(u256), &pCG, nCG))
        {
            LOGMSG("Execute_Enc_Get_Rnd_Number: Encrypting random number with MKD returned error. Returning error %d.",
                CExecStatus::CMD_ERR_CRYPTO_FAIL);
            return bRet;
        }
    } break;
    case 2:
    {
        return CExecStatus::CMD_EXEC_OK; // No encryption just return RND.
    }
    default:
    {
        LOGMSG("Execute_Enc_Get_Rnd_Number: Invalid parameter (%d). Returning error %d.",
            isEncryptedWithSk, CExecStatus::CMD_ERR_BAD_PARAMETER);
        return CExecStatus::CMD_ERR_BAD_PARAMETER;
    }
    }

    // if size is u256 then we have the right packet. Copy and ret ok
    if (nCG == sizeof(u256))
    {
        memcpy(pRN, pCG, nCG);
        bRet = CExecStatus::CMD_EXEC_OK;
    }

    FREE_MEM(pCG);

    return bRet;
}


CExecStatus        CmdExecutiveMSK00::Execute_Enc_Factory_Set_Key(u8* pInCG, uint nInCGSize, u256 pDigSig)
{
    CryptoMgrMSK00* pCryptMgr = CryptoMgrMSK00::GetInstance();
    if (false == pCryptMgr->SetMKD(pInCG, nInCGSize, pDigSig))
    {
        LOGMSG("Execute_Enc_Factory_Set_Key:Set key (MKD) returned error. Returning error %d.",
            CExecStatus::CMD_ERR_CRYPTO_FAIL);
        return CExecStatus::CMD_ERR_CRYPTO_FAIL;
    }

    return CExecStatus::CMD_EXEC_OK;
}


CExecStatus        CmdExecutiveMSK00::Execute_Enc_Capture(_V100_CAPTURE_TYPE nCapType)
{
    // Currently these are the only types supported.
    switch (nCapType)
    {
    case CAPTURE_IMAGE:
    case CAPTURE_ENROLL_1:
    case CAPTURE_ENROLL_2:
    case CAPTURE_ENROLL_3:
    case CAPTURE_VERIFY:
    {
    } break;

    default:
    {
        LOGMSG("Execute_Enc_Capture:Invalid capture type (%d). Returning error %d.",
            nCapType, CExecStatus::CMD_ERR_BAD_PARAMETER);
        return CExecStatus::CMD_ERR_BAD_PARAMETER;
    }
    };


    IEnrollSvc* pES = ISensorInstance::GetInstance()->GetCfgMgr()->GetEnrollSvc();
    if (ENROLL_OK != pES->SetCaptureMode(nCapType))
    {
        LOGMSG("Execute_Enc_Capture:Set capture mode returned error. Returning error %d.",
            CExecStatus::CMD_ERR_NOT_SUPPORTED);
        return CExecStatus::CMD_ERR_NOT_SUPPORTED;
    }

    CExecStatus CERC = CmdExecutiveCommon::GetInstance().Execute_Arm_Trigger(TRIGGER_ON);

    // set capttype on failure like legacy
    if (CExecStatus::CMD_EXEC_OK != CERC)
    {
        LOGMSG("Execute_Enc_Capture:Arm trigger returned error %d.",
            CERC);
        pES->SetCaptureMode(CAPTURE_IMAGE);
    }

    return CERC;
}


CExecStatus        CmdExecutiveMSK00::Execute_Enc_Enroll()
{
    IEnrollSvc* pES = ISensorInstance::GetInstance()->GetCfgMgr()->GetEnrollSvc();

    // Generate match scores
    u32 Scores[3] = { 0,0,0 };
    // 1 vs 2
    if (false == pES->ValidateEnrollment(CAPTURE_ENROLL_1, CAPTURE_ENROLL_2, Scores[0]))
    {
        LOGMSG("Execute_Enc_Enroll:Validate enrollment 1 vs 2 returned error. Returning error %d.",
            CExecStatus::CMD_ERR_ENROLL);
        return CExecStatus::CMD_ERR_ENROLL;
    }
    // 1 vs 3
    if (false == pES->ValidateEnrollment(CAPTURE_ENROLL_1, CAPTURE_ENROLL_3, Scores[1]))
    {
        LOGMSG("Execute_Enc_Enroll:Validate enrollment 1 vs 3 returned error. Returning error %d.",
            CExecStatus::CMD_ERR_ENROLL);
        return CExecStatus::CMD_ERR_ENROLL;
    }
    // 2 vs 3
    if (false == pES->ValidateEnrollment(CAPTURE_ENROLL_2, CAPTURE_ENROLL_3, Scores[2]))
    {
        LOGMSG("Execute_Enc_Enroll:Validate enrollment 2 vs 3 returned error. Returning error %d.",
            CExecStatus::CMD_ERR_ENROLL);
        return CExecStatus::CMD_ERR_ENROLL;
    }

    // Choose the best enrollment
    if (ENROLL_OK != reinterpret_cast<EnrollSvcMSK00*>(pES)->PickBestEnrollment())
    {
        LOGMSG("Execute_Enc_Enroll:Pick best enrollment returned error. Returning error %d.",
            CExecStatus::CMD_ERR_ENROLL_SCORE);
        return CExecStatus::CMD_ERR_ENROLL_SCORE;
    }

    return CExecStatus::CMD_EXEC_OK;
}


/***************************************************************************************************/
//1.    Decrypt the InCG buffer for ANSOL and set ANSOL onto CryptMgr
//2.    If enorllment happened then get best enrollment template
//3.    If not, Get the last captured template
//4.    Create BIR record of the template
//5.    Encrypt BIR record in BioData format
/***************************************************************************************************/
CExecStatus        CmdExecutiveMSK00::Execute_Enc_ReturnCapturedBIR(_V100_ENC_BIR_HEADER_PROPRIETARY* pBIRHdr, u8* pInCG, uint  nInCGSize, u8** pOutCG, uint& nOutCGSize, u256 pOutDigSig)
{
    IEnrollSvc* pES = ISensorInstance::GetInstance()->GetCfgMgr()->GetEnrollSvc();
    CryptoMgrMSK00* pCryptMgr = CryptoMgrMSK00::GetInstance();
    DataMgr* pDataMgr = ISensorInstance::GetInstance()->GetDataMgr();
    BioParameters* pBioParam = (&BioParameters::GetInstance());
    u8* pANSOL = NULL;
    uint nANSOLSize = 0;
    TAdapter TA;
    // Decrypt InCg

    if (false == pCryptMgr->Decrypt(KT_MSK_SK, pInCG, nInCGSize, &pANSOL, nANSOLSize))
    {
        LOGMSG("Execute_Enc_ReturnCapturedBIR:Decryption step returned error. Returning error %d.",
            CExecStatus::CMD_ERR_ENCRYPTION_VALIDATION);
        return CExecStatus::CMD_ERR_ENCRYPTION_VALIDATION;
    }

    pCryptMgr->SetANSOL((u128*)pANSOL);
    FREE_MEM(pANSOL);

    _V100_CAPTURE_TYPE nBestEnrollment;
    u8* pTpl;
    uint nTplSize = 0;
    _V100_TEMPLATE_MODE nTplMode = pBioParam->GetTemplateMode();
    u8* pTplRec = NULL;
    uint nTplRecSize = 0;

    /* Has an ENROLL been executed?     If not, merely return the last record */
    if (pES->GetBestEnrollment(nBestEnrollment) != ENROLL_OK)
    {
        pTpl = pDataMgr->GetProbeTemplate();
        nTplSize = pDataMgr->GetProbeTemplateSize();
    }
    else
    {

        pTpl = pDataMgr->GetCapturedTemplate(nBestEnrollment);
        nTplSize = pDataMgr->GetCapturedTemplateSize(nBestEnrollment);
    }


    if (pTpl == NULL || nTplSize == 0)
    {
        LOGMSG("Execute_Enc_ReturnCapturedBIR:No template available. Returning error %d.",
            CExecStatus::CMD_ERR_DATA_UNAVAILABLE);
        return CExecStatus::CMD_ERR_DATA_UNAVAILABLE;
    }



    if (false == TA.CreateBIRTemplateProprietary(pBIRHdr, pTpl, nTplSize, &pTplRec, nTplRecSize, nTplMode))
    {
        LOGMSG("Execute_Enc_ReturnCapturedBIR:Create BIR Template returned error. Returning error %d.",
            CExecStatus::CMD_ERR_BSP);
        return CExecStatus::CMD_ERR_BSP;
    }

    // Perform hardware encryption
    if (false == pCryptMgr->EncryptBioData(pTplRec, nTplRecSize, pOutCG, nOutCGSize, pOutDigSig))
    {
        LOGMSG("Execute_Enc_ReturnCapturedBIR:Encrypt bio data returned error. Returning error %d.",
            CExecStatus::CMD_ERR_CRYPTO_FAIL);
        return CExecStatus::CMD_ERR_CRYPTO_FAIL;
    }

    // TAdapter takes care of pTplRec
    return CExecStatus::CMD_EXEC_OK;
}


/***************************************************************************************************/
//1.    Perform CE_CMD_ENC_RETURNCAPTUREDBIR which also sets the ANSOL onto CryptMgr. Ansol used for BioData format
//2.    If enrollment happened then get best enrollment image in the format requested(WSQ or composite)
//3.    If not, Get the last captured image in the format requested (WSQ or composite)
//4.    Create Image buffer with header and retrieved image
//5.    Encrypt Image buffer in BioData format
/***************************************************************************************************/
CExecStatus        CmdExecutiveMSK00::Execute_Enc_ReturnCapturedBIR_IM(_V100_IMAGE_TYPE imageType, _V100_ENC_BIR_HEADER_PROPRIETARY* pBIRHdr, u8* pInCG, uint  nInCGSize, u8** pOutTplCG,
    uint& nOutTplCGSize, u256 pOutTplDigSig, u8** pOutImgCG, uint& nOutImgCGSize, u256 pOutImgDigSig)
{
    IEnrollSvc* pES = ISensorInstance::GetInstance()->GetCfgMgr()->GetEnrollSvc();
    DataMgr* pDataMgr = ISensorInstance::GetInstance()->GetDataMgr();
    CryptoMgrMSK00* pCryptMgr = CryptoMgrMSK00::GetInstance();

    _V100_INTERFACE_CONFIGURATION_TYPE* pICT = pDataMgr->GetInterfaceConfiguration();

    if ((imageType != IMAGE_COMPOSITE) && (imageType != IMAGE_WSQ))
    {
        LOGMSG("Execute_Enc_ReturnCapturedBIR_IM:Invalid image type (%d). Returning error %d.",
            imageType, CExecStatus::CMD_ERR_NOT_SUPPORTED);
        return CExecStatus::CMD_ERR_NOT_SUPPORTED;
    }

    // ANSOL set in this call
    CExecStatus CERet = Execute_Enc_ReturnCapturedBIR(pBIRHdr, pInCG, nInCGSize, pOutTplCG, nOutTplCGSize, pOutTplDigSig);
    if (CERet != CExecStatus::CMD_EXEC_OK)
    {
        LOGMSG("Execute_Enc_ReturnCapturedBIR_IM:Return captured BIR returned %d.",
            CERet);
        return CERet;
    }


    u8* pImage = nullptr;
    uint nImageSize = 0, nX, nY;
    _V100_CAPTURE_TYPE nBestEnrollment;
    nX = pICT->Composite_Image_Size_X;
    nY = pICT->Composite_Image_Size_Y;

    bool freeImagePointer = false;

    /* Has an ENROLL been executed?     If not, merely return the last captured image in the format requested */
    if (pES->GetBestEnrollment(nBestEnrollment) != ENROLL_OK)
    {
        // NOTE: This will return a pointer to DataMgr's image buffer. Cannot free this memory...
        CmdExecutiveCommon::GetInstance().Execute_Get_Image(imageType, &pImage, nImageSize, 0);
        if (pImage == nullptr || nImageSize == 0)
        {
            LOGMSG("Execute_Enc_ReturnCapturedBIR_IM:No image available. Returning error %d.",
                CExecStatus::CMD_ERR_DATA_UNAVAILABLE);
            return CExecStatus::CMD_ERR_DATA_UNAVAILABLE;
        }
    }
    else
    {
        freeImagePointer = true;
        // We have best enrollment. Retrieve in the format requested
        nImageSize = nX * nY;
        pImage = (u8*)MALLOC(nImageSize);
        if (pImage == nullptr)
        {
            LOGMSG("Execute_Enc_ReturnCapturedBIR_IM:Failed to allocate memory. Returning error %d.",
                CExecStatus::CMD_ERR_OUT_OF_MEMORY);
            return CExecStatus::CMD_ERR_OUT_OF_MEMORY;
        }
        if (false == pDataMgr->GetCapturedImageData(nBestEnrollment, pImage, nImageSize))
        {
            // TODO: Don't think this is the correct error. Might be an internal error
            LOGMSG("Execute_Enc_ReturnCapturedBIR_IM:Failed to get captured image data. Returning error %d.",
                CExecStatus::CMD_ERR_DATA_UNAVAILABLE);
            FREE(pImage);
            return CExecStatus::CMD_ERR_DATA_UNAVAILABLE;
        }
        if (pImage == nullptr || nImageSize == 0)
        {
            LOGMSG("Execute_Enc_ReturnCapturedBIR_IM:No best enrollment image available. Returning error %d.",
                CExecStatus::CMD_ERR_DATA_UNAVAILABLE);
            FREE(pImage);
            return CExecStatus::CMD_ERR_DATA_UNAVAILABLE;
        }
        if (imageType == IMAGE_WSQ)
        {
            //Get WSQ image
            uint nBPP = 8;
            uint nDPI = pICT->Composite_DPI;
            uchar* pCompressedImg = NULL;
            // Compress it now
            int nCompressedImgSz = 0;
            uint nCompressionRatio = (&BioParameters::GetInstance())->GetCompressionRatio();// Get compression ratio
            float r_bitrate = (1 / (float)nCompressionRatio) * 8;
            if (Lumi_Encode_WSQ(pImage, r_bitrate, (int)nX, (int)nY, (int)nBPP, (int)nDPI, &pCompressedImg, &nCompressedImgSz))
            {
                LOGMSG("Execute_Enc_ReturnCapturedBIR_IM:Encode WSQ image returned error. Returning error %d.",
                    CExecStatus::CMD_ERR_BSP);
                FREE(pImage);
                return CExecStatus::CMD_ERR_BSP;
            }
            else
            {
                FREE(pImage);
                pImage = pCompressedImg;
                nImageSize = nCompressedImgSz;
            }
        }

    }
    // Perform software encryption
    if (false == pCryptMgr->EncryptBioData(pImage, nImageSize, pOutImgCG, nOutImgCGSize, pOutImgDigSig))
    {
        LOGMSG("Execute_Enc_ReturnCapturedBIR_IM:Encrypt bio data returned error. Returning error %d.",
            CExecStatus::CMD_ERR_CRYPTO_FAIL);
        if (freeImagePointer && pImage != nullptr)
        {
            FREE(pImage);
        }
        return CExecStatus::CMD_ERR_CRYPTO_FAIL;
    }
    if (freeImagePointer && pImage != nullptr)
    {
        FREE(pImage);
    }

    return CExecStatus::CMD_EXEC_OK;
}


CExecStatus        CmdExecutiveMSK00::Execute_Enc_Set_Parameters(u8* pInCG, uint nInCGSize, u8* pANSOL, uint nANSOLSize, _V100_ENC_SPOOF_PROTECTION_LEVEL nSpoofProtection, int nTimeout)
{
    // Get Crypto
    BioParameters* pBioParams = (&BioParameters::GetInstance());
    CryptoMgrMSK00* pCryptMgr = CryptoMgrMSK00::GetInstance();

    CExecStatus CERet = CExecStatus::CMD_EXEC_OK;

    if (nInCGSize > 0)
    {
        if (false == pCryptMgr->SetSK(pInCG, nInCGSize, pANSOL, nANSOLSize))
        {
            LOGMSG("Execute_Enc_Set_Parameters:Set key (SK) returned error. Returning error %d.",
                CExecStatus::CMD_ERR_BSP);
            CERet = CExecStatus::CMD_ERR_BSP;
        }
    }
    if (nSpoofProtection != SPOOF_PROTECT_DONT_CHANGE)
    {
        if (false == pBioParams->SetSpoofProtLevel(nSpoofProtection))
        {
            LOGMSG("Execute_Enc_Set_Parameters:Set spoof protection level (%d) returned error. Returning error %d.",
                nSpoofProtection, CExecStatus::CMD_ERR_BSP);
            CERet = CExecStatus::CMD_ERR_BSP;
        }
    }
    if (nTimeout != -1)
    {
        if (false == pBioParams->SetCaptureTimeOut((u16)nTimeout))
        {
            LOGMSG("Execute_Enc_Set_Parameters:Set capture timeout (%d) returned error. Returning error %d.",
                nTimeout, CExecStatus::CMD_ERR_BSP);
            CERet = CExecStatus::CMD_ERR_BSP;
        }
    }

    return CERet;
}


CExecStatus        CmdExecutiveMSK00::Execute_Enc_Get_KCV(u8** pKCV, uint nKCVSize)
{
    if (nKCVSize > KCV_SIZE)
    {
        LOGMSG("Execute_Enc_Get_KCV:Invalid key check value size (%d). Returning error %d.",
            nKCVSize, CExecStatus::CMD_ERR_DATA_UNAVAILABLE);
        return CExecStatus::CMD_ERR_DATA_UNAVAILABLE;
    }

    //Get the KCV_SIZE allowed and client can copy the required size.
    *pKCV = (u8*)MALLOC(KCV_SIZE);
    memset(*pKCV, 0, KCV_SIZE);

    if (*pKCV == NULL)
    {
        LOGMSG("Execute_Enc_Get_KCV:Unable to allocate memory for key check value. Returning error %d.",
            CExecStatus::CMD_ERR_OUT_OF_MEMORY);
        return CExecStatus::CMD_ERR_OUT_OF_MEMORY;
    }

    if (false == CryptoMgrMSK00::GetInstance()->GetSKCV(*pKCV, KCV_SIZE))
    {
        FREE(*pKCV);
        *pKCV = NULL;
        LOGMSG("Execute_Enc_Get_KCV:Get SK key check value returned error. Returning error %d.",
            CExecStatus::CMD_ERR_BSP);
        return CExecStatus::CMD_ERR_BSP;
    }
    return CExecStatus::CMD_EXEC_OK;
}


/***************************************************************************************************/
//1.    Decrypt the TemplateCG buffer for Tpl and size
//2.    Decrypt the Ansol CG
//3.    Validate DigSig - Decrypt DigSig for HashValProvided and Cal Hash of [DevID ANSOL ANBIO FMRReq Tpl ]. Verify CalHash is same as provided one
//4.    Retrieve the verify template to match with tpl provided.
//5.    Construct verify match results and encrypt to return.
/***************************************************************************************************/
CExecStatus        CmdExecutiveMSK00::Execute_Enc_VerifyMatch(uint nFMRRequested, u8* pInCG, uint  nInCGSize, u256  pInDigSig, u256  pInANSOLCG, u8** pOutCG,
    uint& nOutCGSize, uint* nFMRScore, uint* nResult)
{
    u8* pTpl = NULL, * pANSOL = NULL;
    uint nTplSize = 0, nANSOLSize = 0;
    CryptoMgrMSK00* pCryptMgr = CryptoMgrMSK00::GetInstance();
    BioParameters* pBioParam = (&BioParameters::GetInstance());
    DataMgr* pDataMgr = ISensorInstance::GetInstance()->GetDataMgr();

    // Step 1
    if (false == pCryptMgr->Decrypt(KT_MSK_SK, pInCG, nInCGSize, &pTpl, nTplSize))
    {
        LOGMSG("Execute_Enc_VerifyMatch:Decryption step for template returned error. Returning error %d.",
            CExecStatus::CMD_ERR_ENCRYPTION_VALIDATION);
        return CExecStatus::CMD_ERR_ENCRYPTION_VALIDATION;
    }
    AutoFreePtr afp_pTpl(pTpl);

    // Step 2
    if (false == pCryptMgr->Decrypt(KT_MSK_SK, pInANSOLCG, sizeof(u256), &pANSOL, nANSOLSize))
    {
        LOGMSG("Execute_Enc_VerifyMatch:Decryption step for ANSOL returned error. Returning error %d.",
            CExecStatus::CMD_ERR_ENCRYPTION_VALIDATION);
        return CExecStatus::CMD_ERR_ENCRYPTION_VALIDATION;
    }
    AutoFreePtr afp_pANSOL(pANSOL);

    //Step 3
    uint nBuffToValSize = DEVID_SIZE + ANSOL_SIZE + ANBIO_SIZE + sizeof(nFMRRequested) + nTplSize;
    u8* pBuffToVal = (u8*)MALLOC(nBuffToValSize);
    if (pBuffToVal == NULL)
    {
        LOGMSG("Execute_Enc_VerifyMatch:Unable to allocate memory for validation block. Returning error %d.",
            CExecStatus::CMD_ERR_OUT_OF_MEMORY);
        return CExecStatus::CMD_ERR_OUT_OF_MEMORY;
    }
    AutoFreePtr afp_pBuffToVal(pBuffToVal);
    memset(pBuffToVal, 0, nBuffToValSize);

    u8* pTempBuff = pBuffToVal;
    char pDevID[DEVID_SIZE];
    u128 ANBIO;
    pCryptMgr->GetDeviceID(pDevID);
    memcpy(pTempBuff, pDevID, DEVID_SIZE);
    pTempBuff += DEVID_SIZE;
    memcpy(pTempBuff, pANSOL, ANSOL_SIZE);
    pTempBuff += ANSOL_SIZE;

    if (false == pCryptMgr->GetANBIO(&ANBIO))
    {
        LOGMSG("Execute_Enc_VerifyMatch:Get ANBIO returned error. Returning error %d.",
            CExecStatus::CMD_ERR_BSP);
        return CExecStatus::CMD_ERR_BSP;
    }

    memcpy(pTempBuff, ANBIO, ANBIO_SIZE);
    pTempBuff += ANBIO_SIZE;

    //TODO use a function call to swap bytes
    unsigned char* pFlip = (unsigned char*)&nFMRRequested;
    pTempBuff[0] = pFlip[3];
    pTempBuff[1] = pFlip[2];
    pTempBuff[2] = pFlip[1];
    pTempBuff[3] = pFlip[0];
    pTempBuff += sizeof(nFMRRequested);

    memcpy(pTempBuff, pTpl, nTplSize);
    pTempBuff += nTplSize;

    if (false == pCryptMgr->ValidateDigSig(pBuffToVal, nBuffToValSize, pInDigSig))
    {
        LOGMSG("Execute_Enc_VerifyMatch:Validate digital signature returned error. Returning error %d.",
            CExecStatus::CMD_ERR_ENCRYPTION_VALIDATION);
        return CExecStatus::CMD_ERR_ENCRYPTION_VALIDATION;
    }

    //Step 4 & 5
    _V100_ENC_VERIFY_MATCH_RESULT VerMatchResult;
    memset(&VerMatchResult, 0, sizeof(_V100_ENC_VERIFY_MATCH_RESULT));
    CExecStatus stat = VerifyBIRAgainstCaptured(pTpl, nFMRRequested, nResult, nFMRScore, VerMatchResult);
    if (CExecStatus::CMD_EXEC_OK != stat)
    {
        LOGMSG("Execute_Enc_VerifyMatch:Verify BIR against captured returned error %d.",
            stat);
        return stat;
    }
    memcpy(VerMatchResult.ANSOL, pANSOL, ANSOL_SIZE);

    if (false == pCryptMgr->Encrypt(KT_MSK_SK, (u8*)(&VerMatchResult), sizeof(_V100_ENC_VERIFY_MATCH_RESULT), pOutCG, nOutCGSize))
    {
        LOGMSG("Execute_Enc_VerifyMatch:Encryption step returned error. Returning error %d.",
            CExecStatus::CMD_ERR_ENCRYPTION_VALIDATION);
        return CExecStatus::CMD_ERR_ENCRYPTION_VALIDATION;
    }

    return CExecStatus::CMD_EXEC_OK;
}


CExecStatus        CmdExecutiveMSK00::Execute_Enc_IdentifyMatch(uint nFMRRequested, u8* pInCG, uint  nInCGSize, u256  pInDigSig, u256  pInANSOLCG, u8** pOutCG,
    uint* nOutCGSize, uint* nFMRScore, uint* nResult)
{
    u8* pTpl = NULL, * pANSOL = NULL;
    uint nTplSize = 0, nANSOLSize = 0;
    CryptoMgrMSK00* pCryptMgr = CryptoMgrMSK00::GetInstance();
    BioParameters* pBioParam = (&BioParameters::GetInstance());
    DataMgr* pDataMgr = ISensorInstance::GetInstance()->GetDataMgr();

    if (false == pCryptMgr->Decrypt(KT_MSK_SK, pInANSOLCG, sizeof(u256), &pANSOL, nANSOLSize))
    {
        LOGMSG("Execute_Enc_IdentifyMatch:Decryption step returned error. Returning error %d.",
            CExecStatus::CMD_ERR_ENCRYPTION_VALIDATION);
        return CExecStatus::CMD_ERR_ENCRYPTION_VALIDATION;
    }
    AutoFreePtr afp_pANSOL(pANSOL);

    uint nBuffToValSize = DEVID_SIZE + ANSOL_SIZE + ANBIO_SIZE + sizeof(nFMRRequested);
    u8* pBuffToVal = (u8*)MALLOC(nBuffToValSize);
    if (pBuffToVal == NULL)
    {
        LOGMSG("Execute_Enc_IdentifyMatch:Unable to allocate memory for validation block. Returning error %d.",
            CExecStatus::CMD_ERR_OUT_OF_MEMORY);
        return CExecStatus::CMD_ERR_OUT_OF_MEMORY;
    }
    AutoFreePtr afp_pBuffToVal(pBuffToVal);
    memset(pBuffToVal, 0, nBuffToValSize);

    u8* pTempBuff = pBuffToVal;
    char pDevID[DEVID_SIZE];
    u128 ANBIO;
    pCryptMgr->GetDeviceID(pDevID);
    memcpy(pTempBuff, pDevID, DEVID_SIZE);
    pTempBuff += DEVID_SIZE;
    memcpy(pTempBuff, pANSOL, ANSOL_SIZE);
    pTempBuff += ANSOL_SIZE;

    pCryptMgr->GetANBIO(&ANBIO);
    memcpy(pTempBuff, ANBIO, ANBIO_SIZE);
    pTempBuff += ANBIO_SIZE;

    //TODO use a function call to swap bytes
    unsigned char* pFlip = (unsigned char*)&nFMRRequested;
    pTempBuff[0] = pFlip[3];
    pTempBuff[1] = pFlip[2];
    pTempBuff[2] = pFlip[1];
    pTempBuff[3] = pFlip[0];
    pTempBuff += sizeof(nFMRRequested);

    if (false == pCryptMgr->ValidateDigSig(pBuffToVal, nBuffToValSize, pInDigSig))
    {
        LOGMSG("Execute_Enc_IdentifyMatch:Validate digital signature returned error. Returning error %d.",
            CExecStatus::CMD_ERR_ENCRYPTION_VALIDATION);
        return CExecStatus::CMD_ERR_ENCRYPTION_VALIDATION;
    }

    // pInCG is actually a series of cryptograms
    // seperated by headers, so lets iterate
    _V100_ENC_VERIFY_MATCH_RESULT verMatchResult;
    memset(&verMatchResult, 0, sizeof(_V100_ENC_VERIFY_MATCH_RESULT));
    bool bRecordFound = false;
    {
        typedef struct
        {
            u32     nLength;
            void* pDummy;
        } BIO_DATA_HEADER;

        bool bCounted = false;
        u32 nNumberOfRecords = 0;
        u32 nTotalLength = 0;
        BIO_DATA_HEADER* pNode = (BIO_DATA_HEADER*)pInCG;
        BIO_DATA_HEADER* cHeader[20];

        while (bCounted == false)
        {
            if (pNode->nLength > 1024 * 2)
            {
                LOGMSG("Execute_Enc_IdentifyMatch:Invalid bio data header size (%d). Returning error %d.",
                    pNode->nLength, CExecStatus::CMD_ERR_ENCRYPTION_VALIDATION);
                return CExecStatus::CMD_ERR_ENCRYPTION_VALIDATION;
            }
            nTotalLength += pNode->nLength + sizeof(BIO_DATA_HEADER);
            // Save the header location
            cHeader[nNumberOfRecords] = pNode;
            // Iterate to the next header.
            pNode = (BIO_DATA_HEADER*)((unsigned char*)pNode + pNode->nLength + sizeof(BIO_DATA_HEADER));
            // Are we at the end?
            if (nTotalLength >= nInCGSize)
            {
                bCounted = true;
            }
            nNumberOfRecords++;
        }
        // For each BIR template, decrypt.
        for (uint ii = 0; ii < nNumberOfRecords; ii++)
        {
            u8* pCryptoGramIn = (u8*)cHeader[ii] + sizeof(BIO_DATA_HEADER);
            // Decrypt this node
            if (false == pCryptMgr->Decrypt(KT_MSK_SK, pCryptoGramIn, cHeader[ii]->nLength, &pTpl, nTplSize))
            {
                LOGMSG("Execute_Enc_IdentifyMatch:Decryption step for BIR CG %d returned error. Returning error %d.",
                    ii, CExecStatus::CMD_ERR_ENCRYPTION_VALIDATION);
                return CExecStatus::CMD_ERR_ENCRYPTION_VALIDATION;
            }
            AutoFreePtr afp_pBIR(pTpl);
            // Perform the verification.
            CExecStatus stat = VerifyBIRAgainstCaptured(pTpl, nFMRRequested, nResult, nFMRScore, verMatchResult);

            if (CExecStatus::CMD_EXEC_OK != stat)
            {
                LOGMSG("Execute_Enc_IdentifyMatch:Verify BIR %d against captured returned error %d.",
                    ii, stat);
                return stat;
            }
            verMatchResult.FMR = *nFMRScore;
            // Did we get a match?
            if (verMatchResult.MatchResult == 1)
            {
                // Retain the index...
                verMatchResult.MatchResult = ii + 1;
                *nResult = ii + 1;
                bRecordFound = true;
                ii = nNumberOfRecords; // Fake break, I'm a snake.
            }
        }
    }
    memcpy(verMatchResult.ANSOL, pANSOL, ANSOL_SIZE);
    //
    //u256 pOutDigSig;
    *pOutCG = (u8*)MALLOC(sizeof(u256));
    *nOutCGSize = sizeof(u256);

    // Hash Result.
    if (false == pCryptMgr->CreateDigSig((u8*)&verMatchResult, sizeof(_V100_ENC_VERIFY_MATCH_RESULT), *pOutCG))
    {
        LOGMSG("Execute_Enc_IdentifyMatch:Create digital signature returned error. Returning error %d.",
            CExecStatus::CMD_ERR_ENCRYPTION_VALIDATION);
        return CExecStatus::CMD_ERR_ENCRYPTION_VALIDATION;
    }

    return CExecStatus::CMD_EXEC_OK;
}

CExecStatus        CmdExecutiveMSK00::Execute_Enc_Get_Spoof_Score(u8* pInCG, uint nInCGSize, u8** pOutCG, uint* nOutCGSize)
{
    CryptoMgrMSK00* pCryptMgr = CryptoMgrMSK00::GetInstance();

    u8* pANSOL = NULL;
    uint  nANSOLSize = 0;

    // Decrypt
    if (false == pCryptMgr->Decrypt(KT_MSK_SK, pInCG, nInCGSize, &pANSOL, nANSOLSize))
    {
        LOGMSG("Execute_Enc_Get_Spoof_Score:Decryption step returned error. Returning error %d.",
            CExecStatus::CMD_ERR_ENCRYPTION_VALIDATION);
        return CExecStatus::CMD_ERR_ENCRYPTION_VALIDATION;
    }
    AutoFreePtr afp_pANSOL(pANSOL);


    if (nANSOLSize != ANSOL_SIZE)
    {
        LOGMSG("Execute_Enc_Get_Spoof_Score:ANSOL size is incorrect (%d). Returning error %d.",
            nANSOLSize, CExecStatus::CMD_ERR_BAD_PARAMETER);
        return CExecStatus::CMD_ERR_BAD_PARAMETER;
    }
    uint nOutSize = ANSOL_SIZE + sizeof(u32);
    uint nPadSize = nOutSize + (BLOCK_SIZE - nOutSize % BLOCK_SIZE);
    u8* pResult = (u8*)MALLOC(nPadSize);
    memset(pResult, 0, nPadSize);

    // Business logic
    {
        DataMgr* pDMgr = ISensorInstance::GetInstance()->GetDataMgr();
        // Copy ANSOL back into spoofResult
        memcpy(pResult, pANSOL, ANSOL_SIZE);
        // Copy Spoof into structure
        u32 nSpoofScore = pDMgr->GetLastSpoofScore();
        memcpy(pResult + ANSOL_SIZE, &nSpoofScore, sizeof(u32));
    }
    // Encrypt
    u8* pCryptogram = NULL;
    uint cryptogramSize = 0;
    if (false == pCryptMgr->Encrypt(KT_MSK_SK, pResult, nPadSize, pOutCG, cryptogramSize))
    {
        FREE(pResult);
        LOGMSG("Execute_Enc_Get_Spoof_Score:Encryption step returned error. Returning error %d.",
            CExecStatus::CMD_ERR_CRYPTO_FAIL);
        return CExecStatus::CMD_ERR_CRYPTO_FAIL;
    }
    FREE(pResult);
    *nOutCGSize = cryptogramSize;
    return CExecStatus::CMD_EXEC_OK;
}


CExecStatus     CmdExecutiveMSK00::Execute_Enc_Clear()
{
    // this function cannot fail
    return CmdExecutiveCommon::GetInstance().Execute_Enc_Clear();
}


CExecStatus       CmdExecutiveMSK00::Execute_Enc_Get_Diag_Status(_V100_ENC_DIAG_STATUS& diagStatus)
{
    // this function cannot fail
    return CmdExecutiveBase::GetInstance().Execute_Enc_Get_Diag_Status(diagStatus);
}


CExecStatus        CmdExecutiveMSK00::Execute_Enc_Get_KeyVersion(_V100_ENC_KEY_TYPE nKeyType, u16& nKeyVer, _V100_ENC_KEY_MODE& nKeyMode, u8* pKCV)
{
    u16 keyMode = (u16)nKeyMode;
    if (!CryptoMgrMSK00::GetInstance()->GetKeyInfo(nKeyType, nKeyVer, keyMode, pKCV))
    {
        LOGMSG("Execute_Enc_Get_KeyVersion:Get key info for Key %d returned error. Returning error %d.",
            nKeyType, CExecStatus::CMD_ERR_CRYPTO_FAIL);
        return CExecStatus::CMD_ERR_CRYPTO_FAIL;
    }
    nKeyMode = (_V100_ENC_KEY_MODE)keyMode;
    return CExecStatus::CMD_EXEC_OK;
}


CExecStatus CmdExecutiveMSK00::VerifyBIRAgainstCaptured(u8* pTpl, u32 nFMRRequested, uint* nResult, uint* nFMRScore, _V100_ENC_VERIFY_MATCH_RESULT& verMatchResult)
{
    BioParameters* pBioParam = (&BioParameters::GetInstance());
    DataMgr* pDataMgr = ISensorInstance::GetInstance()->GetDataMgr();
    TAdapter TA;

    uchar* pInTpl = NULL;
    uint nInTplSize = 0;
    _V100_TEMPLATE_MODE nTplMode = pBioParam->GetTemplateMode();
    _V100_FIRMWARE_CONFIG fwconfig = ISensorInstance::GetInstance()->GetCfgMgr()->GetCurrentCfg();
    if (fwconfig == FIRMWARE_TM_MSK00)
    {
        if (false == TA.Get378TemplateFromBIRProprietary(pTpl, &pInTpl, nInTplSize, nTplMode))
        {
            LOGMSG("VerifyBIRAgainstCaptured:Get template from BIR (proprietary) returned error. Returning error %d.",
                CExecStatus::CMD_ERR_BAD_PARAMETER);
            return CExecStatus::CMD_ERR_BAD_PARAMETER;
        }
    }
    else
    {
        if (false == TA.Get378TemplateFromBIR(pTpl, &pInTpl, nInTplSize, nTplMode))
        {
            LOGMSG("VerifyBIRAgainstCaptured:Get template from BIR returned error. Returning error %d.",
                CExecStatus::CMD_ERR_BAD_PARAMETER);
            return CExecStatus::CMD_ERR_BAD_PARAMETER;
        }
    }


    uint nMatchScore = 0;
    u8* pVerifyTpl = pDataMgr->GetCapturedTemplate(CAPTURE_VERIFY);
    uint nVerifyTplSize = pDataMgr->GetCapturedTemplateSize(CAPTURE_VERIFY);
    if (pVerifyTpl == nullptr || nVerifyTplSize == 0)
    {
        LOGMSG("VerifyBIRAgainstCaptured:No verify template available. Returning error %d.",
            CExecStatus::CMD_ERR_DATA_UNAVAILABLE);
        return CExecStatus::CMD_ERR_DATA_UNAVAILABLE;
    }

    if (false == ISensorInstance::GetInstance()->GetCfgMgr()->GetEnrollSvc()->Match(pInTpl, nInTplSize, pVerifyTpl, nVerifyTplSize, nMatchScore))
    {
        LOGMSG("VerifyBIRAgainstCaptured:Failed to match templates. Returning error %d.", CExecStatus::CMD_ERR_REMOTE_EXEC_FAIL);
        return CExecStatus::CMD_ERR_REMOTE_EXEC_FAIL;
    }

    memset(&verMatchResult, 0, sizeof(_V100_ENC_VERIFY_MATCH_RESULT));
    uint FMRScore = ISensorInstance::GetInstance()->GetCfgMgr()->GetEnrollSvc()->GetFMRScore(pInTpl, nInTplSize, pVerifyTpl, nVerifyTplSize, nMatchScore);

    if (FMRScore < nFMRRequested)
    {
        verMatchResult.MatchResult = 1;
    }
    else
    {
        verMatchResult.MatchResult = 0;
    }
    verMatchResult.FMR = FMRScore;
    *nResult = verMatchResult.MatchResult;
    *nFMRScore = verMatchResult.FMR;

    //TAdapter takes care of pInTpl
    return CExecStatus::CMD_EXEC_OK;
}

/***************************************************************************************************/
//1.    Decrypt the TemplatesCG buffer for TplsData and size
//2.    Validate DigSig - Decrypt DigSig for HashValProvided and Cal Hash of [Tpls ]. Verify CalHash is same as provided one
//3.    Validate ANBIO- Get ANBIO from CryptoMgr and compare with one provided in TplsData
//4.    Set ANSOL provided onto CryptoMgr
//5.    Create and issue Macromessage
/***************************************************************************************************/
CExecStatus     CmdExecutiveMSK00::Execute_Enc_VerifyMatch_Many(u8* pInCG, uint nInCGSize, u256 pInDigSig)
{
    u8* pTplsData = nullptr;
    uint nTplsDataSize = 0, nANSOLSize = 0;
    CryptoMgrMSK00* pCryptMgr = CryptoMgrMSK00::GetInstance();

    u128 pANBIO;

    // Step 1 -Perform software decrypt
    if (false == pCryptMgr->Decrypt(KT_MSK_SK, pInCG, nInCGSize, &pTplsData, nTplsDataSize))
    {
        return CExecStatus::CMD_ERR_ENCRYPTION_VALIDATION;
    }
    AutoFreePtr afp_pTpl(pTplsData);

    // Step 1
    if (false == pCryptMgr->ValidateDigSig(pTplsData, nTplsDataSize, pInDigSig))
    {
        return CExecStatus::CMD_ERR_ENCRYPTION_VALIDATION;
    }

    // Step 3
    _V100_ENC_VERIFY_MANY_HDR* pPacket = (_V100_ENC_VERIFY_MANY_HDR*)(pTplsData);

    if (false == pCryptMgr->GetANBIO(&pANBIO))
    {
        return CExecStatus::CMD_ERR_BSP;
    }


    if (memcmp(pPacket->ANBIO, pANBIO, ANBIO_SIZE) != 0)
    {
        return CExecStatus::CMD_ERR_ENCRYPTION_VALIDATION;
    }

    // Step 4
    if (false == pCryptMgr->SetANSOL((u128*)(&pPacket->ANSOL)))
    {
        return CExecStatus::CMD_ERR_BSP;
    }

    //Step 5
    if (CmdExecutiveBase::CreateAndPostMacroMessage(CMD_ENC_VERIFYMATCH_MANY, pTplsData, nTplsDataSize, App_Busy_Macro) == false)
    {
        return CExecStatus::CMD_ERR_OUT_OF_MEMORY;
    }

    return CExecStatus::CMD_EXEC_OK;
}

/***************************************************************************************************/
//1.    Create VerifyMatch_Results Data = [_V100_ENC_VERIFY_MANY_RST_HDR + nNumRecords*_V100_ENC_VERIFY_MANY_RESULT]
//2.    Encrypt the data
//3.    Create Dig Sig
/***************************************************************************************************/
CExecStatus     CmdExecutiveMSK00::Execute_Enc_VerifyMatch_Result(u8** pOutCG, uint& nOutCGSize, u256 pOutDigSig)
{
    DataMgr* pDataMgr = ISensorInstance::GetInstance()->GetDataMgr();
    CryptoMgrMSK00* pCryptMgr = CryptoMgrMSK00::GetInstance();

    uint nNumVMRs = pDataMgr->GetEncCaptureResultLength();
    // Calculate Data length.
    uint nVMRDataSize = sizeof(_V100_ENC_VERIFY_MANY_RST_HDR) + nNumVMRs * sizeof(_V100_ENC_VERIFY_MANY_RESULT);
    uchar* pVMRData = (uchar*)MALLOC(nVMRDataSize);
    if (pVMRData == NULL)
    {
        return CExecStatus::CMD_ERR_OUT_OF_MEMORY;
    }
    AutoFreePtr afp_pVMRData(pVMRData);
    memset(pVMRData, 0, nVMRDataSize);

    _V100_ENC_VERIFY_MANY_RST_HDR* pVMRHdr = (_V100_ENC_VERIFY_MANY_RST_HDR*)pVMRData;
    pVMRHdr->nNumberOfMatchesAttempted = nNumVMRs;

    // Get ANSOL from CryptoMgr
    u128 pANSOL;
    if (false == pCryptMgr->GetANSOL(&pANSOL))
    {
        return CExecStatus::CMD_ERR_BSP;
    }

    memcpy(&(pVMRHdr->ANSOL), &pANSOL, sizeof(u64));
    // Pull the records.
    _V100_ENC_VERIFY_MANY_RESULT* pVMResult = (_V100_ENC_VERIFY_MANY_RESULT*)(pVMRData + sizeof(_V100_ENC_VERIFY_MANY_RST_HDR));
    for (uint ii = 0; ii < nNumVMRs; ii++, pVMResult++)
    {
        V100_ENC_VERIFY_MANY_RESULT_LL* pRes = pDataMgr->GetEncCaptureResult(ii);
        memcpy(pVMResult, &(pRes->result), sizeof(_V100_ENC_VERIFY_MANY_RESULT));
    }

    if (false == pCryptMgr->Encrypt(KT_MSK_SK, pVMRData, nVMRDataSize, pOutCG, nOutCGSize))
    {
        return CExecStatus::CMD_ERR_ENCRYPTION_VALIDATION;
    }

    if (false == pCryptMgr->CreateDigSig(pVMRData, nVMRDataSize, pOutDigSig))
    {
        return CExecStatus::CMD_ERR_ENCRYPTION_VALIDATION;
    }

    return CExecStatus::CMD_EXEC_OK;
}


bool CmdExecutiveMSK00::IsMKDLoaded()
{
    return CryptoMgrMSK00::GetInstance()->IsMKDLoaded();
}
