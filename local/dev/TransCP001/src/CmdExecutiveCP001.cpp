#include "AutoHeapBuffer.h"
#include "CmdExecutiveCP001.h"
#include "CmdExecutiveCommon.h"
#include "DataMgr.h"
#include "CryptoMgrCP001.h"
#include "IBSP.h"
#include "IMsgPoster.h"
#include "AppStateObserver.h"
#include "BioParameters.h"
#include "TAdapter.h"
#include "wsqWrapper.h"

// Marshal cryptoMgrError to CmdExecutive Error codes
static CExecStatus MarshalCMErrToCEErr(int nCMErr);

CExecStatus CmdExecutiveCP001::Execute_Get_Image(_V100_IMAGE_TYPE type, uchar** pOutCG, uint* nOutCGSize)
{
    uchar* pImage = NULL;

    uint nX, nY, nP;

    DataMgr* pDM = ISensorInstance::GetInstance()->GetDataMgr();
    uint  nImageSize = 0;

    if (pDM->HasCaptureCleared() == true)
    {
        return CExecStatus::CMD_ERR_DATA_UNAVAILABLE;
    }

    switch (type)
    {
    case IMAGE_COMPOSITE:
    {
        pDM->GetScaledDims(nX, nY, nP);
        pImage = (uchar*)pDM->GetImage(type);
        // Encrypt
        nImageSize = nX * nY;
        break;
    }
    case IMAGE_WSQ:
    {
        pDM->GetRawDims(nX, nY, nP);
        nImageSize = nX * nY;
        // Compress it now
        pImage = (uchar*)pDM->GetImage(type);// Get buffer to store compressed image
        _V100_INTERFACE_CONFIGURATION_TYPE* pICT = pDM->GetInterfaceConfiguration();
        uchar* pCompositeImg = (uchar*)pDM->GetImage(IMAGE_COMPOSITE);// Get composite image to compress
        uchar* pCompressedImg = NULL;
        int nCompressedImgSz = 0;
        uint nCompressionRatio = BioParameters::GetInstance().GetCompressionRatio();
        float r_bitrate = (1 / (float)nCompressionRatio) * 8;
        if (Lumi_Encode_WSQ(pCompositeImg, r_bitrate, (int)pICT->Composite_Image_Size_X, (int)pICT->Composite_Image_Size_Y, 8, (int)pICT->Composite_DPI, &pCompressedImg, &nCompressedImgSz))
        {
            pImage = NULL;
        }
        else
        {
            // Copy it if successfull
            memcpy(pImage, pCompressedImg, nCompressedImgSz);
            Lumi_Release(pCompressedImg); pCompressedImg = NULL;
            nImageSize = nCompressedImgSz;
        }
        break;
    }
    default:
    {
        return CExecStatus::CMD_ERR_NOT_SUPPORTED;
    }
    }
    if (pImage != NULL)
    {
        // Encrypt
        CryptoMgrCP001* pCM = CryptoMgrCP001::GetInstance();
        if (false == pCM->Encrypt(pImage, nImageSize, pOutCG, nOutCGSize, NULL))
        {
            return CExecStatus::CMD_ERR_ENCRYPTION_FAIL;
        }
    }
    else
    {
        return CExecStatus::CMD_ERR_BUSY;
    }

    return CExecStatus::CMD_EXEC_OK;
}

CExecStatus CmdExecutiveCP001::Execute_Get_Template(uchar** pRecord, uint* nRecordSize)
{
    // Grab the template
    u8* pTemplate;
    uint nTemplateSize = 0;
    DataMgr* pDM = ISensorInstance::GetInstance()->GetDataMgr();
    pTemplate = pDM->GetProbeTemplate();
    nTemplateSize = pDM->GetProbeTemplateSize();

    if (pDM->HasCaptureCleared() == true)
    {
        return CExecStatus::CMD_ERR_DATA_UNAVAILABLE;
    }

    if (pTemplate == NULL || nTemplateSize == 0)
    {
        return CExecStatus::CMD_ERR_DATA_UNAVAILABLE;
    }

    _V100_ENC_RECORD_MODE record_mode = BioParameters::GetInstance().GetRecordMode();
    _V100_TEMPLATE_MODE template_mode = BioParameters::GetInstance().GetTemplateMode();

    // Convert to desired format
    TAdapter TA;
    u8* pTemplateRecord = NULL;
    uint nTemplateRecordSize = 0;
    if (record_mode == RAW_MODE)
    {
        if (false == TA.ConvertFrom378(template_mode, pTemplate, nTemplateSize, &pTemplateRecord, &nTemplateRecordSize))
        {
            return CExecStatus::CMD_ERR_BAD_DATA;
        }
    }
    else    // BIR_MODE
    {
        _V100_ENC_BIR_HEADER_PROPRIETARY BIRHeader = {};
        if (false == TA.CreateBIRTemplateProprietary(&BIRHeader, pTemplate, nTemplateSize, &pTemplateRecord, nTemplateRecordSize, template_mode))
        {
            return CExecStatus::CMD_ERR_BAD_DATA;
        }

    }

    // Encrypt
    uchar* pCG = NULL;
    uint nCGSize = 0;
    u256 pDS = {};
    CryptoMgrCP001* pCM = CryptoMgrCP001::GetInstance();
    if (false == pCM->Encrypt(pTemplateRecord, nTemplateRecordSize, &pCG, &nCGSize, pDS))
    {
        return CExecStatus::CMD_ERR_ENCRYPTION_FAIL;
    }

    *nRecordSize = nCGSize + sizeof(pDS);
    *pRecord = (uchar*)MALLOC(*nRecordSize);
    if (*pRecord == NULL)
    {
        Release_Buffer(&pCG);
        return CExecStatus::CMD_ERR_ENCRYPTION_FAIL;
    }
    memcpy(*pRecord, pCG, nCGSize);
    memcpy(*pRecord + nCGSize, pDS, sizeof(pDS));
    Release_Buffer(&pCG);

    //Note: TAdapter takes care of  releasing pTemplateRecord buffer
    return CExecStatus::CMD_EXEC_OK;
}

CExecStatus CmdExecutiveCP001::Execute_Set_Option(_V100_OPTION_TYPE OptionType, uchar* pOptData, uint nOptDataSize) {

    CryptoMgrCP001* pCM = CryptoMgrCP001::GetInstance();
    CExecStatus status = CExecStatus::CMD_EXEC_OK;

    switch (OptionType) {

        case OPTION_SET_FACTORY_KEY_LOAD_MODE: {
            _V100_FACTORY_KEY_LOAD_MODE fklMode = (_V100_FACTORY_KEY_LOAD_MODE)(*pOptData);
            if (fklMode == FACTORY_KEY_LOAD_COMPLETE) {
                status = pCM->LockFactoryMode() ? CExecStatus::CMD_EXEC_OK : CExecStatus::CMD_ERR_CRYPTO_FAIL;
            } else
            if (fklMode == FACTORY_KEY_LOAD_RESTORE) {
                status = CExecStatus::CMD_ERR_BAD_PARAMETER;
            } else {
                status = CExecStatus::CMD_ERR_BAD_PARAMETER;
            }

            ISensorInstance::GetInstance()->GetDataMgr()->GetInterfaceConfiguration()->pPDBuffer = pCM->IsFactoryKeyLoadingLocked();
            break;
        }

        case OPTION_SET_WSQ_COMPRESSION_RATIO: {
            uint nCompressionRatio = *(int*)pOptData;
            if (false == BioParameters::GetInstance().SetCompressionRatio(nCompressionRatio)) {
                status = CExecStatus::CMD_ERR_BAD_PARAMETER;
            }
            break;
        }

        case OPTION_SET_LATENT_DETECTION: {
            uint CalcLatent = *(uint*)pOptData;
            if (false == BioParameters::GetInstance().SetLatentMode((_V100_LATENT_DETECTION_MODE)CalcLatent)) {
                status = CExecStatus::CMD_ERR_BAD_PARAMETER;
            }
            break;
        }

        case OPTION_SET_TEMPLATE_MODE: {
            uchar TemplateMode = 0;
            memcpy(&TemplateMode, pOptData, 1);
            _V100_TEMPLATE_MODE mode = (_V100_TEMPLATE_MODE)(TemplateMode);
            if (false == BioParameters::GetInstance().SetTemplateMode(mode)) {
                status = CExecStatus::CMD_ERR_BAD_PARAMETER;
            }
            break;
        }

        case OPTION_SET_TEMPLATE_RECORD_MODE: {
            uchar nRecMode;
            memcpy(&nRecMode, pOptData, 1);
            if (false == BioParameters::GetInstance().SetRecordMode((_V100_ENC_RECORD_MODE)nRecMode)) {
                status = CExecStatus::CMD_ERR_BAD_PARAMETER;
            }
            break;
        }

        default: {
            status = CExecStatus::CMD_ERR_BAD_PARAMETER;
        }

    }

    return status;
}

CExecStatus CmdExecutiveCP001::Execute_Enc_Clear() {
    return CmdExecutiveCommon::GetInstance().Execute_Enc_Clear();
}

CExecStatus CmdExecutiveCP001::Execute_Enc_Get_Spoof_Score(u128* pANSOL, u8** pOutCG, uint* nOutCGSize) {

    DataMgr* pDM = ISensorInstance::GetInstance()->GetDataMgr();

    if (pDM->HasCaptureCleared() == true) {
        return CExecStatus::CMD_ERR_DATA_UNAVAILABLE;
    }

    // quick sanity check
    if ((pOutCG == NULL) || (nOutCGSize == NULL)) {
        return CExecStatus::CMD_ERR_BAD_PARAMETER;
    }

    int nSpoofScore = pDM->GetLastSpoofScore();
    if (nSpoofScore == -1) {
        return CExecStatus::CMD_ERR_DATA_UNAVAILABLE;
    }

    typedef struct {
        u128    ANSOL;            // Random number sent in from host, as an argument to V100_Enc_Get_Spoof_Score
        u128    SpoofScore;        //  4 bytes of spoof score, 12 bytes of padding,
    }   _V100_SPOOF_RESULT;        //

    _V100_SPOOF_RESULT SpoofResult = {};
    memcpy(SpoofResult.ANSOL, pANSOL, sizeof(u128));
    memcpy(SpoofResult.SpoofScore, &nSpoofScore, sizeof(int));

    CryptoMgrCP001* pCM = CryptoMgrCP001::GetInstance();

    if (false == pCM->Encrypt((u8*)&SpoofResult, sizeof(_V100_SPOOF_RESULT), pOutCG, nOutCGSize, NULL)) {
        return CExecStatus::CMD_ERR_ENCRYPTION_FAIL;
    }

    return CExecStatus::CMD_EXEC_OK;
}

CExecStatus    CmdExecutiveCP001::Execute_Enc_Set_Parameters( u8* iSPLCG, int nSPLCGSize, u8* ANSOL, int ANSOLSize, _V100_CROP_LEVEL nCropLevel, int nTimeout) {

    if (nSPLCGSize > 0) {

        u8* pSPLOut = nullptr;
        uint nSPLOutSize = 0;

        if (!CryptoMgrCP001::GetInstance()->DecryptAndValidateSPLCG(iSPLCG, &pSPLOut, nSPLCGSize, &nSPLOutSize)) {
            Release_Buffer(&pSPLOut);
            return CExecStatus::CMD_ERR_DECRYPTION_FAIL;
        }

        if (nSPLOutSize < (ANBIO_SIZE_CP001 + 4)) {
            Release_Buffer(&pSPLOut);
            return CExecStatus::CMD_ERR_DECRYPTION_FAIL;
        }

        uint nSPL = 0;
        memcpy(&nSPL, pSPLOut + ANBIO_SIZE_CP001, 4);

        if ((_V100_ENC_SPOOF_PROTECTION_LEVEL)nSPL != SPOOF_PROTECT_DONT_CHANGE) {
            if (!BioParameters::GetInstance().SetSpoofProtLevel((_V100_ENC_SPOOF_PROTECTION_LEVEL)nSPL)) {
                Release_Buffer(&pSPLOut);
                return CExecStatus::CMD_ERR_DECRYPTION_FAIL;
            }
        }

        Release_Buffer(&pSPLOut);
    }

    if (nTimeout != -1) {
        BioParameters::GetInstance().SetCaptureTimeOut(nTimeout);
    }

    // NOTE: V42x supports crop level 272x400, this code may need to be updated in future
    if ((nCropLevel != CROP_LAST) && (nCropLevel != CROP_NONE)) { // && (nCropLevel != CROP_272x400))// We only support these modes. CROP_LAST for no change
        return CExecStatus::CMD_ERR_BAD_PARAMETER;
    }

    if (nCropLevel != CROP_LAST) {
        if (!BioParameters::GetInstance().SetCropLevel(_V100_CROP_LEVEL(nCropLevel))) {
            return CExecStatus::CMD_ERR_BAD_PARAMETER;
        }
    }

    return CExecStatus::CMD_EXEC_OK;
}

CExecStatus    CmdExecutiveCP001::Execute_Enc_Get_Rnd_Number(u256* pRN, short isEncryptedWithSk) {

    //isEncryptedWithSk unused
    CryptoMgrCP001* pCM = CryptoMgrCP001::GetInstance();

    CExecStatus nRet = CExecStatus::CMD_ERR_CRYPTO_FAIL;

    memset(pRN, 0, sizeof(u256));
    if (false == pCM->GetRandomNumber(pRN)) {
        return nRet;
    }

    return CExecStatus::CMD_EXEC_OK;
}

CExecStatus CmdExecutiveCP001::Execute_Enc_Generate_SessionKey() {

    CryptoMgrCP001* pCM = CryptoMgrCP001::GetInstance();
    return (pCM->GenerateSessionKey()) ? CExecStatus::CMD_EXEC_OK : CExecStatus::CMD_ERR_CRYPTO_FAIL;
}

CExecStatus CmdExecutiveCP001::Execute_Enc_Get_Key(_V100_ENC_KEY_TYPE nKeyType, u2048 pKey, u32& nKeySize, u8* pKCV, u16& nVerNum, _V100_ENC_KEY_MODE& nKeyMode) {

    CryptoMgrCP001* pCM = CryptoMgrCP001::GetInstance();
    return pCM->GetKey(nKeyType, nVerNum, nKeyMode, pKCV, pKey, nKeySize) ? CExecStatus::CMD_EXEC_OK : CExecStatus::CMD_ERR_CRYPTO_FAIL;
}

CExecStatus    CmdExecutiveCP001::Execute_Enc_Get_KeyVersion(_V100_ENC_KEY_TYPE nKeyType, u16& nKeyVer, _V100_ENC_KEY_MODE& nKeyMode, u8* pKCV) {

    CryptoMgrCP001* pCM = CryptoMgrCP001::GetInstance();
    return pCM->GetKeyInfo(nKeyType, nKeyVer, nKeyMode, pKCV) ? CExecStatus::CMD_EXEC_OK : CExecStatus::CMD_ERR_CRYPTO_FAIL;
}

CExecStatus CmdExecutiveCP001::Execute_Enc_Set_Key(_V100_ENC_KEY_TYPE nKeyType, u8* pCGKey, uint nCGKeySize) {

    int nCMRet = CM_ERROR;
    CryptoMgrCP001* pCM = CryptoMgrCP001::GetInstance();

    // What state are we in?
    if (!pCM->IsFactoryKeyLoadingLocked())     {
        nCMRet = pCM->InitiateFactoryKeyLoad(nKeyType, pCGKey, nCGKeySize);
    } else {
        nCMRet = pCM->InitiateRemoteKeyLoad(nKeyType, pCGKey, nCGKeySize);
    }

    return MarshalCMErrToCEErr(nCMRet);
}

CExecStatus CmdExecutiveCP001::Execute_Enc_Set_ActiveKey(_V100_ENC_KEY_TYPE nKeyType) {

    DataMgr* pDMgr = ISensorInstance::GetInstance()->GetDataMgr();
    CryptoMgrCP001* pCM = CryptoMgrCP001::GetInstance();

    //GetTC flag for DUKPT Keys
    bool bTCInc = false;
    if ((nKeyType == KT_EXTKEY_TDES0)) {
        pDMgr->GetTCIncFlag(0, &bTCInc);
    } else
    if (nKeyType == KT_EXTKEY_TDES1) {
        pDMgr->GetTCIncFlag(1, &bTCInc);
    }

    int nCMRet = pCM->SetActiveKey(nKeyType, bTCInc);

    if (CM_OK == nCMRet) {
        // We only increment TC for DUKPT keys once per new capture data.
        // turn TCInc off now.
        if ((nKeyType == KT_EXTKEY_TDES0)) {
            pDMgr->SetTCIncFlag(0, false);
        } else
        if (nKeyType == KT_EXTKEY_TDES1) {
            pDMgr->SetTCIncFlag(1, false);
        }
    }

    return MarshalCMErrToCEErr(nCMRet);
}

void CmdExecutiveCP001::Release_Buffer(u8** pBuffer) {

    if (pBuffer && *pBuffer) {
        FREE(*pBuffer);
        *pBuffer = nullptr;
    }

    //CRYPTO_MGR.ReleaseSEBuff(pBuffer);
}

CExecStatus MarshalCMErrToCEErr ( int nCMErr ) {

    CExecStatus    nCEStat;

    switch (nCMErr) {
        case CM_OK:
            nCEStat = CExecStatus::CMD_EXEC_OK;
            break;
        case CM_ERROR_NOT_SUPPORTED:
            nCEStat = CExecStatus::CMD_ERR_NOT_SUPPORTED;
            break;
        case CM_ERROR:
        default:
            nCEStat = CExecStatus::CMD_ERR_CRYPTO_FAIL;
            break;
    }

    return nCEStat;
}
