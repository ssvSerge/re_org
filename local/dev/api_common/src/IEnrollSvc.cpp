#include "IEnrollSvc.h"
#include "ISensorInstance.h"
#include "Configuration.h"
#include "BioParameters.h"
#include "far_lut.h"
#include "ILib1ToN.h"
#include "ISEngineProc.h"
#include "CriticalErrorLog.h"


bool IEnrollSvc::Match(u8* pTpl1, u32 nTpl1Sz, u8* pTpl2, u32 nTpl2Size, u32& nMatchScore)
{
    // Get BioParameters to determine threshold levels.,.
    // Perform Match.

    uint nScore = 0;

    if (pTpl1 == NULL || pTpl2 == NULL)
    {
        LOGMSG("Match:Invalid input template(s). Returning false.");
        return false;
    }

    if (nTpl1Sz == 0 || nTpl2Size == 0)
    {
        LOGMSG("Match:Invalid input template size(s). Returning false.");
        return false;
    }

    if (false == ISensorInstance::GetInstance()->GetSEProc()->Match(pTpl1, nTpl1Sz, pTpl2, nTpl2Size, &nScore))
    {
        LOGMSG("Match:Match on SEProc failed. Returning false.");
        return false;
    }

    nMatchScore = nScore;
    return true;
}

bool IEnrollSvc::CheckTemplate(uchar* pTemplate, int& nNumMinutia)
{
    IDStatus iS = ILib1ToN::CheckTemplate(pTemplate, nNumMinutia);
    if (ID_OK != iS)
    {
        return false;
    }

    return true;
}

uint IEnrollSvc::GetFMRScore(u8* pTpl1, u32 nTpl1Sz, u8* pTpl2, u32 nTpl2Size, u32 nMatchScore)
{
    uint nType = ISensorInstance::GetInstance()->GetConfiguration()->GetSensorType();
    if (nType == V310_10_Type || nType == V310_Type || nType == V520_Type || nType == V320_Type)
    {
        _V100_CROP_LEVEL nCropLev = BioParameters::GetInstance().GetCropLevel();
        double FMRPercentLumi = 0.0f;
        if (nCropLev == CROP_272x400)
        {
            FMRPercentLumi = (double)GetFARForCropped(nMatchScore);
        }
        else
        {
            FMRPercentLumi = (double)GetFARForFullSized(nMatchScore);
        }

        // Normalizando o valor.
        double FMRScore = (double)(0x7FFFFFFF * (FMRPercentLumi / 100.0));
        return (uint)FMRScore;
        return 0;
    }
    else
    {
        uint FMRScore;

        MATCH_SCORE_TRANSFORM_TABLE MSTT = GetSTTForM(pTpl1, pTpl2);

        double FMRPercentLumi = 0.0f;
        if (MSTT == STT_M32X)
        {
            FMRPercentLumi = (double)GetFARForMX2X(nMatchScore);
        }
        else
        {
            FMRPercentLumi = (double)GetFARForVenusCroppedMX2X(nMatchScore);
        }


        // Normalizando o valor.
        double nFMRScore = (double)(0x7FFFFFFF * (FMRPercentLumi / 100.0));
        FMRScore = (uint)nFMRScore;

        return FMRScore;
    }
}


bool CheckIfTplIsM(uchar* pTpl)
{
    int pos = 16;
    uint nImgXSize = pTpl[pos++] << 8;
    nImgXSize += pTpl[pos++];

    uint nImgYSize = pTpl[pos++] << 8;
    nImgYSize += pTpl[pos++];

    if (nImgXSize == 280 && nImgYSize == 352)        return true;
    else return false;
}
//This function assumes that we are using it for M sensor type
MATCH_SCORE_TRANSFORM_TABLE IEnrollSvc::GetSTTForM(uchar* pProbeTpl, uchar* pGallTpl)
{
    // If both tempaltes are M series use default STT otherwise use Mixed ST VCroppedvsM
    MATCH_SCORE_TRANSFORM_TABLE MSTT = DEFAULT_M_STT;
    if (false == CheckIfTplIsM(pProbeTpl) || false == CheckIfTplIsM(pGallTpl))
    {
        MSTT = STT_VENUS_CROPPED_MX2X;
    }

    return MSTT;
}