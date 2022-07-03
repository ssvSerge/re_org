#ifndef __DATAMGR_H__
#define __DATAMGR_H__

#define MAX_FLAFS_CNT       (16)

class DataMgr {
    public:
        DataMgr();
        void GetTCIncFlag ( int idx, bool* bTcVal );
        void SetTCIncFlag ( int idx, bool  bTcVal );

    public:
        static DataMgr* GetInstance();

    private:
        static bool  m_TcFlagsList[MAX_FLAFS_CNT];
};
#endif
