#include "Cypher.h"
#include "stdio.h"

Cypher *pCypher = NULL;

ICypher* ICypher::GetInstance()
{
    if(pCypher == NULL)
    {
        pCypher = new Cypher();
    }
    return pCypher;
}

void ICypher::Destroy()
{
    if(pCypher != NULL)
    {
        delete pCypher;
        pCypher = NULL;
    }

}

Cypher::Cypher()
{
    m_pBSP = NULL;
}

Cypher::~Cypher(void)
{
}
bool Cypher::Initialize(ICryptoBSP* pBSP)
{
    m_pBSP = pBSP;
    return (m_pBSP==NULL)?false:true;
}

//Initialize without BSP service
bool Cypher::Initialize()
{
    return true;
}


//// BSP Services
//IMAXQ*  Cypher::GetMAXQ()
//{
//    if(m_pBSP == NULL ) return NULL;
//    return m_pBSP->GetMAXQ();
//}
//IATSHA*    Cypher::GetATSHA()
//{
//    if(m_pBSP == NULL ) return NULL;
//    return m_pBSP->GetATSHA();
//}


// methods
bool Cypher::Open(void)
{
    return false;
}
bool Cypher::Close(void)
{
    return false;
}
u32     Cypher::Get_Revision(void)
{
    return 0;
}
u32     Cypher::Timer_Start(void)
{
    return 0;
}
u32     Cypher::Timer_Stop(u32 timeIn)
{
    return 0;
}
u32     Cypher::Calc_Period( int freq )
{
    return 0;
}

IAES* Cypher::GetAES()
{
    return &m_oAES;
}
INULL* Cypher::GetNULL()
{
    return &m_oNULL;
}
ISHA1* Cypher::GetSHA1()
{
    return &m_oSHA1;
}
ISHA256* Cypher::GetSHA256()
{
    return &m_oSHA256;
}
ISHA512* Cypher::GetSHA512()
{
    return &m_oSHA512;
}
IRAND* Cypher::GetRAND()
{
    return &m_oRAND;
}
IDES* Cypher::GetDES()
{
    return &m_oDES;
}
IHMAC* Cypher::GetHMAC()
{
    return &m_oHMAC;
}
IRSA* Cypher::GetRSA()
{
    return &m_oRSA;
}
//IKEYMAN* Cypher::GetKEYMAN()
//{
//    return &m_oKEYMAN;
//}
IDUKPT* Cypher::GetDUKPT()
{
    return &m_oDUKPT;
}
IENVELOPE* Cypher::GetEnvelope()
{
    return &m_oEnvelope;
}
IX509* Cypher::GetX509()
{
    return &m_oX509;
}
//IECDSA* Cypher::GetECDSA()
//{
//    return &m_oECDSA;
//}
u32 Cypher::CRC32_Calc(u8 *buffer, u32 size, u32 seed )
{
    return 0;
}
void Cypher::CRC32_CreateTable( void )
{
    return;
}
u32 Cypher::CRC32_Reflect(u32 ref, char ch)
{
    return 0;
}
u32 Cypher::GetErrorCode(void)
{
    return 0;
}

#include "envelope.h"
bool IENVELOPE::GetKeySlot(const unsigned char * input, size_t input_len, unsigned char & keyslot_enc)
{
    if (input_len <= IENVELOPE::MIN_OVERHEAD)
        return false;
    const USBCB * pUSBCB = reinterpret_cast<const USBCB *>(input);
    uint32_t slot = (pUSBCB->ulCommand & ((uint32_t)ENVELOPE_SLOT_MASK)) >> ENVELOPE_SLOT_SHIFT;
    keyslot_enc = slot;
    return true;
}

