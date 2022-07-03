#include "DataMgr.h"
#include <string.h>

bool  DataMgr::m_TcFlagsList[MAX_FLAFS_CNT];

DataMgr::DataMgr() {
    memset ( &m_TcFlagsList, 0x00, sizeof(m_TcFlagsList) );
}

DataMgr* DataMgr::GetInstance() {
    static DataMgr instance;
    return &instance;
}

void DataMgr::GetTCIncFlag ( int idx, bool* bTCInc ) {

    if ( bTCInc != nullptr) {
        if (idx<MAX_FLAFS_CNT) {
            *bTCInc = m_TcFlagsList[idx];
        } else {
            *bTCInc = false;
        }
    }
}

void DataMgr::SetTCIncFlag ( int idx, bool bTCInc ) {

    if (idx<MAX_FLAFS_CNT) {
        m_TcFlagsList[idx] = bTCInc;
    }
}
