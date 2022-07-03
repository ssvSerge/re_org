#include "EnrollSvc.h"
#include "IProcess.h"
#include "Configuration.h"
#include "BioParameters.h"
#include "far_lut.h"
#include "CriticalErrorLog.h"
#include "SEngineProcINT16.h"



/****************************************************************
*
* Mode logic for this class
*
* 1) When a first enrollment flag gets set, clear all the buffers associated with all enrolls.
* 2) Only the "AddNewEnrollment" function may advance the m_CurrentEnrollState.
* 3) The ClearEnroll() function may reset the m_CurrentEnrollState as well
*
*
*
*
*
******************************************************************/

IEnrollSvc* EnrollSvc::GetInstance()
{
    static EnrollSvc instance;
    return &instance;
}

EnrollSvc::EnrollSvc()
{
    m_CurrentEnrollState = Enroll_None;
    m_CaptureType = CAPTURE_IMAGE;
}

EnrollStatus EnrollSvc::SetCaptureMode(_V100_CAPTURE_TYPE ct)
{
    switch(ct)
    {
        case CAPTURE_ENROLL_1:
        {
            ClearEnroll();
        } break;
        case CAPTURE_ENROLL_2:
        {
            // Check to make certain Enroll 1 is complete
            if (m_CurrentEnrollState != Enroll_Capture_1_Complete)
            {
                return ENROLL_ERROR_UNALIGNED;
            }
        } break;
        case CAPTURE_ENROLL_3:
        {
            // Check to make certain Enroll 2 is complete
            if (m_CurrentEnrollState != Enroll_Capture_2_Complete)
            {
                return ENROLL_ERROR_UNALIGNED;
            }
        } break;
    }
    m_CaptureType = ct;

    return ENROLL_OK;
}

bool EnrollSvc::ClearEnroll()
{
    DataMgr* pDM = ISensorInstance::GetInstance()->GetDataMgr();
    pDM->ClearEnrollMemory();
    // Reset Enroll State.
    m_CurrentEnrollState = Enroll_None;
    // Match Scores
    m_nMatchMatrix[0] = 0;
    m_nMatchMatrix[1] = 0;
    m_nMatchMatrix[2] = 0;

    return true;
}


EnrollStatus EnrollSvc::GetBestEnrollment(_V100_CAPTURE_TYPE& nBestEnrollment)
{
    // Find Template NOT involved in the lowest match
    // If 0 (1 vs 2) is lowest match, answer = 3
    // If 1 (2 vs 3) is lowest match, answer = 1
    // If 2 (1 vs 3) is lowest match, answer = 2
    nBestEnrollment = CAPTURE_ENROLL_2;

    if( m_nMatchMatrix[0] <= m_nMatchMatrix[1] &&  m_nMatchMatrix[0] <= m_nMatchMatrix[2])
    {
         nBestEnrollment = CAPTURE_ENROLL_3;
    } else
    if( m_nMatchMatrix[1] <= m_nMatchMatrix[2] &&  m_nMatchMatrix[1] <= m_nMatchMatrix[0])
    {
        nBestEnrollment = CAPTURE_ENROLL_1;
    }
    return ENROLL_OK;
}

/*
**
** As we know, modally, where we are, we add this enrollment.   If its enrollment 2/3, we do an enrollment qualification as well.
** Even if the enrollment fails, DONT delete it out of the system, as it would give the end-use a chance to retrieve the image
** and template for data collection purposes.   Just don't advance the Enrollment Mode in case of failure.
** NOTE: Expect this call to fail if Extractor is OFF and capture types are other than CAPTURE_IMAGE, CAPTURE_VERIFY
*/

EnrollStatus EnrollSvc::AddNewEnrollment(u8* pImageData, u32 nImageDataSize, u8* pTemplateData, u32 nTemplateDataSize, int nSpoofScore)
{
    DataMgr* pDM = ISensorInstance::GetInstance()->GetDataMgr();

    // Check for valid template data only if Extractor is On. If Off only Capture types CAPTURE_IMAGE, CAPTURE_VERIFY succeed which doesn't validate the templates
    if ((&BioParameters::GetInstance())->DoExtract())
    {

        if (pTemplateData == NULL || nTemplateDataSize == 0)
        {
            LOGMSG("AddNewEnrollment:Input template is NULL or size is zero (size = %d). Returning %d.",
                nTemplateDataSize, ENROLL_ERROR_TEMPLATE);
            return ENROLL_ERROR_TEMPLATE;
        }
    }

    if (pImageData == NULL || nImageDataSize == 0)
    {
        LOGMSG("AddNewEnrollment:Input image is NULL or size is zero (size = %d). Returning %d.",
            nImageDataSize, ENROLL_ERROR_IMAGE);
        return ENROLL_ERROR_IMAGE;
    }

    if (false == pDM->SetCapturedImageData(m_CaptureType, pImageData, nImageDataSize))
    {
        LOGMSG("AddNewEnrollment:Set image for capture type %d returned error. Returning %d.",
            m_CaptureType, ENROLL_ERROR_IMAGE);
        return ENROLL_ERROR_IMAGE;
    }
    if (false == pDM->SetCapturedTemplateData(m_CaptureType, pTemplateData, nTemplateDataSize))
    {
        LOGMSG("AddNewEnrollment:Set template for capture type %d returned error. Returning %d.",
            m_CaptureType, ENROLL_ERROR_TEMPLATE);
        return ENROLL_ERROR_TEMPLATE;
    }
    if (false == pDM->SetCapturedSpoofScore(m_CaptureType, nSpoofScore))
    {
        LOGMSG("AddNewEnrollment:Set spoof score for capture type %d returned error. Returning %d.",
            m_CaptureType, ENROLL_ERROR_SPOOF);
        return ENROLL_ERROR_SPOOF;
    }

    // If the above are successful, advance the enrollment mode
    switch(m_CaptureType)
    {
        case CAPTURE_ENROLL_1:
        {
            m_CurrentEnrollState = Enroll_Capture_1_Complete;
        } break;
        case CAPTURE_ENROLL_2:
        {
            bool bRet = ValidateEnrollment( CAPTURE_ENROLL_1, CAPTURE_ENROLL_2, m_nMatchMatrix[0]);
            if (false == bRet)
            {
                return ENROLL_ERROR_MATCH;
            }
            m_CurrentEnrollState = Enroll_Capture_2_Complete;
        } break;
        case CAPTURE_ENROLL_3:
        {
            bool bRet1 = ValidateEnrollment( CAPTURE_ENROLL_1, CAPTURE_ENROLL_3, m_nMatchMatrix[2]);
            bool bRet2 = ValidateEnrollment( CAPTURE_ENROLL_2, CAPTURE_ENROLL_3, m_nMatchMatrix[1]);
            if ((false == bRet1) && (false == bRet2))
            {
                return ENROLL_ERROR_MATCH;
            }
            // Indicate completion.
            m_CurrentEnrollState = Enroll_Capture_3_Complete;
        } break;
        case CAPTURE_IMAGE:
        {
            m_CurrentEnrollState = Enroll_Generic_Complete;
        } break;
        case CAPTURE_VERIFY:
        {
            m_CurrentEnrollState = Enroll_Verify_Complete;
        } break;
    }

    return ENROLL_OK;
}

bool EnrollSvc::ValidateEnrollment(_V100_CAPTURE_TYPE t1, _V100_CAPTURE_TYPE t2, u32& nOutScore)
{
    DataMgr* pDM = ISensorInstance::GetInstance()->GetDataMgr();
    // Get BioParameters to determine threshold levels.,.
    // Perform Match.

    uint nScore      = 0;
    u32  nTplSize1   = 0;
    u32  nTplSize2   = 0;


    u8* pT1   = pDM->GetCapturedTemplate(t1);
    u8* pT2   = pDM->GetCapturedTemplate(t2);
    nTplSize1 = pDM->GetCapturedTemplateSize(t1);
    nTplSize2 = pDM->GetCapturedTemplateSize(t2);

    if (pT1 == NULL || pT2 == NULL)
    {
        LOGMSG("ValidateEnrollment:Invalid input template(s). Returning false.");
        return false;
    }

    if (nTplSize1 == 0 || nTplSize2 == 0)
    {
        LOGMSG("ValidateEnrollment:Invalid input template size(s). Returning false.");
        return false;
    }

    bool bRet = false;

    bRet = Match(pT1, nTplSize1, pT2, nTplSize2, nScore);
    if (bRet)
    {
        float fBioMetric = 0;
        bRet = GetMatchResult(pT1, nTplSize1, pT2, nTplSize2, nScore, &fBioMetric, true);
        nOutScore = nScore;
    }

    //// Populate local match matrix for selecting best enrollment
    //if (t1 == CAPTURE_ENROLL_1 && t2 == CAPTURE_ENROLL_2)
    //{
    //    m_nMatchMatrix[0] = nScore;
    //}
    //else if (t1 == CAPTURE_ENROLL_1 && t2 == CAPTURE_ENROLL_3)
    //{
    //    m_nMatchMatrix[2] = nScore;
    //}
    //else if (t1 == CAPTURE_ENROLL_2 && t2 == CAPTURE_ENROLL_3)
    //{
    //    m_nMatchMatrix[1] = nScore;
    //}

    return bRet;
}

// TODO: Move to base class
bool EnrollSvc::GetMatchResult(u8* pTpl1, u32 nTpl1Sz, u8* pTpl2, u32 nTpl2Size, u32 nMatchScore, float* fBioMetric, bool bUseEnrollThresh)
{
    BioParameters* pBioParams = (&BioParameters::GetInstance());
    _V100_CROP_LEVEL nCropLevel = (_V100_CROP_LEVEL)pBioParams->GetCropLevel();
    _V100_ENC_THRESHOLD_MODE nThresholdMode = (_V100_ENC_THRESHOLD_MODE)pBioParams->GetThresholdMode();
    bool bMatch = false;
    uint nType = ISensorInstance::GetInstance()->GetConfiguration()->GetSensorType();
    *fBioMetric = -1.0f;
    u16 nThreshold;

    if (bUseEnrollThresh)
    {
        nThreshold = pBioParams->GetEnrollThreshold();//% in tenths of a %
    }
    else
    {
        nThreshold = pBioParams->GetVerifyThreshold();
    }

    // nThreshold is in % in tenths of a %. Multiple FAR/FRR % from LUT tables by 10 when comparing with nThreshold

    //V-Series
    if (nType == V520_Type ||
        nType == V320_Type ||
        nType == V310_10_Type ||
        nType == V310_Type )
    {
        if (nCropLevel == CROP_NONE)
        {
            if (nThresholdMode == THRESHOLD_MODE_FAR)
            {
                float fFAR = GetFARForFullSized(nMatchScore);
                *fBioMetric = fFAR;
                fFAR = 10 * fFAR;
                if (fFAR < nThreshold)
                {
                    bMatch = true;
                }
            }
            else
            {
                float fFRR = GetFRRForFullSized(nMatchScore);
                *fBioMetric = fFRR;
                fFRR = 10 * fFRR;
                if (fFRR > nThreshold)
                {
                    bMatch = true;
                }
            }
        }
        else if (nCropLevel == CROP_272x400)
        {
            if (nThresholdMode == THRESHOLD_MODE_FAR)
            {
                float fFAR = GetFARForCropped(nMatchScore);
                *fBioMetric = fFAR;
                fFAR = 10 * fFAR;
                if (fFAR < nThreshold)
                {
                    bMatch = true;
                }
            }
            else
            {
                float fFRR = GetFRRForCropped(nMatchScore);
                *fBioMetric = fFRR;
                fFRR = 10 * fFRR;
                if (fFRR > nThreshold)
                {
                    bMatch = true;
                }
            }
        }
    }
    // M-Series //<TODO>: Add support for this later, for now just turn off
    else if (false)
    {
        MATCH_SCORE_TRANSFORM_TABLE MSTT = GetSTTForM(pTpl1, pTpl2);
        if (nThresholdMode == THRESHOLD_MODE_FAR)
        {
            float fFAR;
            if (MSTT == STT_M32X)
            {
                fFAR = GetFARForMX2X(nMatchScore);
            }
            else
            {
                fFAR = GetFARForVenusCroppedMX2X(nMatchScore);
            }
            *fBioMetric = fFAR;
            fFAR = 10 * fFAR;
            if (fFAR < nThreshold)
            {
                bMatch = true;
            }
        }
        else
        {
            float fFRR;
            if (MSTT == STT_M32X)
            {
                fFRR = GetFRRForMX2X(nMatchScore);
            }
            else
            {
                fFRR = GetFRRForVenusCroppedMX2X(nMatchScore);
            }
            *fBioMetric = fFRR;
            fFRR = 10 * fFRR;
            if (fFRR > nThreshold)
            {
                bMatch = true;
            }
        }
    }
    else
    {
        LOGMSG("GetMatchResult: Invalid sensor type for operation.");
    }

    return bMatch;
}

