// Capture Statistics
//

#include "CaptureStats.h"
#include "ISensorInstance.h"
#include "CfgMgr.h"
#include "DataMgr.h"
#include "IEnrollSvc.h"
#include "V100_enc_types.h"
#include "V100_shared_types.h"
#include "string.h"


#define IMAGE_QUALITY_VERSION        0    // 0 means unimplemented
#define ENROLL_QUALITY_VERSION        0    // ... update as needed


// TODO: Revisit capture stats. Make independent of IEnrollSvc? Or only IEnrollSvc calls into CaptureStats?


const _V100_CAPTURE_STATS* CaptureStats::Get(_V100_CAPTURE_TYPE CaptureType)
{
    DataMgr* pDM = ISensorInstance::GetInstance()->GetDataMgr();

    int nIndex = pDM->GetIndexByCaptureType(CaptureType);
    if (nIndex < 0)
    {
        return NULL;
    }

    // a zero-sized image area will occur if the capture info
    // has been cleared, never filled, or if the processing is
    // disabled.  return NULL to indicate the data is not
    // available.  (likewise, the template size can be zero
    // if the extractor is turned off, but this is not an
    // error.)
    if (m_CaptureStats[nIndex].nImageArea == 0)
    {
        return NULL;
    }

    return &m_CaptureStats[nIndex];
}


bool CaptureStats::Update(_V100_CAPTURE_TYPE nCaptureType)
{
    DataMgr* pDM = ISensorInstance::GetInstance()->GetDataMgr();

    int nIndex = pDM->GetIndexByCaptureType(nCaptureType);
    if (nIndex < 0)
        return false;

    m_CaptureStats[nIndex].nImageArea = ImageArea();
    m_CaptureStats[nIndex].nImageQuality = ImageQuality();
    m_CaptureStats[nIndex].nImageQualityVersion = IMAGE_QUALITY_VERSION;
    m_CaptureStats[nIndex].nImageMinutiaCount = ImageMinutiaCount();
    m_CaptureStats[nIndex].nEnrollQuality = EnrollQuality();
    m_CaptureStats[nIndex].nEnrollQualityVersion = ENROLL_QUALITY_VERSION;
    return true;
}

bool CaptureStats::Copy(_V100_CAPTURE_TYPE src, _V100_CAPTURE_TYPE dst)
{
    DataMgr* pDM = ISensorInstance::GetInstance()->GetDataMgr();

    int nIndexSrc = pDM->GetIndexByCaptureType(src);
    int nIndexDst = pDM->GetIndexByCaptureType(dst);
    if (nIndexSrc < 0 || nIndexDst < 0)
        return false;

    memcpy(&m_CaptureStats[nIndexDst], &m_CaptureStats[nIndexSrc], sizeof(_V100_CAPTURE_STATS));
    return true;
}

void CaptureStats::Clear()
{
    memset(m_CaptureStats, 0, sizeof(m_CaptureStats));
}

void CaptureStats::Clear(int index)
{
    if (index < MAX_CAPTURE_TYPES)
        memset(&m_CaptureStats[index], 0, sizeof(_V100_CAPTURE_STATS));
}


// private functions

unsigned int CaptureStats::ImageArea()
{
    // compute the number of pixels set in the scaled mask
    DataMgr* pDM = ISensorInstance::GetInstance()->GetDataMgr();
    _V100_INTERFACE_CONFIGURATION_TYPE* pICT = pDM->GetInterfaceConfiguration();

    uint count = 0;
    uchar* pScaledMask = pDM->GetScaledMask();
    for (int i = 0; i < pICT->Composite_Image_Size_X * pICT->Composite_Image_Size_Y; ++i)
    {
        if (pScaledMask[i])
        {
            count++;
        }
    }
    return count;
}

unsigned char CaptureStats::ImageQuality()
{
    return 0;
}

unsigned char CaptureStats::ImageMinutiaCount()
{
    // extract the minutia count, if template extraction enabled...
    DataMgr* pDM = ISensorInstance::GetInstance()->GetDataMgr();
    _V100_INTERFACE_COMMAND_TYPE* pCMD = pDM->GetCMD();

    u8 minutia_count = 0;
    if (pCMD->Select_Extractor != 0 )
    {
        uint nProbeTemplateSize = 0;
        uchar* pProbeTemplate = pDM->GetProbeTemplate(&nProbeTemplateSize);
        if (nProbeTemplateSize)
        {
            int nNumMinutia = 0;
            bool bRC = IEnrollSvc::CheckTemplate(pProbeTemplate, nNumMinutia);
            if( true == bRC)
            {
                minutia_count = (u8)nNumMinutia;
            }
            else
            {
                minutia_count = 0;
            }
        }
    }

    return minutia_count;
}

unsigned char CaptureStats::EnrollQuality()
{
    return 0;
}


// private data

_V100_CAPTURE_STATS CaptureStats::m_CaptureStats[] = {};
