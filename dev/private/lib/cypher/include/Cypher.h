#pragma once
#include "ICypher.h"
#include "polarssl/lumi_config.h"

#include "polarssl/aes.h"
#include "polarssl/bignum.h"
#include "polarssl/ctr_drbg.h"
#include "polarssl/des.h"
#include "polarssl/md.h"
#include "polarssl/rsa.h"
#include "polarssl/sha1.h"
#include "polarssl/sha256.h"
#include "polarssl/sha512.h"
#include "polarssl/ecdsa.h"
#include "polarssl/ecp.h"
#include "polarssl/x509.h"
#include "c_aes.h"
#include "c_des.h"
#include "c_null.h"
//#include "c_bignum.h"
#include "c_dsa.h"
#include "c_hmac.h"
#include "c_md.h"
#include "c_rand.h"
#include "c_rsa.h"
#include "c_sha1.h"
#include "c_sha256.h"
#include "c_sha512.h"
//#include "c_keyman.h"
#include "c_dukpt.h"
#include "c_envelope.h"
#include "c_x509.h"
#include "c_ecdsa.h"
#include "IMemMgr.h"



class Cypher : public ICypher, public MemoryBase
{
public:
	Cypher();
	virtual ~Cypher(void);
	// methods
	virtual bool		Initialize(ICryptoBSP* pBSP);
	virtual bool 		Initialize();
	virtual bool		Open(void);
	virtual bool		Close(void);
 	virtual u32			Get_Revision(void);		
	virtual u32			Timer_Start(void);		
	virtual u32			Timer_Stop(u32 timeIn);	
	virtual u32			Calc_Period( int freq );

	virtual IAES*		GetAES();
	virtual INULL*		GetNULL();
	virtual ISHA1*		GetSHA1();
	virtual ISHA256*	GetSHA256();
	virtual ISHA512*	GetSHA512();
	virtual IRAND*		GetRAND();
	virtual IDES*		GetDES();
	virtual IHMAC*		GetHMAC();
	virtual IRSA*		GetRSA();
	virtual IDUKPT*		GetDUKPT();
    virtual IENVELOPE*  GetEnvelope();
    virtual IX509*      GetX509();
	//virtual IECDSA*		GetECDSA();
	// BSP Services
	//virtual IMAXQ*		GetMAXQ();
	//virtual IATSHA*		GetATSHA();
	virtual u32  CRC32_Calc(u8 *buffer, u32 size, u32 seed );
	virtual void CRC32_CreateTable( void );					 
	virtual u32  CRC32_Reflect(u32 ref, char ch);			 
	
	virtual u32 GetErrorCode(void);	
private:
	ICryptoBSP*	m_pBSP;
	oAES		m_oAES;
	oNULL		m_oNULL;
	oDES		m_oDES;
	oSHA1		m_oSHA1;
	oSHA256		m_oSHA256;
	oSHA512		m_oSHA512;
	oRAND		m_oRAND;
	oHMAC		m_oHMAC;
	oRSA		m_oRSA;
	//oKEYMAN		m_oKEYMAN;
	oDUKPT		m_oDUKPT;
    oEnvelope   m_oEnvelope;
    oX509       m_oX509;
	//oECDSA		m_oECDSA;
	

};