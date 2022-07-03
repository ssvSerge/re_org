// Capture Statistics
//

#include "V100_enc_types.h"
#include "V100_shared_types.h"
#include "DataMgr.h"    // For MAX_CAPTURE_TYPES definition


class CaptureStats
{
public:
    static const _V100_CAPTURE_STATS* Get(_V100_CAPTURE_TYPE CaptureType);
    static bool Update(_V100_CAPTURE_TYPE nCaptureType);
    static bool Copy(_V100_CAPTURE_TYPE src, _V100_CAPTURE_TYPE dst);
    static void Clear();
    static void Clear(int index);
private:
    static inline unsigned int ImageArea();
    static inline unsigned char ImageQuality();
    static inline unsigned char ImageMinutiaCount();
    static inline unsigned char EnrollQuality();

    static _V100_CAPTURE_STATS m_CaptureStats[MAX_CAPTURE_TYPES];
};
