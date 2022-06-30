#pragma once

/*******************************************************************************
**
**	ICypher - ABC for Cypher class.
**	$Id: IBSP.h 22877 2014-03-03 03:38:32Z alitz $
**
**	COPYRIGHT INFORMATION:	
**		This software is proprietary and confidential.  
**		By using this software you agree to the terms and conditions of the 
**		associated Lumidigm Inc. License Agreement.
**
**		Lumidigm Inc Copyright 2011 All Rights Reserved.
**
*******************************************************************************/
#pragma once

#include "CypherTypes.h"
#include "ICryptoBSP.h"	
#include "envelope.h"
#include "usbcmdset.h"
#include "lumi_stdint.h"

class ICRYPTOAlgo
{ 
	public:
		
		virtual bool EncryptData(const u8 *in, u8 *out, int NBytesIn, int NBytesOut, const u8 *Key, int KeyLength, u8 *IV, int Mode) =0;
		virtual bool DecryptData(const u8 *in, u8 *out, int NBytesIn, int NBytesOut, const u8 *Key, int KeyLength, u8 *IV, int Mode) =0;
};

class IAuthenticatedCRYPTOAlgo
{
	public:
	
		virtual bool Init(void)=0;
		virtual bool Self_Test(bool verbose)=0;
		virtual u32  Get_Revision(void) = 0;
		virtual bool AuthenticatedEncrypt(const u8 *in, const u8* AAD, int AAD_len, u8 *out, int NBytes,
                                          const u8 *Key, int KeyLength, u8 *IV, int IV_len, u8 *MAC, int MAC_len)=0;
		virtual bool AuthenticatedDecrypt(const u8 *in, const u8* AAD, int AAD_len, u8 *out, int NBytes,
                                          const u8 *Key, int KeyLength, u8 *IV, int IV_len, const u8* MAC, int MAC_len)=0;
};



// KEYEX Interface
class IKEYEX
{
	public:
	
	virtual bool Init(void) 		= 0;
	virtual bool Self_Test( void )	= 0;
	virtual bool EncryptAndSign( void *pSecret, size_t nBytes, u8 *pPublicKey, u8*pCT, u8 *pDS )=0;
};		

// Crypto Interface
class ICRYPTO
{ 
	public:
	
		virtual bool Init(void) = 0;
		virtual bool Self_Test( bool verbose )=0;
		virtual u32  Get_Revision(void) = 0;
		
		virtual bool Crypto_Support(void)=0;	// Detect Device Support

		// Support Functions for Ciphering
		virtual bool GetRNC( u8 *rand, int Size )=0;
		virtual bool SelectSessionKeySlot( int )=0;
		virtual bool SetKeyVersion( int )=0;
		virtual bool GetDeviceSerialNumber ( u64 *)=0;
		virtual bool GetOTP( u8 *buf )=0;
		virtual bool CheckBootImage( void )=0;
		virtual bool Doorway( int, u8 *)=0;	
		virtual bool DecryptBootImage( u8 *pIn, u8 *pOut, int ldrsize )=0;
		virtual bool UpdateLifeCounter( u64 *val )=0;
		virtual bool Erase_CSP( void ) = 0;
		virtual bool Generate_Symmetric_Key( u8 *pKey )=0;
};

class ISHA1
{
	public:
	
		virtual bool Init( void )		= 0;
		virtual bool Self_Test( bool verbose )	= 0;
		virtual u32  Get_Revision(void) = 0;
		virtual bool Hash(const u8 *Message, int NumBytes, u8 *HashValue)=0;
};

class ISHA256
{
	public:
	
		virtual bool Init( void )=0;
		virtual bool Self_Test( bool verbose )=0;
		virtual u32  Get_Revision(void) = 0;
		virtual bool Hash(const u8 *Message, int NumBytes, u8 *HashValue, bool is224 = false) = 0;
};

class ISHA512
{
	public:
	
		virtual bool Init( void )=0;
		virtual bool Self_Test( bool verbose )=0;
		virtual u32  Get_Revision(void) = 0;
		virtual bool Hash(const u8 *Message, int NumBytes, u8 *HashValue, int is384 = false) = 0;
		virtual bool HMAC_Hash(const u8 *pKey, int nKeySize, const u8 *pMsg, int nMsgSize, u8 *HashValue, int is384 = false) = 0;
};

class IAES : public ICRYPTOAlgo
{
	public:
	
		virtual bool Init( void )=0;
		virtual bool Self_Test( bool verbose )=0;
		virtual u32  Get_Revision(void) = 0;
		virtual bool Encrypt(const u8 *in, u8 *out, int NBytes, const u8 *Key, int KeyLength, u8 *IV, int Mode)=0;
		virtual bool Decrypt(const u8 *in, u8 *out, int NBytes, const u8 *Key, int KeyLength, u8 *IV, int Mode)=0;
		virtual bool EncryptData(const u8 *in, u8 *out, int NBytesIn, int NBytesOut, const u8 *Key, int KeyLength, u8 *IV, int Mode)
		{
			if (NBytesIn != NBytesOut)
                return false;
			return Encrypt(in, out, NBytesIn, Key, KeyLength, IV, Mode);
		}
		virtual bool DecryptData(const u8 *in, u8 *out, int NBytesIn, int NBytesOut, const u8 *Key, int KeyLength, u8 *IV, int Mode)
		{
			if (NBytesIn != NBytesOut)
                return false;
			return Decrypt(in, out, NBytesIn,  Key, KeyLength, IV, Mode); 
		}
};

class INULL: public ICRYPTOAlgo
{
	virtual bool EncryptData(const u8 *in, u8 *out, int NBytesIn, int NBytesOut, const u8 *Key, int KeyLength, u8 *IV, int Mode) = 0;
	virtual bool DecryptData(const u8 *in, u8 *out, int NBytesIn, int NBytesOut, const u8 *Key, int KeyLength, u8 *IV, int Mode) = 0;
};
class IAESGCM : IAuthenticatedCRYPTOAlgo
{
	public:
		virtual bool Init(void)=0;
		virtual bool Self_Test(bool verbose)=0;
		virtual u32  Get_Revision(void) = 0;
		virtual bool AuthenticatedEncrypt(const u8 *in, const u8* AAD, int AAD_len, u8 *out, int NBytes,
                                          const u8 *Key, int KeyLength, u8 *IV, int IV_len, u8 *MAC, int MAC_len)=0;
		virtual bool AuthenticatedDecrypt(const u8 *in, const u8* AAD, int AAD_len, u8 *out, int NBytes,
                                          const u8 *Key, int KeyLength, u8 *IV, int IV_len, const u8* MAC, int MAC_len)=0;
};
class IRAND
{
	public:
	
		virtual bool Init(bool predictRes = false) = 0;
		virtual bool Self_Test( bool verbose )=0;
		virtual u32  Get_Revision(void) = 0;
		virtual bool Random( u8 *pData, int N )=0;	// Generate Random Number of Bytes
        virtual bool Generate_NIST(
                                    bool                  bPredictRes,
                                    const unsigned char * EntropyInput,
                                    size_t                EntropyInput_len,
                                    const unsigned char * EntropyInputPR_1,
                                    size_t                EntropyInputPR_1_len,
                                    const unsigned char * EntropyInputPR_2,
                                    size_t                EntropyInputPR_2_len,
                                    const unsigned char * Nonce,
                                    size_t                Nonce_len,
                                    const unsigned char * PersonalizationString,
                                    size_t                PersonalizationString_len,
                                    const unsigned char * AdditionalInput_1,
                                    size_t                AdditionalInput_1_len,
                                    const unsigned char * AdditionalInput_2,
                                    size_t                AdditionalInput_2_len,
                                    const unsigned char * EntropyInputReseed,
                                    size_t                EntropyInputReseed_len,
                                    const unsigned char * AdditionalInputReseed,
                                    size_t                AdditionalInputReseed_len,
                                          unsigned char * Result,
                                    size_t                Result_len
                                  ) = 0;
};

class IDES : public ICRYPTOAlgo
{
	public:
	
		virtual bool Init( void ) = 0;
		virtual bool Self_Test( bool verbose ) =0;
		virtual u32  Get_Revision(void) = 0;
		virtual bool Encrypt(const u8 *pIn, u8 *pOut, int NBytes, const u8 *pKEY1, const u8 *pKEY2, const u8 *pKEY3, u8 *IV, int Mode)=0;
		virtual bool Decrypt(const u8 *pIn, u8 *pOut, int NBytes, const u8 *pKEY1, const u8 *pKEY2, const u8 *pKEY3, u8 *IV, int Mode)=0;
		virtual bool EncryptData(const u8 *in, u8 *out, int NBytesIn, int NBytesOut, const u8 *Key, int KeyLength, u8 *IV, int Mode)
		{
			if(NBytesIn != NBytesOut) return false;
			switch (Mode)
			{
				case DES_ECB:
				case DES_CBC:
					return Encrypt(in, out, NBytesIn, Key, NULL, NULL, IV, Mode);
				case TDES_CBC:
				case TDES_ECB:
					if(KeyLength == 16)
					{
						return Encrypt(in, out, NBytesIn, Key, Key+8, Key, IV, Mode);
					}
					else if(KeyLength == 24)
					{
						return Encrypt(in, out, NBytesIn, Key, Key+8, Key+2*8, IV, Mode);
					}
					else
					{
						return false;
					}
				default:
					return false;
			}
		}
		virtual bool DecryptData(const u8 *in, u8 *out, int NBytesIn, int NBytesOut, const u8 *Key, int KeyLength, u8 *IV, int Mode)
		{
			if(NBytesIn != NBytesOut) return false;
			switch (Mode)
			{
				case DES_ECB:
				case DES_CBC:
					return Decrypt(in, out, NBytesIn, Key, NULL, NULL, IV, Mode);
				case TDES_CBC:
				case TDES_ECB:
					if(KeyLength == 16)
					{
						return Decrypt(in, out, NBytesIn, Key, Key+8, Key, IV, Mode);
					}
					else if(KeyLength == 24)
					{
						return Decrypt(in, out, NBytesIn, Key, Key+8, Key+2*8, IV, Mode);
					}
					else
					{
						return false;
					}
				default:
					return false;
			}
		}
};

// DUKPT Interface
class IDUKPT : public ICRYPTOAlgo
{
	public:
	
		virtual bool Init(void) = 0;
		virtual bool Self_Test(bool verbose)=0;
		virtual u32  Get_Revision(void) = 0;
		virtual	bool Encrypt(const u8 *pIn, size_t nBytesIn, u8 *pOut, size_t nBytesOut, size_t *pSize)=0;
		virtual bool Decrypt(const u8 *pIn, size_t nBytesIn, u8 *pOut, size_t nBytesOut, size_t *pSize, const u8 *Key, int KeyLength) =0;
		virtual bool SetContext(u8* IPEK, u8* KSN, u32 cnt) = 0;
		virtual bool EncryptData(const u8 *in, u8 *out, int NBytesIn, int NBytesOut, const u8 *Key, int KeyLength, u8 *IV, int Mode)
		{
			size_t nActualBytesOut;
			if(false  == Encrypt(in, NBytesIn, out, NBytesOut, &nActualBytesOut)) return false;
			if(NBytesOut != (int)nActualBytesOut) return false;
			return true;
		}
		virtual bool DecryptData(const u8 *in, u8 *out, int NBytesIn, int NBytesOut, const u8 *Key, int KeyLength, u8 *IV, int Mode)
		{
			size_t nActualBytesOut;
			if(false == Decrypt(in, NBytesIn, out,  NBytesOut, &nActualBytesOut, Key, KeyLength)) return false;
			if(NBytesOut != (int)nActualBytesOut) return false;
			return true;
		}
		virtual bool Update_KSN(u32 cnt) = 0;
};

class IHMAC
{
	public:
	
		virtual bool Init( void )=0;
		virtual bool Self_Test( bool verbose )=0;
		virtual u32  Get_Revision(void) = 0;
		virtual bool HMAC( u8 *Msg, int MsgSize, u8 *Key, int KeySize, u8 *MAC, int *MACSize, int Mode)=0;
		//virtual bool HMAC_Authentication(u256 *pHMAC, u256 *pHostRNC, u256 *pSensorRNC, void *pOtherData, size_t nOtherSize)=0;
		//virtual bool HMAC_Authentication_Generic(u256 *pHMAC, void *pOtherData, size_t nOtherSize)=0;
};		

class IRSA
{
	public:
	
	virtual bool Init( int padding,  int hash_id)=0;
    virtual void Clear() = 0;
	virtual bool Self_Test( bool verbose )=0;
	virtual u32  Get_Revision(void)=0;
	virtual bool Encrypt( void *p_rng, int mode, size_t ilen, u8 *input, u8 *output )=0;
	virtual bool Decrypt( int mode, size_t *olen, u8 *input, u8 *output, size_t output_max_len )=0;
	virtual bool Sign( void *p_rng, int mode, int hash_id, u32 hashlen, u8 *hash, u8 *sig )=0;
	virtual bool Verify(int mode, int hash_id, u32 hashlen, const u8 *hash, const u8 *sig)=0;
	virtual bool Generate_Keypair( void *p_rng, u32 nbits, int exponent )=0;
	virtual bool GetPublicKey( u8 *pKey, u32 *rSize ) = 0;
	virtual bool GetPrivateKey( u8 *pKey, u32 *rSize ) = 0;
	
	virtual bool SetContext(const u8 *pKeyPub, const u8 *pKeyPriv, u32 Exponent, u32 rSize)=0;
	virtual bool SetContextPublic(const u8 *pKey, u32 nBytes, u32 Exponent) = 0;
	virtual bool GetPrimes( u8 *pP, u8 *pQ, u32 *, u32 nSize ) = 0;

	virtual bool SetContextKeyPair(const u8 *pN, const u8 *pD, u32 rSize, u32 Exponent)=0;
	virtual bool SetMode (int Padding, int Hash)=0;
	virtual bool SetRandTestMode (bool bMode, u8 *pSeed, int nSeedLen =20)=0;
    virtual bool SetContextPublicPEM(const char * pKey) = 0;
    virtual bool SetContextPrivatePEM(const char * pKey) = 0;
    virtual bool GetPublicKeyPEM(char *pKey, u32 *rSize) = 0;
    virtual bool GetPrivateKeyPEM(char *pKey, u32 *rSize) = 0;
    virtual bool GetRootCAPEM(const char * subject, uint32_t validity_days, char * PEM, u32 * rSize) = 0;
    virtual bool GetCertSignReqPEM(const char * subject, int key_usage, char * PEM, u32 * rSize) = 0;

    virtual bool Generate_Keypair_Sealed(void * p_rng, u32 nbits, int exponent,
                                         const unsigned char * EKEY,        size_t   EKEY_len,
                                         const unsigned char * MKEY,        size_t   MKEY_len,
                                               unsigned char * ENV_PEM_prv, size_t * ENV_PEM_prv_len,   // NULLs to disable prv
                                               unsigned char * ENV_PEM_pub, size_t * ENV_PEM_pub_len,   // NULLs to disable pub
                                               unsigned char * ENV_PEM_csr, size_t * ENV_PEM_csr_len,   // NULLs to disable csr
                                                  const char * subject, int key_usage) = 0;             // NULL/-1 to disable csr
    virtual bool SetContextPrivateSealed(const unsigned char * EKEY,           size_t   EKEY_len,
                                         const unsigned char * MKEY,           size_t   MKEY_len,
                                         const unsigned char * ENV_sealed_prv, size_t   ENV_sealed_prv_len) = 0;
    virtual bool SetContextPublicSealed( const unsigned char * EKEY,           size_t   EKEY_len,
                                         const unsigned char * MKEY,           size_t   MKEY_len,
                                         const unsigned char * ENV_sealed_pub, size_t   ENV_sealed_pub_len) = 0;
	virtual bool Restore_Device_Context() = 0;
};		


class IECDSA
{
	public:
		virtual bool Init(int ecp_gid, int hash_id)= 0;
    virtual void Clear() = 0;
	virtual bool Self_Test(bool verbose)=0;
	virtual u32  Get_Revision(void)=0;
	virtual bool Generate_Keypair(void *p_rng, int ecp_gid, u32 nbits) = 0;
	virtual bool GetPrivateKey(u8 *pD, u32 *rSize) = 0;
	virtual bool GetPublicKey(u8 *pQx, u8*pQy, u32 *rSize) = 0;
	virtual bool SetContextKeyPair(int ecp_gid, const u8 *pQx, const u8  *pQy, const u8 *pD, u32 rSize) = 0;
	virtual bool SetContextPublic(int ecp_gid, const u8 *pQx, const u8  *pQy, u32 rSize) = 0;
	virtual bool Sign(void *p_rng, int hash_id, u32 hashlen, u8 *hash, u8 *r, u8* s) = 0;
	virtual bool Verify(int hash_id, u32 hashlen, u8 *hash, u8 *r, u8* s) = 0;
	virtual bool CalHashAndSign(void *p_rng, int hash_id, u32 ilen, u8 *input, u8 *r, u8* s) = 0;
	virtual bool CalHashAndVerify(int hash_id, u32 ilen, u8 *input, u8 *r, u8* s) = 0;
	virtual bool SetMode(int Mode, int Hash) = 0;
	virtual bool SetRandTestMode(bool bMode, u8 *pSeed, int nSeedLen) = 0;
	
};


class IENVELOPE
{
public:
    // Message ID field of ciphertext envelope.
    enum MID
    {
		// RESERVED SPACE FOR VCOM COMMAND ID FOR BACKEND-STYLE CRYPTOGRAMS
		// 0x00000000 -> 0x000000FF
		// DO NOT USE FOR ANY OTHER PURPOSE.

        // PROTOCOLS
		FIRMWARE_IMAGE					=	0x4D524946,    // BINARY firmware image 'F''I''R''M' in LE format
		XPRT_VCOM_ENCAPSULATED			=	0x4D4F4356,	   // BINARY Transport Security VCOM encapsulated 'V''C''O''M' in LE format

        // KEY STORAGE
		HMAC_SHA_256_FW_AUTH_TOKEN		=   0x7FFFCFFF,	   // Binary HMAC-SHA-256/256 Firmware  auth token
        HMAC_SHA_256_128_SECRET_KEY		=   0x7FFFDFFF,    // BINARY HMAC-SHA-256/128 secret key
        AES_GCM_256_SECRET_KEY			=   0x7FFFEFFA,    // BINARY AES-GCM-256 secret key
        AES_GCM_192_SECRET_KEY			=   0x7FFFEFFB,    // BINARY AES-GCM-192 secret key
        AES_GCM_128_SECRET_KEY			=   0x7FFFEFFC,    // BINARY AES-GCM-128 secret key
        AES_CTR_256_SECRET_KEY			=   0x7FFFEFFD,    // BINARY AES-CTR-256 secret key
        AES_CTR_192_SECRET_KEY			=   0x7FFFEFFE,    // BINARY AES-CTR-192 secret key
        AES_CTR_128_SECRET_KEY			=   0x7FFFEFFF,    // BINARY AES-CTR-128 secret key
        RSA_2048_CERT_CHAIN				=   0x7FFFFFF9,    // PEM PKCS # 7 RSA-2048 CONCATENATED CERTIFICATE CHAIN
        RSA_2048_PUB_KEY				=   0x7FFFFFFA,    // PEM PKCS # 8 RSA-2048 public key
        RSA_2048_PRIV_KEY				=   0x7FFFFFFB,    // PEM PKCS # 1 RSA-2048 private key
        RSA_2048_CERT_REQ				=   0x7FFFFFFC,    // PEM PKCS #10 RSA-2048 certificate signing request, self-signed
        RSA_2048_LEAF_CERT				=   0x7FFFFFFD,    // PEM PKCS # 7 RSA-2048 X.509 LEAF certificate
        RSA_2048_INTER_CA				=   0x7FFFFFFE,    // PEM PKCS # 7 RSA-2048 X.509 INTERMEDIATE CA certificate
        RSA_2048_ROOT_CA				=   0x7FFFFFFF,    // PEM PKCS # 7 RSA-2048 X.509 ROOT CA certificate
    };

    enum CONSTANTS
    {
		MIN_MAC_SIZE                =         16,          // we don't think about MACs less than 128 bits
		MAX_MAC_SIZE                =         32,          // we don't think about MACs greater than output of HMAC-SHA-256
        RSA_2048_PEM_PUB_SIZE       =        512,          // reasonable size for RSA-2048 public key
        RSA_2048_PEM_PRV_SIZE       =       1700,          // reasonable size for RSA-2048 private key
        RSA_2048_PEM_CSR_SIZE       =       1200,          // reasonable size for RSA-2048 certificate signing request, self-signed
        RSA_2048_PEM_CRT_SIZE       =       1300,          // reasonable size for RSA-2048 public certificate
		MIN_OVERHEAD                = sizeof(USBCB) + 16/*IV*/ + sizeof(uint32_t)/*MID*/ + MIN_MAC_SIZE/*max MAC*/,
        MAX_OVERHEAD                = sizeof(USBCB) + 16/*IV*/ + sizeof(uint32_t)/*MID*/ + MAX_MAC_SIZE/*max MAC*/
    };

	enum CIPHERSUITE
	{
		CIPHERSUITE_FIPS_AES_HMAC = ENVELOPE_CIPHERSUITE_FIPS_AES_HMAC, // #defines from envelope.h
		CIPHERSUITE_FIPS_AES_GCM  = ENVELOPE_CIPHERSUITE_FIPS_AES_GCM
	};

    virtual bool   Seal(const unsigned char * EKEY, size_t EKEY_len,
                        const unsigned char * MKEY, size_t MKEY_len,
                        uint32_t mid, unsigned char * iv,
                        const unsigned char * input,  size_t   input_len,
                              unsigned char * output, size_t * output_len,
						uint32_t      SEQ         = 0,
                        unsigned char SLOT        = 0,
                        unsigned char LAYERS      = 0,
                        unsigned char CIPHERSUITE = CIPHERSUITE_FIPS_AES_HMAC) = 0;

    virtual bool Unseal(const unsigned char * EKEY, size_t EKEY_len,
                        const unsigned char * MKEY, size_t MKEY_len,
                        uint32_t mid, unsigned char * iv,
                        const unsigned char * input,  size_t   input_len,
                              unsigned char * output, size_t * output_len,
						uint32_t      * SEQ           = NULL,
                        unsigned char * SLOT          = NULL,
                        unsigned char * LAYERS        = NULL,
                        unsigned char * CIPHERSUITE   = NULL) = 0;

	static bool GetKeySlot(const unsigned char * input, size_t input_len, unsigned char & keyslot_enc);
};

class IX509
{
public:
                 IX509() {}
    virtual      ~IX509() {}

    virtual bool UnsealRootCA(const unsigned char * EKEY,       size_t   EKEY_len,
                              const unsigned char * MKEY,       size_t   MKEY_len,
                              const unsigned char * CertSealed, size_t   CertSealedLen,
                                             char * CertPEM,    size_t * CertPEMLen) = 0;

    virtual bool UnsealInterCA(const unsigned char * EKEY,       size_t   EKEY_len,
                               const unsigned char * MKEY,       size_t   MKEY_len,
                               const unsigned char * CertSealed, size_t   CertSealedLen,
                                              char * CertPEM,    size_t * CertPEMLen) = 0;

    virtual bool UnsealLeafCert(const unsigned char * EKEY,       size_t   EKEY_len,
                                const unsigned char * MKEY,       size_t   MKEY_len,
                                const unsigned char * CertSealed, size_t   CertSealedLen,
                                               char * CertPEM,    size_t * CertPEMLen) = 0;

    virtual bool UnsealCertSignReq(const unsigned char * EKEY,              size_t   EKEY_len,
                                   const unsigned char * MKEY,              size_t   MKEY_len,
                                   const unsigned char * CertSignReqSealed, size_t   CertSignReqSealedLen,
                                                  char * CertSignReqPEM,    size_t * CertSignReqPEMLen) = 0;

    virtual bool SealRootCA(const unsigned char * EKEY,       size_t   EKEY_len,
                            const unsigned char * MKEY,       size_t   MKEY_len,
                            const          char * CertPEM,    size_t   CertPEMLen,
                                  unsigned char * CertSealed, size_t * CertSealedLen) = 0;

    virtual bool SealInterCA(const unsigned char * EKEY,       size_t   EKEY_len,
                             const unsigned char * MKEY,       size_t   MKEY_len,
                             const          char * CertPEM,    size_t   CertPEMLen,
                                   unsigned char * CertSealed, size_t * CertSealedLen) = 0;
 
    virtual bool SealLeafCert(const unsigned char * EKEY,       size_t   EKEY_len,
                              const unsigned char * MKEY,       size_t   MKEY_len,
                              const          char * CertPEM,    size_t   CertPEMLen,
                                    unsigned char * CertSealed, size_t * CertSealedLen) = 0;

    virtual bool SealCertSignReq(const unsigned char * EKEY,              size_t   EKEY_len,
                                 const unsigned char * MKEY,              size_t   MKEY_len,
                                 const          char * CertSignReqPEM,    size_t   CertSignReqPEMLen,
                                       unsigned char * CertSignReqSealed, size_t * CertSignReqSealedLen) = 0;

    virtual bool VerifyCertSignReq(const char * CertReqPEM, size_t CertReqPEMLen) = 0;
	// SIGN a certificate request with a CA cert - using plaintext private key and producing plaintext output
	virtual bool SignCertReq(const          char * CertReqPEM,		 size_t   CertReqPEMLen,
							 const          char * SigningCertPEM,	 size_t   SigningCertPEMLen,
							 const          char * PrivateKeyPEM,	 size_t   PrivateKeyPEMLen,
							 uint32_t              validity_days,	 uint32_t serial,
							 int                   key_usage,        bool CA, int max_pathlen,
							                char * CertSignedPEM,	 size_t * CertSignedPEMLen) = 0;
	// SIGN a certificate request with a CA cert - using sealed private key and producing sealed output
    virtual bool SignCertReq(const unsigned char * EKEY,             size_t   EKEY_len,
                             const unsigned char * MKEY,             size_t   MKEY_len,
                             const          char * CertReqPEM,       size_t   CertReqPEMLen,
                             const          char * SigningCertPEM,   size_t   SigningCertPEMLen,
                             const unsigned char * PrivateKeySealed, size_t   PrivateKeySealedLen,
                             uint32_t              validity_days,    uint32_t serial,
                             int                   key_usage,        uint32_t mid,
                                   unsigned char * SignedCertSealed, size_t * SignedCertSealedLen) = 0;

    virtual bool VerifyCertChain(const char * RootCAPEM,    size_t RootCAPEMLen,
                                 const char * CertChainPEM, size_t CertChainPEMLen) = 0;

    virtual bool CertReqGetPublicKeyPEM(const char * CertReqPEM,   size_t   CertReqPEMLen,
                                              char * PublicKeyPEM, size_t * PublicKeyPEMLen) = 0;

    virtual bool CertGetPublicKeyPEM   (const char * CertPEM,      size_t   CertPEMLen,
                                              char * PublicKeyPEM, size_t * PublicKeyPEMLen) = 0;

protected:

    virtual bool Unseal(const unsigned char * EKEY,   size_t   EKEY_len,
                        const unsigned char * MKEY,   size_t   MKEY_len,
                        const unsigned char * Sealed, size_t   SealedLen,
                        uint32_t              mid,
                                       char * PEM,    size_t * PEMLen) = 0;

    virtual bool Seal(const unsigned char * EKEY,   size_t   EKEY_len,
                      const unsigned char * MKEY,   size_t   MKEY_len,
                      const          char * PEM,    size_t   PEMLen,
                      uint32_t              mid,
                            unsigned char * Sealed, size_t * SealedLen) = 0;
};
class ICypher
{
public:
		// Init
		virtual bool Initialize(ICryptoBSP* pBSP) = 0;
		virtual bool Initialize() 				  = 0;
		// Cypher/Hash Engines
		virtual ~ICypher(){};
		virtual IAES*		GetAES()    = 0;
		virtual INULL*		GetNULL()	  = 0;
		virtual ISHA1*		GetSHA1()   = 0;
		virtual ISHA256*	GetSHA256() = 0;
		virtual ISHA512*	GetSHA512() = 0;
		virtual IRAND*		GetRAND()   = 0;
		virtual IDES*		GetDES() 	= 0;
		virtual IHMAC*		GetHMAC()   = 0;
		virtual IRSA*		GetRSA() 	= 0;
		virtual IDUKPT*		GetDUKPT()    = 0;
        virtual IENVELOPE*  GetEnvelope() = 0;
        virtual IX509*      GetX509()     = 0;
        
		// BSP Services
		//virtual IMAXQ*		GetMAXQ()	  = 0;
		//virtual IATSHA*		GetATSHA()	  = 0;
		// CRC 32
		virtual u32  CRC32_Calc(u8 *buffer, u32 size, u32 seed ) =0;
		virtual void CRC32_CreateTable( void )					 =0;
		virtual u32  CRC32_Reflect(u32 ref, char ch)			 =0;
		// Last Error
		virtual u32 GetErrorCode(void)							 =0;
		// Diagnostic methods
		virtual u32 Timer_Start() = 0;
		virtual u32 Timer_Stop(u32 time)  = 0;
		/*
		**	Static Singleton
		*/

		static ICypher* GetInstance();
		static void     Destroy();
};


