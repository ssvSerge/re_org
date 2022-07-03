#pragma once

#include "IMemMgr.h"
#include "DataMgr.h"
#include "IEnrollSvc.h"


class EnrollSvcMSK00 : public IEnrollSvc
{
public:
    static IEnrollSvc* GetInstance();
    //
    EnrollSvcMSK00();
    ~EnrollSvcMSK00(){};
    // Accessors
    EnrollStatus                    PickBestEnrollment();
    virtual EnrollStatus            GetBestEnrollment(_V100_CAPTURE_TYPE& nBestEnrollment);
    virtual EnrollStatus             AddNewEnrollment(u8* pImageData, uint nImageDataSize, u8* pTemplateData, uint nTemplateDataSize, int nSpoofScore);
    //
    virtual EnrollStatus            SetCaptureMode(_V100_CAPTURE_TYPE ct);
    virtual _V100_CAPTURE_TYPE        GetCaptureMode() { return m_CaptureType; }
    virtual bool                     ValidateEnrollment(_V100_CAPTURE_TYPE t1, _V100_CAPTURE_TYPE t2, u32& nScore);

    virtual bool                    ClearEnroll();
    virtual bool                    IsEnrolled() { return (m_nBestEnroll !=-1)?true:false; }

    bool                            GetMatchResult(u8* pTpl1, u32 nTpl1Sz, u8* pTpl2, u32 nTpl2Size, u32 nMatchScore, float* fBioMetric, bool bUseEnrollThresh) { return false; };

private:

    // Singleton
    static IEnrollSvc*         g_pEnrollSvc;

    int                        m_nBestEnroll;

};

