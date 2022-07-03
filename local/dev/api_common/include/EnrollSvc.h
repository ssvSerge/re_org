#pragma once

#include "IMemMgr.h"
#include "DataMgr.h"
#include "IEnrollSvc.h"

class EnrollSvc : public IEnrollSvc
{
public:
    static IEnrollSvc* GetInstance();
    //
    EnrollSvc();
    ~EnrollSvc(){};
    // Accessors
    virtual EnrollStatus            GetBestEnrollment(_V100_CAPTURE_TYPE& nBestEnrollment);
    virtual EnrollStatus             AddNewEnrollment(u8* pImageData, u32 nImageDataSize, u8* pTemplateData, u32 nTemplateDataSize, int nSpoofScore);
    //
    virtual EnrollStatus            SetCaptureMode(_V100_CAPTURE_TYPE ct);
    virtual _V100_CAPTURE_TYPE        GetCaptureMode() { return m_CaptureType; }

    virtual bool                    ClearEnroll();
    virtual bool                    IsEnrolled() { return m_CurrentEnrollState == Enroll_Capture_3_Complete; }
    virtual bool                     ValidateEnrollment(_V100_CAPTURE_TYPE t1, _V100_CAPTURE_TYPE t2, u32& nScore);
    virtual bool                    GetMatchResult(u8* pTpl1, u32 nTpl1Sz, u8* pTpl2, u32 nTpl2Size, u32 nMatchScore, float* fBioMetric, bool bUseEnrollThresh);
private:

    // Singleton
    static IEnrollSvc*         g_pEnrollSvc;


};

