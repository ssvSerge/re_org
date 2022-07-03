#pragma once

#include "IMemMgr.h"
#include "V100_shared_types.h"
#include "lumi_stdint.h"
#include "IEngineV2.h"


typedef enum
{
    ENROLL_OK = 0x00,
    ENROLL_ERROR_IMAGE         = 0x01,
    ENROLL_ERROR_SPOOF        = 0x02,
    ENROLL_ERROR_TEMPLATE    = 0x03,
    ENROLL_ERROR_MATCH        = 0x04,
    ENROLL_ERROR_UNALIGNED  = 0x05,
    ENROLL_ERROR_SYSTEM        = 0x06,
    ENROLL_ERROR_PARAMETER    = 0x07,
} EnrollStatus;

typedef enum
{
    Enroll_None,
    Enroll_Capture_1_Complete,
    Enroll_Capture_2_Complete,
    Enroll_Capture_3_Complete,
    Enroll_Verify_Complete,
    Enroll_Generic_Complete
} EnrollState;



class IEnrollSvc : public MemoryBase
{
public:
    virtual ~IEnrollSvc(){};

    // Accessors
    virtual EnrollStatus    GetBestEnrollment(_V100_CAPTURE_TYPE& nBestEnrollment)=0;
    virtual EnrollStatus     AddNewEnrollment(u8* pImageData, u32 nImageDataSize, u8* pTemplateData, u32 nTemplateDataSize, int nSpoofScore) =0;
    //
    virtual EnrollStatus    SetCaptureMode(_V100_CAPTURE_TYPE ct) =0;
    _V100_CAPTURE_TYPE        GetCaptureMode() { return m_CaptureType; }

    virtual bool            ClearEnroll() =0;
    virtual bool            IsEnrolled()=0;
    virtual bool             ValidateEnrollment(_V100_CAPTURE_TYPE t1, _V100_CAPTURE_TYPE t2, u32& nScore)=0;
    virtual bool            GetMatchResult(u8* pTpl1, u32 nTpl1Sz, u8* pTpl2, u32 nTpl2Size, u32 nMatchScore, float* fBioMetric, bool bUseEnrollThresh)=0;
    static bool    Match(u8* pTpl1, u32 nTpl1Sz, u8* pTpl2, u32 nTpl2Size, u32& nMatchScore);
    static bool CheckTemplate(uchar* pTemplate, int& nNumMinutia);
    static uint GetFMRScore(u8* pTpl1, u32 nTpl1Sz, u8* pTpl2, u32 nTpl2Size, u32 nMatchScore);

protected:
    // Helper method
    static MATCH_SCORE_TRANSFORM_TABLE GetSTTForM(uchar* pProbeTpl, uchar* pGallTpl);

    _V100_CAPTURE_TYPE    m_CaptureType;
    EnrollState               m_CurrentEnrollState;
    u32                        m_nMatchMatrix[3];   // 0: 1 vs 2, 1: 2 vs 3, 2:, 1 vs 3

};
