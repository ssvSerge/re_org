#include "EnrollSvcMSK00.h"
#include "IProcess.h"
#include "BioParameters.h"
#include "CriticalErrorLog.h"
#include "far_lut.h"
#include "SEngineProcINT16.h"

#define ENROLL_THRESH    27500 //Enrollment match threshold

/****************************************************************
*
* Mode logic for this class
*
* 1) When a first enrollment flag or verify flag gets set, clear all the buffers associated with all enrolls.
* 2) Only the "AddNewEnrollment" function may advance the m_CurrentEnrollState.
* 3) The ClearEnroll() function may reset the m_CurrentEnrollState as well
*
******************************************************************/

IEnrollSvc* EnrollSvcMSK00::GetInstance()
{
    static EnrollSvcMSK00 instance;
    return &instance;
}

EnrollSvcMSK00::EnrollSvcMSK00()
{
    m_CurrentEnrollState = Enroll_None;
    m_CaptureType = CAPTURE_IMAGE;
    m_nBestEnroll = -1;
}

EnrollStatus EnrollSvcMSK00::SetCaptureMode(_V100_CAPTURE_TYPE ct)
{
    switch(ct)
    {
        case CAPTURE_ENROLL_1:
        {
            ClearEnroll();
        } break;
        case CAPTURE_ENROLL_2:
        {
            //
        } break;
        case CAPTURE_ENROLL_3:
        {
            //
        } break;
        case CAPTURE_IMAGE:
        case CAPTURE_VERIFY:
        {
            ClearEnroll();
        } break;
    }

    m_CaptureType = ct;

    return ENROLL_OK;
}

bool EnrollSvcMSK00::ClearEnroll()
{
    DataMgr* pDM = ISensorInstance::GetInstance()->GetDataMgr();
    pDM->ClearEnrollMemory();
    // Reset Enroll State.
    m_CurrentEnrollState = Enroll_None;
    // Match Scores
    m_nMatchMatrix[0] = 0;
    m_nMatchMatrix[1] = 0;
    m_nMatchMatrix[2] = 0;

    m_nBestEnroll = -1;

    return true;
}


EnrollStatus EnrollSvcMSK00::PickBestEnrollment()
{
    // Find Template NOT involved in the lowest match
    // If 0 (1 vs 2) is lowest match, answer = 3
    // If 1 (2 vs 3) is lowest match, answer = 1
    // If 2 (1 vs 3) is lowest match, answer = 2
    _V100_CAPTURE_TYPE nBestEnrollment = CAPTURE_ENROLL_2;

    if (m_nMatchMatrix[0] <= m_nMatchMatrix[1] &&
        m_nMatchMatrix[0] <= m_nMatchMatrix[2])
    {
         nBestEnrollment = CAPTURE_ENROLL_3;
    }
    else
    if (m_nMatchMatrix[1] <= m_nMatchMatrix[2] &&
        m_nMatchMatrix[1] <= m_nMatchMatrix[0])
    {
        nBestEnrollment = CAPTURE_ENROLL_1;
    }


    bool bEnrollSuccess = false;
    for (int ii = 0 ; ii < 3; ii++)
    {
        if (m_nMatchMatrix[ii] > ENROLL_THRESH)
        {
            bEnrollSuccess = true;
            break;
        }
    }

    if (bEnrollSuccess)
    {
        m_nBestEnroll = (int)nBestEnrollment;
        return ENROLL_OK;
    }
    else
    {
        m_nBestEnroll = -1;
        LOGMSG("PickBestEnrollment:All match scores are below threshold. Returning %d",
                ENROLL_ERROR_MATCH);
        return ENROLL_ERROR_MATCH;
    }
}

EnrollStatus EnrollSvcMSK00::GetBestEnrollment(_V100_CAPTURE_TYPE& nBestEnrollment)
{

    if( m_nBestEnroll != -1)
    {
        nBestEnrollment = (_V100_CAPTURE_TYPE)m_nBestEnroll;
        return ENROLL_OK;
    }
    else
    {
        LOGMSG("GetBestEnrollment:No best enrollment found. Returning %d",
                ENROLL_ERROR_MATCH);
        return ENROLL_ERROR_MATCH;
    }
}

/*
**
** As we know, modally, where we are, we add this enrollment.
** Even if the enrollment fails, DONT delete it out of the system, as it would give the end-use a chance to retrieve the image
** and template for data collection purposes.   Just don't advance the Enrollment Mode in case of failure.
*/
EnrollStatus EnrollSvcMSK00::AddNewEnrollment(u8* pImageData, uint nImageDataSize, u8* pTemplateData, uint nTemplateDataSize, int nSpoofScore)
{
    DataMgr* pDM = ISensorInstance::GetInstance()->GetDataMgr();

    // Check for valid template data only if Extractor is On.
    if ((&BioParameters::GetInstance())->DoExtract())
    {
        if( pTemplateData == NULL || nTemplateDataSize == 0 )
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
    switch (m_CaptureType)
    {
        case CAPTURE_ENROLL_1:
        {
            m_CurrentEnrollState = Enroll_Capture_1_Complete;
        } break;
        case CAPTURE_ENROLL_2:
        {

            m_CurrentEnrollState = Enroll_Capture_2_Complete;
        } break;
        case CAPTURE_ENROLL_3:
        {
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

bool EnrollSvcMSK00::ValidateEnrollment(_V100_CAPTURE_TYPE t1, _V100_CAPTURE_TYPE t2, u32& nOutScore)
{
    DataMgr* pDM = ISensorInstance::GetInstance()->GetDataMgr();
    uint nScore =0;
    u32  nTplSize1 = 0, nTplSize2 =0;

    u8* pT1 = pDM->GetCapturedTemplate(t1);
    nTplSize1 = pDM->GetCapturedTemplateSize(t1);
    u8* pT2 = pDM->GetCapturedTemplate(t2);
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

    if (!Match(pT1, nTplSize1, pT2, nTplSize2, nScore))
    {
        LOGMSG("ValidateEnrollment:Match failed. Returning an error.");
        return false;
    }

    if (t1 == CAPTURE_ENROLL_1 && t2 == CAPTURE_ENROLL_2)
    {
        m_nMatchMatrix[0] = nScore;
    }
    else if (t1 == CAPTURE_ENROLL_1 && t2 == CAPTURE_ENROLL_3)
    {
        m_nMatchMatrix[2]= nScore;
    }
    else if (t1 == CAPTURE_ENROLL_2 && t2 == CAPTURE_ENROLL_3)
    {
        m_nMatchMatrix[1] = nScore;
    }

    nOutScore = nScore;
    return true;
}
