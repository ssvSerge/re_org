#include "CryptoBSP.h"


ICryptoBSP* pBSP = NULL;

ICryptoBSP* ICryptoBSP::GetInstance()
{
    if(pBSP == NULL)
    {
        pBSP = new CryptoBSP();
    }
    return pBSP;
}

bool    CryptoBSP::Open(void)
{
    return false;
}
bool    CryptoBSP::Close(void)
{
    return false;
}
u32        CryptoBSP::Get_Revision(void)
{
    return 0;
}
u64        CryptoBSP::Timer_Start(void)
{
    return 0;
}
u64        CryptoBSP::Timer_Stop(u64 timeIn)
{
    return 1;
}
u32        CryptoBSP::Calc_Period( int freq )
{
    return 0;
}
/*IMAXQ*  CryptoBSP::GetMAXQ()
{
    return &m_oMAXQ;
}*/
/*IATSHA* CryptoBSP::GetATSHA()
{
    return &m_oATSHA;
}*/
u32        CryptoBSP::CRC32_Calc(u8 *buffer, u32 size, u32 seed )
{
    return 0;
}
void    CryptoBSP::CRC32_CreateTable( void )
{
    return;
}
u32        CryptoBSP::CRC32_Reflect(u32 ref, char ch)
{
    return 0;
}
u32        CryptoBSP::GetErrorCode(void)
{
    return 0;
}
