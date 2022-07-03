#ifndef __CRYPTOMGR_H__
#define __CRYPTOMGR_H__

#include <hid_stdint.h>
#include <V100_enc_types.h>
#include <ICypher.h>

// #include <Cypher.h>

// #include "ICypher.h"
// #include "IMemMgr.h"
// #include "V100_enc_types.h"
// #include "CryptoShared.h"
// 

#define KT_KEY_CMPK             (0x2000)
#define N_ROW                   (4)
#define N_COL                   (4)
#define N_BLOCK                 (N_ROW * N_COL)
// #define N_MAX_ROUNDS           14


class HostCryptoMgr {

    private:
        HostCryptoMgr();
       ~HostCryptoMgr();

    public:
        static HostCryptoMgr* GetInstance();
        static void           Destroy();

    public:
        u16  GetKeySizeFromSlot(u16 nSlot, u16 nKeyMode);
        u16  GetKCVTypeFromSlot(u16 nSlot, u16 nKeyMode);
        bool CalculateKCV(u8* pKey, u16 nKeySize, u32 nBytesToCheck, u8* pKCV, int nAlgo);
        bool SetActiveKey(u16 nKeySlot, u8* pKeyVal, u16 nKeySize, u16 nKeyMode);
        bool Encrypt( u8 *pIn, u8 **pOut, uint* nSizeInOut, u256 pDigSigOut = NULL);
        bool Decrypt( u8 *pIn, u8 *pOut,  uint* nSizeInOut, u256 pDigSigIn = NULL);
        void ReleaseMem(u8* pBuffer);
        bool GetRandomNumber(u8* pRnd, int nSize);
        bool Init();

        // bool PerformSelfDiagnostic(bool bVerbose);
        // u16  GetActiveKeySlot() { return m_nActiveKeySlot; };
        // u16  GetActiveKeySize() { return m_nActiveKeySz; };
        // bool WriteASN( u8 *pKey, size_t nKeySize, u8 *pExp, size_t nExpSize, char *pFilename );
        // bool GenerateASN( u8 *pKey, size_t nKeySize, u8 *pExp, size_t nExpSize, unsigned char** pASNBuff,  size_t &ASNSz);
        // bool ParseASN( u8 *pKey, size_t nKeySize, u8 *pExp, size_t nExpSize, u8* pASNBuff );
        // bool GetSHA256(u8* InputData, size_t InputDataSize, u256 pHashValue);

    private:
        bool CreateKeyCheckValue(u8 *pKey, size_t nKey, size_t nZeros, size_t nVals, u8* pKCV, int nAlg);
        u32  GetKCVTypeForVariableMode(u16 nKeyMode);
        u32  GetKeySizeForVariableMode(u16 nKeyMode);
        bool GetTransaction_Counter(u32* pCnt, int whichone);

        // bool ValidateKeyMode(u16 nSlot, u16 nKeyMode);
        // bool Set_DUKPT_Mode(int iMode);

        u2048                m_ActiveKey;           // The active key for cyphering
        _V100_ENC_KEY_MODE   m_nKeyMode;            // The key mode for the active key
        uint                 m_nMode;               // Cypher mode
        uint                 m_nActiveKeySz;        // The size of the active key
        u16                  m_nActiveKeySlot;        // The active key slot for cyphering
        ICypher*             m_pCypher;             // 
        ICRYPTOAlgo*         m_pCryptoAlgo;         // 
};

#endif

