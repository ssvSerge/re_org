#include "AutoHeapBuffer.h"
#include "CaptureStats.h"
#include "CmdExecutiveHYB02.h"
#include "CmdExecutiveCommon.h"
#include "IEnrollSvc.h"
#include "EnrollSvc.h"
#include "DataMgr.h"
#include "CryptoMgrHYB02.h"
#include "BioParameters.h"
#include "CfgMgr.h"
#include "TAdapter.h"
#include "wsqWrapper.h"
#include "CriticalErrorLog.h"

#define CRYPTO_MGR CryptoMgrHYB02::GetInstance()

// Marshal cryptoMgrError to CmdExecutive Error codes
CExecStatus MarshalCMErrToCEErr(int nCMErr);

CExecStatus    CmdExecutiveHYB02::Execute_Enroll(_V100_CAPTURE_TYPE& nBestInsertion)
{
    IEnrollSvc* pES = ISensorInstance::GetInstance()->GetCfgMgr()->GetEnrollSvc();

    if( false == pES->IsEnrolled() )
    {
        return CExecStatus::CMD_ERR_ENROLL;
    }

    EnrollStatus es = pES->GetBestEnrollment(nBestInsertion);

    if( es != ENROLL_OK )
    {
        return CExecStatus::CMD_ERR_ENROLL;
    }
    return CExecStatus::CMD_EXEC_OK;

}


CExecStatus CmdExecutiveHYB02::Execute_Capture(_V100_CAPTURE_TYPE nCapType)
{
    // Currently these are the only types supported.
    switch(nCapType)
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
            return CExecStatus::CMD_ERR_NOT_SUPPORTED;
        }
    };

    IEnrollSvc* pES = ISensorInstance::GetInstance()->GetCfgMgr()->GetEnrollSvc();
    if (ENROLL_OK != pES->SetCaptureMode(nCapType))
    {
         return CExecStatus::CMD_ERR_NOT_SUPPORTED;
    }

    CExecStatus CERC = CmdExecutiveCommon::GetInstance().Execute_Arm_Trigger(TRIGGER_ON);

    // set capttype on failure like legacy
    if (CExecStatus::CMD_EXEC_OK != CERC)
    {
        pES->SetCaptureMode(CAPTURE_IMAGE);
    }

    return CERC;
}


CExecStatus CmdExecutiveHYB02::Execute_Clear()
{
    // this function cannot fail
    return CmdExecutiveCommon::GetInstance().Execute_Enc_Clear();
}


CExecStatus CmdExecutiveHYB02::Execute_Set_Option(_V100_OPTION_TYPE OptionType, uchar* pOptData, uint nOptDataSize)
{
    DataMgr* pDataMgr = ISensorInstance::GetInstance()->GetDataMgr();
    CExecStatus status = CExecStatus::CMD_EXEC_OK;

    switch(OptionType)
    {
        case OPTION_SET_FACTORY_KEY_LOAD_MODE:
        {
            _V100_FACTORY_KEY_LOAD_MODE fklMode = (_V100_FACTORY_KEY_LOAD_MODE)(*pOptData);

            if (fklMode == FACTORY_KEY_LOAD_COMPLETE)
            {
                status = CRYPTO_MGR->LockFactoryMode() ? CExecStatus::CMD_EXEC_OK : CExecStatus::CMD_ERR_CRYPTO_FAIL;
            }
            else
            if (fklMode == FACTORY_KEY_LOAD_RESTORE)
            {
                status = CExecStatus::CMD_ERR_BAD_PARAMETER;    //Not Supported

            }
            else
            {
                status = CExecStatus::CMD_ERR_BAD_PARAMETER;
            }

            pDataMgr->GetInterfaceConfiguration()->pPDBuffer = CRYPTO_MGR->IsFactoryKeyLoadingLocked();
            break;
        }
        default:
        {
            status = CExecStatus::CMD_ERR_BAD_PARAMETER;
            break;
        }
    }

     return status;
}


CExecStatus    CmdExecutiveHYB02::Execute_Get_Image(_V100_CAPTURE_TYPE capture_type,
                u8* pInCG, uint nInCGSize, u256 pInDS,
                u8** pOutCG, uint* nOutCGSize, u256 pOutDS)
{
    // quick sanity check
    if (!pInCG || (nInCGSize == 0) || !pOutCG )
        return CExecStatus::CMD_ERR_BAD_PARAMETER;


    // The decrypted data packet contains:
    //        ANSOL                    16
    //        Record Header Size         4            (0 if FMR_MODE, non-zero if BIR_MODE)
    //        Record Header            36 or 52     (depending on uszBIRDataType in BIR Header)
    //
    u8* pInputData = nullptr;
    uint nInputDataSize = 0;

    // Get the ANSOL and record header (if present)
    if (!CRYPTO_MGR->Decrypt(pInCG, nInCGSize, &pInputData, nInputDataSize, pInDS))
    {
        return CExecStatus::CMD_ERR_ENCRYPTION_VALIDATION;
    }
    AutoFreePtr Afp_pInputData(pInputData);

    // validate minimum size
    if (nInputDataSize < ANSOL_SIZE_HYB02 + sizeof(uint))
    {
        return CExecStatus::CMD_ERR_BAD_SIZE;
    }

    // extract the ANSOL and header info
    u8 pANSOL[ANSOL_SIZE_HYB02] = {};
    uint nRecordHeaderSize = 0;
    _V100_ENC_BIR_HEADER BIRHeader = {};

    memcpy(pANSOL, pInputData, ANSOL_SIZE_HYB02);
    memcpy(&nRecordHeaderSize, pInputData + ANSOL_SIZE_HYB02, sizeof(uint));
    memcpy(&BIRHeader, pInputData + ANSOL_SIZE_HYB02 + sizeof(uint), nRecordHeaderSize);


    // Get the image options: WSQ or not, w/ BIR header or not
    BioParameters* pBioParam = (&BioParameters::GetInstance());

    _V100_ENC_IMAGE_MODE image_mode = pBioParam->GetImageMode();
    _V100_IMAGE_TYPE image_type = IMAGE_COMPOSITE;
    if (image_mode == IMAGE_WSQ_COMPRESSED)
    {
        image_type = IMAGE_WSQ;
    }

    _V100_ENC_RECORD_MODE record_mode = pBioParam->GetRecordMode();

    // make sure we have a sensible input packet for the mode
    // we are in...
    if ((record_mode == BIR_MODE) && (nRecordHeaderSize == 0))
        return CExecStatus::CMD_ERR_BAD_PARAMETER;


    // Get the image...
    //
    DataMgr* pDataMgr = ISensorInstance::GetInstance()->GetDataMgr();
     _V100_INTERFACE_CONFIGURATION_TYPE* pConfig = pDataMgr->GetInterfaceConfiguration();

    uint nImageSize = pDataMgr->GetCapturedImageSize(capture_type);


    u8* pImage = (u8*)MALLOC(nImageSize);
    if (pImage == nullptr)
    {
        LOGMSG("CE_CMD_ENC_GET_IMAGE:Failed to allocate memory. Returning error %d.",
            CExecStatus::CMD_ERR_OUT_OF_MEMORY);
        return CExecStatus::CMD_ERR_OUT_OF_MEMORY;
    }

    AutoFreePtr afp_pImage(pImage);
    if (false == pDataMgr->GetCapturedImageData(capture_type, pImage, nImageSize))
    {
        // TODO: Don't think this is the correct error. Might be an internal error
        LOGMSG("CE_CMD_ENC_GET_IMAGE:Failed to get captured image data. Returning error %d.",
            CExecStatus::CMD_ERR_DATA_UNAVAILABLE);
        return CExecStatus::CMD_ERR_DATA_UNAVAILABLE;
    }

    if (nImageSize == 0)
    {
        LOGMSG("CE_CMD_ENC_GET_IMAGE:No image available. Returning error %d.",
            CExecStatus::CMD_ERR_DATA_UNAVAILABLE);
        return CExecStatus::CMD_ERR_DATA_UNAVAILABLE;
    }

    uint nBPP = 8;
    uint nX = 0;
    uint nY = 0;
    uint nP = 0;
    pDataMgr->GetScaledDims(nX, nY, nP);
    // Use WSQ compression, if requested
    uchar* pCompressedImg = NULL;
    if (image_type == IMAGE_WSQ)
    {
        int nCompressedImgSz = 0;

        // Get compression ratio
        uint nCompressionRatio = (&BioParameters::GetInstance())->GetCompressionRatio();
        float r_bitrate = (1 / (float)nCompressionRatio) * 8;
        int DPI = pConfig->Composite_DPI;

        // Compress it
        if (Lumi_Encode_WSQ(pImage, r_bitrate, (int)nX , (int)nY, (int)nBPP,
                             DPI, &pCompressedImg, &nCompressedImgSz))
        {
            return CExecStatus::CMD_ERR_BSP;
        }

        pImage = pCompressedImg;
        nImageSize = nCompressedImgSz;
    }

    // this will free the memory allocated above (or do nothing, if WSQ not used) when outofscope
    AutoFreePtr afp_pCompressedImg(pCompressedImg);


    // The return (plaintext) output data packet contains:
    //
    //    In FMR (RAW) mode:
    //        [_V100_ENC_IMAGE_HDR + image data]
    //
    //  In BIR mode:
    //        [_V100_ENC_IMAGE_HDR + _V100_ENC_BIR_HEADER + image size field (4) +
    //            image data + SB size field (4)]
    //

    uchar* pImgRec = NULL;
    uint   nImgRecSize = 0;
    TAdapter TA;
    if (record_mode == BIR_MODE)
    {
        // Ouptut packet has:
        //        BIR Header        36 or 52
        //        Image Size        4 (N)
        //        Image Data        N
        //        SB Size            4

        TA.CreateBIR(&BIRHeader, pImage, nImageSize, &pImgRec, nImgRecSize);
        //pImageRec taken care of TA.
    }
    else
    {
        pImgRec = pImage;
        nImgRecSize = nImageSize;
    }

    uint nOutSize = sizeof(_V100_ENC_IMAGE_HDR) + nImgRecSize;
    u8* pOut = (u8*)MALLOC(nOutSize);
    if (!pOut)
    {
        return CExecStatus::CMD_ERR_CRYPTO_FAIL;
    }
    AutoFreePtr afp_pOut(pOut);

    _V100_ENC_IMAGE_HDR* pImageHeader = (_V100_ENC_IMAGE_HDR*)pOut;
    memcpy(&pImageHeader->nANSOL, pANSOL, ANSOL_SIZE_HYB02);
    pImageHeader->nImageMode = image_mode;
    pImageHeader->nRecordMode = record_mode;
    pImageHeader->nImageWidth = nX;
    pImageHeader->nImageHeight = nY;
    pImageHeader->nImageBPP = nBPP;
    int nSpoofScore = 0;
    pDataMgr->GetCapturedSpoofScore(capture_type, nSpoofScore);
    pImageHeader->nSpoofScore = (u32)nSpoofScore;
    pImageHeader->nImageDataSize = nImgRecSize;

    memcpy(pOut + sizeof(_V100_ENC_IMAGE_HDR), pImgRec, nImgRecSize);
    // Encrypt
    if (false == CRYPTO_MGR->Encrypt(pOut, nOutSize, pOutCG, nOutCGSize, pOutDS))
    {
        return CExecStatus::CMD_ERR_CRYPTO_FAIL;
    }

    return CExecStatus::CMD_EXEC_OK;
}


CExecStatus    CmdExecutiveHYB02::Execute_Get_Spoof_Score(_V100_CAPTURE_TYPE capture_type, u8* pInCG, uint nInCGSize, u256 pInDS,u8** pOutCG, uint* nOutCGSize, u256 pOutDS)
{
    // quick sanity check
    if (!pInCG || (nInCGSize == 0) || !pOutCG)
    {
        return CExecStatus::CMD_ERR_BAD_PARAMETER;
    }


    // The decrypted data packet contains:
    //        ANSOL                    16
    //
    u8* pInputData = nullptr;
    uint nInputDataSize = 0 ;

    // Get the ANSOL
    if (!CRYPTO_MGR->Decrypt(pInCG, nInCGSize, &pInputData, nInputDataSize, pInDS))
    {
        return CExecStatus::CMD_ERR_ENCRYPTION_VALIDATION;
    }
    AutoFreePtr Afp_pInputData(pInputData);

    // validate minimum size
    if (nInputDataSize < ANSOL_SIZE_HYB02 )
    {
        return CExecStatus::CMD_ERR_BAD_SIZE;
    }

    DataMgr* pDataMgr = ISensorInstance::GetInstance()->GetDataMgr();

    int nSpoofScore = 0;
    pDataMgr->GetCapturedSpoofScore(capture_type, nSpoofScore);

    if(nSpoofScore == -1)
    {
        return CExecStatus::CMD_ERR_DATA_UNAVAILABLE;
    }

    _V100_ENC_SPOOF_RESULT        spoofResult;
    // Copy ANSOL back into spoofResult
    memcpy(&spoofResult.nANSOL, pInputData, ANSOL_SIZE_HYB02);
    // Copy Spoof into structure
    memcpy(&spoofResult.nSpoofScore, &nSpoofScore, sizeof(nSpoofScore));


    if (false == CRYPTO_MGR->Encrypt((u8*)&spoofResult, sizeof(_V100_ENC_SPOOF_RESULT), pOutCG, nOutCGSize, pOutDS))
    {
        return CExecStatus::CMD_ERR_CRYPTO_FAIL;
    }

    return CExecStatus::CMD_EXEC_OK;
}


CExecStatus    CmdExecutiveHYB02::Execute_Get_Template(
                _V100_CAPTURE_TYPE capture_type,
                u8* pInCG, uint nInCGSize, u256 pInDS,
                u8** pOutCG, uint* nOutCGSize, u256 pOutDS)
{
    u8* pInputData = nullptr;
    uint nInputDataSize = 0;

    // The decrypted data packet contains:
    //        ANSOL                    16
    //        Record Header Size         4            (0 if FMR_MODE, non-zero if BIR_MODE)
    //        Record Header            36 or 52     (depending on uszBIRDataType in BIR Header)
    //        padding

    if (!CRYPTO_MGR->Decrypt(pInCG, nInCGSize, &pInputData, nInputDataSize, pInDS))
    {
        return CExecStatus::CMD_ERR_ENCRYPTION_VALIDATION;
    }
    AutoFreePtr Afp_pInputData(pInputData);

    // validate minimum size
    if (nInputDataSize < ANSOL_SIZE_HYB02 + sizeof(uint))
    {
        return CExecStatus::CMD_ERR_BAD_SIZE;
    }

    // unpack the input data
    u128 pANSOL = {};
    u32 nRecordHeaderSize = 0;
    _V100_ENC_BIR_HEADER BIRHeader = {};

    memcpy(pANSOL, pInputData, ANSOL_SIZE_HYB02);
    memcpy(&nRecordHeaderSize, pInputData + ANSOL_SIZE_HYB02, sizeof(u32));
    // Note: this can be 36 or 52 bytes because the last field is
    // optional.  This will copy only what bytes are present,
    // the unused fields will be zero this way...
    memcpy(&BIRHeader, pInputData + ANSOL_SIZE_HYB02 + sizeof(u32),
            nRecordHeaderSize);


    // Determine the Record mode (BIR or not)
    BioParameters* pBioParam = (&BioParameters::GetInstance());
    _V100_ENC_RECORD_MODE record_mode = pBioParam->GetRecordMode();

    // make sure we have a sensible input packet for the mode
    // we are in...
    if ((record_mode == BIR_MODE) && (nRecordHeaderSize == 0))
    {
        return CExecStatus::CMD_ERR_BAD_PARAMETER;
    }

    // Determine the template mode
    _V100_TEMPLATE_MODE nTplMode = pBioParam->GetTemplateMode();


    // Grab the template
    u8* pTpl;
    uint nTplSize =0;
    DataMgr* pDataMgr = ISensorInstance::GetInstance()->GetDataMgr();
    pTpl     = pDataMgr->GetCapturedTemplate(capture_type);
    nTplSize = pDataMgr->GetCapturedTemplateSize(capture_type);

    if (pTpl == NULL || nTplSize == 0)
    {
        return CExecStatus::CMD_ERR_DATA_UNAVAILABLE;
    }


    // Convert it, if needed (this is automatic, based on template mode)
    TAdapter TA;
    u8* pTplRec = NULL;
    uint nTplRecSize = 0;

    if (record_mode == BIR_MODE)
    {
        if (false == TA.CreateBIRTemplate(&BIRHeader, pTpl, nTplSize, &pTplRec, nTplRecSize, nTplMode))
        {
            return CExecStatus::CMD_ERR_BSP;
        }
    }
    else // FMR_MODE
    {
        if(false == TA.ConvertFrom378(nTplMode, pTpl, nTplSize, &pTplRec, &nTplRecSize))
        {
            return CExecStatus::CMD_ERR_BSP;
        }
    }


    // The return (plaintext) output data packet contains:
    //
    //    In FMR (RAW) mode:
    //        [_V100_ENC_TEMPLATE_HDR + template data]
    //
    //  In BIR mode:
    //        [_V100_ENC_TEMPLATE_HDR + _V100_ENC_BIR_HEADER +
    //            template size field (4) + template data (N) +
    //            SB size field (4)]
    //
    uint nOutSize = sizeof(_V100_ENC_TEMPLATE_HDR) + nTplRecSize;
    u8* pOut = (u8*)MALLOC(nOutSize);
    if (!pOut)
    {
        return CExecStatus::CMD_ERR_CRYPTO_FAIL;
    }
    AutoFreePtr afp_pOut(pOut);
    memset(pOut, 0,  nOutSize);

    _V100_ENC_TEMPLATE_HDR*    pTemplateHeader = (_V100_ENC_TEMPLATE_HDR*)pOut;
    memcpy(&pTemplateHeader->nANSOL, pANSOL, ANSOL_SIZE_HYB02);
    pTemplateHeader->nTemplateMode = nTplMode;
    pTemplateHeader->nRecordMode = record_mode;
    int nSpoofScore = 0;
    pDataMgr->GetCapturedSpoofScore(capture_type, nSpoofScore);
    pTemplateHeader->nSpoofScore = (u32)nSpoofScore;
    pTemplateHeader->nTemplateDataSize = nTplRecSize;

    // Copy the template data itself.
    //
    //    BIR mode template data contains:
    //        BIR Header        36 or 52
    //        Tpl Size        4 (N)
    //        Tpl Data        N
    //        SB Size            4
    //      (all this is done in the call to CreateBIRTemplate())
    //
    //  FMR (RAW) mode template data contains:
    //        Tpl Data        N
    //
    memcpy(pOut + sizeof(_V100_ENC_TEMPLATE_HDR), pTplRec, nTplRecSize);


    // Encrypt
    if (false == CRYPTO_MGR->Encrypt(pOut, nOutSize, pOutCG, nOutCGSize, pOutDS))
    {
        return CExecStatus::CMD_ERR_CRYPTO_FAIL;
    }

    // TAdapter takes care of pTplRec
    return CExecStatus::CMD_EXEC_OK;
}

// Decrypts InCG
// Validates ANSOL in _V100_ENC_VERIFY_REQ_HDR
// Verifies CAPTURE_VERIFY template against all the Templaes sent int
// Records the Verify matchscores and match results
// Returns encrypted packet with [_V100_ENC_VERIFY_RESULT_HDR +nNumRecords*VerifyMatchScores] and DigSig
CExecStatus    CmdExecutiveHYB02::Execute_Verify(u8* pInCG, uint nInCGSize, u256 pInDS, u8** pOutCG, uint* nOutCGSize, u256 pOutDS)
{
    CExecStatus CE;

    // quick sanity check
    if (!pInCG || (nInCGSize == 0) || !pOutCG )
        return CExecStatus::CMD_ERR_BAD_PARAMETER;


    // The decrypted data packet contains:
    //    Header        _V100_ENC_VERIFY_REQ_HDR
    //  TemplateData*nNumRecords
    //
    u8* pInputData = nullptr;
    uint nInputDataSize = 0 ;
    u256 ANBIO;

    // Get the Header and templates data (if present)
    if (!CRYPTO_MGR->Decrypt(pInCG, nInCGSize, &pInputData, nInputDataSize, pInDS))
    {
        return CExecStatus::CMD_ERR_ENCRYPTION_VALIDATION;
    }
    AutoFreePtr Afp_pInputData(pInputData);

    // validate minimum size
    if (nInputDataSize <  sizeof(_V100_ENC_VERIFY_REQ_HDR))
    {
        return CExecStatus::CMD_ERR_BAD_SIZE;
    }

    //Validate the payload
    CRYPTO_MGR->GetANBIO(&ANBIO);
    _V100_ENC_VERIFY_REQ_HDR* pVerifyHdr = (_V100_ENC_VERIFY_REQ_HDR*)pInputData;
    if(memcmp(ANBIO, pVerifyHdr->nANBIO, ANBIO_SIZE_HYB02) !=0)
    {
        return CExecStatus::CMD_ERR_ENCRYPTION_VALIDATION;
    }


    // Verify logic goes here
    _V100_ENC_VERIFY_RESULT_HDR VerifyResultHdr;
    uint* pVerifyScores = (uint*)MALLOC(sizeof(uint)*pVerifyHdr->nNumRecords);
    AutoFreePtr autofree_pVerifyScores(pVerifyScores);
    CE = Verify(pInputData, &VerifyResultHdr, pVerifyScores);

    if(CE != CExecStatus::CMD_EXEC_OK)
    {
        return CE;
    }


    uint nOutDataSize = sizeof(_V100_ENC_VERIFY_RESULT_HDR)+(sizeof(uint)*VerifyResultHdr.nNumRecords);
    u8* pOutData = (u8*)MALLOC(nOutDataSize);
    if(pOutData == NULL) return CExecStatus::CMD_ERR_OUT_OF_MEMORY;
    AutoFreePtr autofree_pOutData(pOutData);
    memcpy(pOutData, &VerifyResultHdr, sizeof(_V100_ENC_VERIFY_RESULT_HDR));
    memcpy(pOutData+ sizeof(_V100_ENC_VERIFY_RESULT_HDR), pVerifyScores, sizeof(uint)*VerifyResultHdr.nNumRecords);

    // Encrypt
    if (false == CRYPTO_MGR->Encrypt(pOutData, nOutDataSize, pOutCG, nOutCGSize, pOutDS))
    {
        return CExecStatus::CMD_ERR_CRYPTO_FAIL;
    }

    return CExecStatus::CMD_EXEC_OK;

}

// Decrypt and validate InTpl1CG, InTpl2CG buffers
// Validate ANSOLs from decrypted Input CG buffers
// Extract ANSI378 Tpl1 and Tpl2 Buffers
// Perfrom Match and return the result

CExecStatus    CmdExecutiveHYB02::Execute_Match(u8* pInTpl1CG, uint nInTpl1CGSize, u256 pInTpl1DS, u8* pInTpl2CG, uint nInTpl2CGSize, u256 pInTpl2DS, uint& nMatchResult)
{
    nMatchResult = 0;

    if (!pInTpl1CG || (nInTpl1CGSize == 0) || !pInTpl2CG || (nInTpl2CGSize == 0))
    {
        return CExecStatus::CMD_ERR_BAD_PARAMETER;
    }


    u8* pInTplData[2];

    pInTplData[0] = nullptr;
    pInTplData[1] = nullptr;
    uint nInTplDataSize[2];

    //Decrypt and validate input CG buffers
    // The decrypted data packet contains:
    //     [_V100_ENC_TEMPLATE_HDR + templatedata]

    if (!CRYPTO_MGR->Decrypt(pInTpl1CG, nInTpl1CGSize, &pInTplData[0], nInTplDataSize[0], pInTpl1DS))
    {
        return CExecStatus::CMD_ERR_ENCRYPTION_VALIDATION;
    }
    AutoFreePtr Afp_pInTplData0(pInTplData[0]);

    // validate minimum size
    if (nInTplDataSize[0] < sizeof(_V100_ENC_TEMPLATE_HDR))
    {
        return CExecStatus::CMD_ERR_BAD_SIZE;
    }

    if (!CRYPTO_MGR->Decrypt(pInTpl2CG, nInTpl2CGSize, &pInTplData[1], nInTplDataSize[1], pInTpl2DS))
    {
        return CExecStatus::CMD_ERR_ENCRYPTION_VALIDATION;
    }
    AutoFreePtr Afp_pInTplData1(pInTplData[1]);

    // validate minimum size
    if (nInTplDataSize[1] < sizeof(_V100_ENC_TEMPLATE_HDR))
    {
        return CExecStatus::CMD_ERR_BAD_SIZE;
    }

    //Validate ANSOL
    _V100_ENC_TEMPLATE_HDR* pTplHdr[2];
     pTplHdr[0] = (_V100_ENC_TEMPLATE_HDR*)pInTplData[0];
     pTplHdr[1] = (_V100_ENC_TEMPLATE_HDR*)pInTplData[1];

    if(memcmp(pTplHdr[0]->nANSOL, pTplHdr[1]->nANSOL, ANSOL_SIZE_HYB02) !=0)
    {
        return CExecStatus::CMD_ERR_ENCRYPTION_VALIDATION;
    }


    {
        uchar* pTpl[2];
        uint    nTplSize[2];
        uchar* pVerifyTpl[2];
        uint    nVerifyTplSize[2];
        // Extract Template
        TAdapter tA;
        for(uint ii =0; ii < 2; ii++)
        {
            nTplSize[ii] = pTplHdr[ii]->nTemplateDataSize;
            _V100_ENC_RECORD_MODE nTplRecMode = (_V100_ENC_RECORD_MODE)pTplHdr[ii]->nRecordMode;
            _V100_TEMPLATE_MODE nTplMode  = (_V100_TEMPLATE_MODE)pTplHdr[ii]->nTemplateMode;

            if(nTplSize[ii] > nInTplDataSize[ii]-sizeof(_V100_ENC_TEMPLATE_HDR))
            {
                return CExecStatus::CMD_ERR_BAD_PARAMETER;
            }

            pTpl[ii] = pInTplData[ii] + sizeof(_V100_ENC_TEMPLATE_HDR);

            if(nTplRecMode == RAW_MODE)
            {
                if (false == tA.AdaptTo378(nTplMode, pTpl[ii], nTplSize[ii], &pVerifyTpl[ii], &nVerifyTplSize[ii]))
                {
                    return CExecStatus::CMD_ERR_BAD_PARAMETER;
                }
            }
            else
            {
                if (false == tA.Get378TemplateFromBIR(pTpl[ii], &pVerifyTpl[ii], nVerifyTplSize[ii], nTplMode))
                {
                    return CExecStatus::CMD_ERR_BAD_PARAMETER;
                }

            }
        }

        uint nScore =0;
        float fBioMetric = -1.0f;

        IEnrollSvc* pES = ISensorInstance::GetInstance()->GetCfgMgr()->GetEnrollSvc();

        if(false == pES->Match(pVerifyTpl[0],nVerifyTplSize[0], pVerifyTpl[1], nVerifyTplSize[1], nScore))
        {
            return CExecStatus::CMD_ERR_BSP;
        }

        nMatchResult = (pES->GetMatchResult(pVerifyTpl[0],nVerifyTplSize[0], pVerifyTpl[1], nVerifyTplSize[1], nScore, &fBioMetric, false))?1:0;
        // pVerifyTpl mem free takencare of TAdapter
    }

    return CExecStatus::CMD_EXEC_OK;

}


CExecStatus    CmdExecutiveHYB02::Execute_Set_Parameters(u8* pInCG, uint nInCGSize, u256 pInDS)
{
    // Get Crypto
    BioParameters* pBioParams = (&BioParameters::GetInstance());
    u256 pANBIO;

    u8* pIn = nullptr;
    uint nInSize =0;

    // quick sanity check
    if (!pInCG || (nInCGSize == 0))
        return CExecStatus::CMD_ERR_BAD_PARAMETER;


    // The decrypted data packet contains:
    //    ANBIO                    8
    //    _V100_ENC_PARAMETERS

    //Decrypt
    if(false == CRYPTO_MGR->Decrypt(pInCG, nInCGSize, &pIn, nInSize, pInDS))
    {
        return CExecStatus::CMD_ERR_ENCRYPTION_VALIDATION;
    }
    AutoFreePtr Afp_pIn(pIn);

    if(nInSize < ANBIO_SIZE_HYB02 + sizeof(_V100_ENC_PARAMETERS))
    {
        return CExecStatus::CMD_ERR_BAD_SIZE;
    }

    //Validate ANBIO
    if(false == CRYPTO_MGR->GetANBIO(&pANBIO))
    {
        return CExecStatus::CMD_ERR_CRYPTO_FAIL;
    }

    if(memcmp(pANBIO, pIn, ANBIO_SIZE_HYB02) != 0)
    {
        return CExecStatus::CMD_ERR_ENCRYPTION_VALIDATION;
    }

    _V100_ENC_PARAMETERS BioParams;
    memcpy(&BioParams, pIn+ANBIO_SIZE_HYB02, sizeof(_V100_ENC_PARAMETERS));
    if(false == pBioParams->SetBioParameters(&BioParams))
    {
        return CExecStatus::CMD_ERR_BAD_PARAMETER;
    }

    return CExecStatus::CMD_EXEC_OK;
}


CExecStatus    CmdExecutiveHYB02::Execute_Get_Parameters(u8* pInCG, uint nInCGSize, u256 pInDS, u8** pOutCG, uint* nOutCGSize, u256 pOutDS)
{
    // Get Crypto
    BioParameters* pBioParams = (&BioParameters::GetInstance());

    CExecStatus CERet = CExecStatus::CMD_EXEC_OK;
    u8* pIn = nullptr;
    uint nInSize =0;

    // quick sanity check
    if (!pInCG || (nInCGSize == 0) || !pOutCG)
        return CExecStatus::CMD_ERR_BAD_PARAMETER;


    // The decrypted data packet contains:
    //    ANSOL        16

    //Decrypt
    if(false == CRYPTO_MGR->Decrypt(pInCG, nInCGSize, &pIn, nInSize, pInDS))
    {
        return CExecStatus::CMD_ERR_ENCRYPTION_VALIDATION;
    }
    AutoFreePtr Afp_pIn(pIn);

    if(nInSize < ANSOL_SIZE_HYB02)
    {
        return CExecStatus::CMD_ERR_BAD_SIZE;
    }

    //_V100_ENC_PARAMETERS BioParams;
    //memcpy(&BioParams, pBioParams->GetBioParameters(), sizeof(_V100_ENC_PARAMETERS));

    uint nOutSize = ANSOL_SIZE_HYB02 + sizeof(_V100_ENC_PARAMETERS);
    AutoHeapBuffer Auto_pOut(nOutSize);
    u8* pOut = Auto_pOut.u8Ptr();
    if(pOut == NULL)
    {
        return CExecStatus::CMD_ERR_OUT_OF_MEMORY;
    }

    memcpy(pOut, pIn, ANSOL_SIZE_HYB02);
    memcpy(pOut+ANSOL_SIZE_HYB02, pBioParams->GetBioParameters(), sizeof(_V100_ENC_PARAMETERS));

    if(false == CRYPTO_MGR->Encrypt(pOut, nOutSize, pOutCG, nOutCGSize, pOutDS))
    {
        return CExecStatus::CMD_ERR_CRYPTO_FAIL;
    }

    return CERet;
}


CExecStatus    CmdExecutiveHYB02::Execute_Get_Rnd_Number(u256* pRN, short isEncryptedWithSk)
{
    //isEncryptedWithSk unused

    CExecStatus bRet = CExecStatus::CMD_ERR_CRYPTO_FAIL;


    memset(pRN, 0, sizeof(u256));
    if (false == CRYPTO_MGR->GetRandomNumber(pRN))
    {
        return bRet;
    }

    return CExecStatus::CMD_EXEC_OK;
}


CExecStatus CmdExecutiveHYB02::Execute_Generate_Session_Key()
{
//<TODO>: Remove this, not supported
    //return (CRYPTO_MGR->GenerateSessionKey()) ? CExecStatus::CMD_EXEC_OK : CExecStatus::CMD_ERR_CRYPTO_FAIL;
    return CExecStatus::CMD_ERR_NOT_SUPPORTED;
}


CExecStatus CmdExecutiveHYB02::Execute_Get_Key(_V100_ENC_KEY_TYPE nKeyType, u2048 pKey, u32& nKeySize, u8* pKCV, u16& nVerNum, _V100_ENC_KEY_MODE& nKeyMode)
{
    return CRYPTO_MGR->GetKey(nKeyType, nVerNum, nKeyMode, pKCV, pKey, nKeySize) ? CExecStatus::CMD_EXEC_OK : CExecStatus::CMD_ERR_CRYPTO_FAIL;
}


CExecStatus    CmdExecutiveHYB02::Execute_Get_Key_Version(_V100_ENC_KEY_TYPE nKeyType, u16& nKeyVer, _V100_ENC_KEY_MODE& nKeyMode, u8* pKCV)
{
    return CRYPTO_MGR->GetKeyInfo(nKeyType, nKeyVer, nKeyMode, pKCV) ? CExecStatus::CMD_EXEC_OK : CExecStatus::CMD_ERR_CRYPTO_FAIL;
}


CExecStatus CmdExecutiveHYB02::Execute_Set_Key(_V100_ENC_KEY_TYPE nKeyType, u8* pCGKey, uint nCGKeySize, u8** pOutCG, uint& nOutCGSize)
{
    int nCMRet = CM_ERROR;

    // What state are we in?
    if (!CRYPTO_MGR->IsFactoryKeyLoadingLocked())
    {

        nCMRet = CRYPTO_MGR->InitiateFactoryKeyLoad(nKeyType, pCGKey, nCGKeySize, pOutCG, nOutCGSize);
    }
    else
    {
        nCMRet = CRYPTO_MGR->InitiateRemoteKeyLoad(nKeyType, pCGKey, nCGKeySize, pOutCG, nOutCGSize);
    }


    return MarshalCMErrToCEErr(nCMRet);
}


CExecStatus CmdExecutiveHYB02::Execute_Set_Active_Key(_V100_ENC_KEY_TYPE nKeyType)
{
    int nCMRet = CRYPTO_MGR->SetActiveKey(nKeyType, true);

    return MarshalCMErrToCEErr(nCMRet);
}


CExecStatus CmdExecutiveHYB02::Execute_Get_Capture_Stats(_V100_CAPTURE_TYPE capture_type, _V100_CAPTURE_STATS* capture_stats)
{
    _V100_CAPTURE_STATS const* pCaptureStats = CaptureStats::Get(capture_type);

    if (pCaptureStats == NULL)
    {
        return CExecStatus::CMD_ERR_DATA_UNAVAILABLE;
    }

    *capture_stats = *pCaptureStats;

    return CExecStatus::CMD_EXEC_OK;
}


CExecStatus MarshalCMErrToCEErr(int nCMErr)
{
    CExecStatus    nCEStat;
    switch(nCMErr)
    {
        case CM_OK:
        {
            nCEStat = CExecStatus::CMD_EXEC_OK;
        }break;
        case CM_ERROR_NOT_SUPPORTED:
        {
            nCEStat = CExecStatus::CMD_ERR_NOT_SUPPORTED;
        }break;
        case CM_ERROR:
        default:
        {
            nCEStat =  CExecStatus::CMD_ERR_CRYPTO_FAIL;
        }break;
    }

    return nCEStat;

}


void CmdExecutiveHYB02::Release_Buffer(u8** pBuffer)
{
    if (pBuffer && *pBuffer)
    {
        FREE(*pBuffer);
        *pBuffer = nullptr;
    }
    //CRYPTO_MGR->ReleaseSEBuff(pBuffer);
}

CExecStatus CmdExecutiveHYB02::Verify(u8* pInData, _V100_ENC_VERIFY_RESULT_HDR* pVerifyResultHdr, uint* pVerifyScores)
{
    _V100_ENC_VERIFY_REQ_HDR* pVerifyHdr = (_V100_ENC_VERIFY_REQ_HDR*)pInData;
    DataMgr* pDataMgr = ISensorInstance::GetInstance()->GetDataMgr();
    BioParameters* pBioParams = (&BioParameters::GetInstance());

    if (!pInData || !pVerifyResultHdr || !pVerifyScores)
    {
        return CExecStatus::CMD_ERR_BAD_PARAMETER;
    }
    if (pVerifyHdr->nNumRecords > MAX_VERIFY_RECORDS || pVerifyHdr->nNumRecords < 1)
    {
        return CExecStatus::CMD_ERR_BAD_PARAMETER;
    }

    //Get Record mode and template mode info from payload
    uchar* pCapTpl = pDataMgr->GetCapturedTemplate(CAPTURE_VERIFY);
    uint nCapTplSize = pDataMgr->GetCapturedTemplateSize(CAPTURE_VERIFY);
    _V100_ENC_RECORD_MODE nTplRecMode = (_V100_ENC_RECORD_MODE)pVerifyHdr->nRecordMode;
    _V100_TEMPLATE_MODE nTplMode = (_V100_TEMPLATE_MODE)pVerifyHdr->nTemplateMode;


    if (pCapTpl == NULL || nCapTplSize == 0)
    {
        return CExecStatus::CMD_ERR_DATA_UNAVAILABLE;
    }

    memset(pVerifyResultHdr, 0, sizeof(_V100_ENC_VERIFY_RESULT_HDR));
    memcpy(pVerifyResultHdr->nANSOL, &pVerifyHdr->nANSOL, ANSOL_SIZE);

    uint* pScores = pVerifyScores;
    u32 nBitMask = 0;

    uchar* pTpl = pInData + sizeof(_V100_ENC_VERIFY_REQ_HDR);
    uint nTplSize = 0;

    //Match and keep track of results
    nBitMask = 1;
    for (uint ii = 0; ii < pVerifyHdr->nNumRecords; ii++)
    {
        TAdapter tA;
        uchar* pVerifyTpl;
        uint nVerifyTplSize = 0;
        // Copy here instead of directly accessing the value from buffer. The size bytes in buffer can be at
        // non aligned address and direct accessing can cause issues.
        memcpy(&nTplSize, pTpl, sizeof(uint));

        if (nTplRecMode == RAW_MODE)
        {
            pTpl += sizeof(uint);
            if (false == tA.AdaptTo378(nTplMode, pTpl, nTplSize, &pVerifyTpl, &nVerifyTplSize))
            {
                return CExecStatus::CMD_ERR_BAD_PARAMETER;
            }
        }
        else
        {
            if (false == tA.Get378TemplateFromBIR(pTpl, &pVerifyTpl, nVerifyTplSize, nTplMode))
            {
                return CExecStatus::CMD_ERR_BAD_PARAMETER;
            }
            // Note that the template size, which is the BIR size field from the
            // BIR header, does not include the four bytes of the size field itself...
            pTpl += sizeof(uint);
        }
        pTpl += nTplSize;

        u32 nScore = 0;
        IEnrollSvc* pES = ISensorInstance::GetInstance()->GetCfgMgr()->GetEnrollSvc();

        if (false == pES->Match(pCapTpl, nCapTplSize, pVerifyTpl, nVerifyTplSize, nScore))
        {
            return CExecStatus::CMD_ERR_BSP;
        }

        pVerifyResultHdr->nNumRecords++;

        pScores[ii] = nScore;
        float fBioMetric = -1.0f;
        if (pES->GetMatchResult(pCapTpl, nCapTplSize, pVerifyTpl, nVerifyTplSize, nScore, &fBioMetric, false))
        {
            pVerifyResultHdr->nMatchResults |= nBitMask;
        }

        nBitMask <<= 1;
        //pVerifyTpl mem free taken care of TAdapter
    }


    return CExecStatus::CMD_EXEC_OK;
}