#pragma once

#include "CExecStatus.h"


#ifdef __cplusplus
extern "C" {
#endif

#define FREE_MEM(A) if (A) {FREE(A); A = nullptr; }

#ifdef __cplusplus
} //extern "C" {
#endif
class CryptogramAdapter
{
public:
    CryptogramAdapter(ushort MID, u8* pCryptogram, uint nCryptogramSize)
    {
        // Shallow
        m_pChallengeCG = pCryptogram;
        m_nChallengeCGSize = nCryptogramSize;
        m_nMID = MID;
        m_pResponseCG = NULL;
        m_nResponseCGSize = 0;
    }
    ~CryptogramAdapter()
    {
        if(m_pResponseCG)
        {
            FREE(m_pResponseCG);
            m_pResponseCG = NULL;
        }
    };
    //
    u8*        GetChallenge() { return m_pChallengeCG; }
    uint    GetChallengeSize() { return m_nChallengeCGSize; }
    u8*      GetResponse() { return m_pResponseCG; }
    void    SetResponse(u8* pResponse, uint nResponseSize)
    {
        m_pResponseCG = pResponse;
        m_nResponseCGSize = nResponseSize;
    }
    void    SetResponseSize(u32 sz) { m_nResponseCGSize = sz; }
    uint    GetResponseSize() { return m_nResponseCGSize; }
    u32        GetMID() { return m_nMID; }
private:
    ushort m_nMID;
    u8*       m_pChallengeCG;
    uint   m_nChallengeCGSize;
    u8*       m_pResponseCG;
    uint   m_nResponseCGSize;
};

