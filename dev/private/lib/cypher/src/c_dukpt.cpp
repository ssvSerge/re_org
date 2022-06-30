/*******************************************************************************
**
**	V300b Updated Board Support Package
**	$Id: v300b_dukpt.cpp 23139 2014-03-27 13:24:21Z spcorcoran $
**
**	COPYRIGHT INFORMATION:
**		This software is proprietary and confidential.
**		By using this software you agree to the terms and conditions of the
**		associated Lumidigm Inc. License Agreement.
**
**		Lumidigm Inc Copyright 2014 All Rights Reserved.
**
**	Note:  	DUKPT is Big Endian, everything here is little endian
**			byte swapping is performed for cipher ops
**			MAC Authentication Not Required
**			Zero Padding is Default
**
**	TODO
**		Validate
**			KSN passed into Key Slot SPARE_1 and SPARE_2
**			unique Transaction Counters
**
**		Future Key Table is NOT managed or used
**		DATA Encryption Key Generated but not Supported
*******************************************************************************/
/*
**	Includes
*/

#include "c_dukpt.h"
#include "c_dukpt_tvec.h"

/*
**	Lib Rev
*/
static char rid[] = "$Rev: 23482 $";

// Define to '1' use PIN encryption key instead of DATA encryption key
#define USE_PIN_KEY		0

/*
**	Support Macros
*/
#define TDES_BLKSIZE	8		// DES Block Size
#define AES_BLKSIZE		16		// AES Block Size
#define _MAX_( a, b )	(a > b ? a : b) 
#define MAX_BLKSIZE		(_MAX_(TDES_BLKSIZE, AES_BLKSIZE)) 	// Max Block Size

// Access to Left/Right Half of Current Key
#define pRH_CURKEY ((u8*)&pREG->CURKEY[0])
#define pLH_CURKEY ((u8*)&pREG->CURKEY[8])
#define RH_CURKEY (*(u64*)&pREG->CURKEY[0])
#define LH_CURKEY (*(u64*)&pREG->CURKEY[8])
#define DUPTK_KSN_SIZE 	10				// Specification of KSN Size in Bytes

u64 VENDOR_KSN = 0x8877665544332211;

typedef struct
{
	u64
	R8,
	R8A,
	R8B;
	u32	 R3 : 21;
	u32  SR : 21;
	u64  KSNR;
	u128 IKEY;
	u128 CURKEY;
} Internal_Reg_Type;

/******************************************************************************
** Revision
******************************************************************************/
u32 oDUKPT::Get_Revision(void)
{
	int rev;
	sscanf(&rid[6], "%d", &rev);
	return (u32)rev;
}

/******************************************************************************
**	Initialize Object
******************************************************************************/
bool oDUKPT::Init(void)
{
	ClearContext();
	return true;
}

void oDUKPT::ClearContext()
{
	memset(&m_dftKSN, 0, sizeof(m_dftKSN));
	memset(&m_iKSN, 0, sizeof(m_iKSN));
	memset(m_tmp, 0, sizeof(m_tmp));
	memset(m_iBDK, 0, sizeof(m_iBDK));
	memset(m_iIPEK, 0, sizeof(m_iIPEK));	
	memset(m_iMAC, 0, sizeof(m_iMAC));
	memset(m_iDATA, 0, sizeof(m_iDATA));
	memset(m_iSK, 0, sizeof(m_iSK));		
}
/******************************************************************************
**	A context consists of KEY + KSN  and stored locally
******************************************************************************/
bool oDUKPT::SetContext(u8* IPEK, u8* KSN, u32 cnt)
{
	ClearContext();
	/*
	**	Get IPEK
	*/

	swap_bytes(IPEK, m_iIPEK, sizeof(u128));// Convert to Little Endian. // Only 128b used

	/*
	**	Get Vendor KSN
	*/
	// Actual KSN is 10 bytes 0xFF 0xFF 43bit ID and 21 bit TC. In Big Endian format. First two bytes are unused
	// Input KSN is 8 Bytes with 0xFF 0xFF 43bit ID and 1 bit TC. We need to convert to Little Endian and fill ID part in to m_dftKSN. Add TC from MAXQ
	u64 nTempKey;
	swap_bytes(KSN, (u8*)&nTempKey, sizeof(u64));

	memset(&m_dftKSN, 0xFF, sizeof(KSNType));		// Default
	memcpy((u8*)(&m_dftKSN) + 2, &nTempKey, sizeof(u64)-2);	// Only 64b used

	if (!Update_KSN(cnt))	// Update TC
	{
		return false;
	}

	return true;
}

/*******************************************************************************
** Wrapper for Decrypt which provides iPEK key
*******************************************************************************/

bool oDUKPT::Decrypt(const u8 *pIn, size_t nBytesIn, u8 *pOut, size_t nBytesOut, size_t *pSize, const u8 *Key, int KeyLength)
{
	if (KeyLength != sizeof(m_iIPEK)) return false;

	// member m_iIPEK is in little Endian. Input Key is in Big Endian 
	u128 iPEK_BE;
	swap_bytes(Key, iPEK_BE, sizeof(u128));
	if (memcmp(m_iIPEK, iPEK_BE, sizeof(u128)) != 0) return false;

	/*
	** Convert BIG ENDIAN DUKPT Packet to LITTLE ENDIAN
	*/
	u8* pLEIn = (u8*)MALLOC(nBytesIn);
	if (pLEIn == NULL) return false;

	if (false == ToLittleEndian(pIn, pLEIn, nBytesIn))
	{
		FREE(pLEIn);
		return false;
	}

	if (false == Decrypt(pLEIn, nBytesIn, pOut, nBytesOut, pSize))
	{
		FREE(pLEIn);
		return false;
	}


	FREE(pLEIn);
	return true;
}

/*******************************************************************************
**	HOST SIDE Decrypt
**  FORMAT:
**		pIn..............[ KSN Header || Cryptogram ]
**		nBytesIn.........Size of pIn Packet
**		pOut.............[ PlainText || Padding]
**		pSize........sizeof of PlainText+Padding
*******************************************************************************/
bool oDUKPT::Decrypt(const u8 *pIn, size_t nBytesIn, u8 *pOut, size_t nBytesMax, size_t *pSize)
{
	KSNType pKSN;
	size_t nCGSize;
	const u8 *pCG;

	/*
	**	Test Inputs
	*/
	if (nBytesIn < sizeof(KSNType))
		return false;

#if 0	
	/*
	**	KSN passed in is BIG Endian, convert to LITTLE
	*/
	swap_bytes(pIn, (u8*)&pKSN, sizeof(KSNType));
#else
	memcpy((u8*)&pKSN, pIn, sizeof(KSNType));
#endif

	//Validate KSN
	if (pKSN.CNTR == 0)	 return false;

	pCG = &pIn[sizeof(KSNType)];
	nCGSize = nBytesIn - sizeof(KSNType);

	if (nCGSize > nBytesMax)
		return false;

	/*
	** Host Known the IPEK
	*/
	if (!Derive_SK(&pKSN, &m_iIPEK))
		goto ABORT;

	/*
	**	Perform Dencryption
	*/
#if USE_PIN_KEY
	if (!decipher(pCG, pOut, m_iSK, nCGSize, sizeof(u128)))
		goto ABORT;
#else
	if (!decipher(pCG, pOut, m_iDATA, nCGSize, sizeof(u128)))
		goto ABORT;
#endif

	*pSize = nCGSize;

	return true;
ABORT:
	return false;
}


/******************************************************************************
**	Encrypt Message using DUKPT Key
**
**	pIn.........Pointer to PlainText Message
**	nBytesIn....Size of PlainText Message
**  pOut........Returned Cryptogram of Format {KSN[10 Byte Header] || Cryptogram[nBytes]}
**  pBytesMax...sizeof Memory Allocated by Calling Function
**	pSize.......Actual Size of Crytogram with Padding
**
**  Ensure:
**		pOut Memory is Sized to accept nBytesIn + sizeof(KSNType) + PAD
**		PAD to modulus of blksize (8 TDES or 16 AES) Per PKCS#7 Technique or Zero Padding
**
******************************************************************************/
bool oDUKPT::Encrypt(const u8 *pInData, size_t nBytesIn, u8 *pOut, size_t nBytesMax, size_t *pSize)
{
	size_t nSize, nPad;
	u8 *pPkt;

	//Ensure we have the valid counter
	if (m_iKSN.CNTR == 0)	return false;// 0 denotes overflow

	/*
	** Ensure Calling Function has Allocated Enough Memory
	** Calculate PKCS#7 Padding
	*/
	if (nBytesIn % TDES_BLKSIZE == 0)
		nPad = 0;
	else
		nPad = TDES_BLKSIZE - (nBytesIn % TDES_BLKSIZE);	// Pad Size and Value

	nSize = nBytesIn + nPad;
	if (nSize % TDES_BLKSIZE != 0)
		return false;

	/*
	**	Check to Ensure Calling Function has Allocated Enough Memory
	*/
	if (sizeof(KSNType)+nSize > nBytesMax)
		return false;

	/*
	**	Create Packet Locally
	*/
	if ((pPkt = (u8*)MALLOC(sizeof(KSNType)+nSize)) == NULL)
		return false;
#if 0		
	memset(pPkt, nPad, sizeof(KSNType)+nSize);					// set to PCKS#7 Pad	
#else
	memset(pPkt, 0x00, sizeof(KSNType)+nSize);					// set to Zero Pad	
#endif

	// Swap the data to return in the same format that we sent in
	swap_bytes(pInData, &pPkt[sizeof(KSNType)] + nPad, nBytesIn);

	/*
	**	Copy KSN and Transaction Counter
	*/
	//swap_bytes( (u8*)&m_iKSN, pPkt, sizeof(KSNType));		// BIG-ENDIAN KSN
	memcpy(pPkt, &m_iKSN, sizeof(KSNType));		// LITTLE-ENDIAN KSN


	/*
	**	Derive New Session Key
	*/
	if (!Derive_SK(&m_iKSN, &m_iIPEK))
		goto ABORT;

	/*
	**	Perform Encryption
	*/
#if USE_PIN_KEY
	if (!cipher(&pPkt[sizeof(KSNType)], &pPkt[sizeof(KSNType)], m_iSK, nSize, sizeof(u128)))
		goto ABORT;
#else
	if (!cipher(&pPkt[sizeof(KSNType)], &pPkt[sizeof(KSNType)], m_iDATA, nSize, sizeof(u128)))
		goto ABORT;
#endif

	/*
	** Return as BIG-ENDIAN
	*/
	swap_bytes(pPkt, pPkt, sizeof(KSNType));			// Swap KSN
	swap_bytes(&pPkt[sizeof(KSNType)], &pPkt[sizeof(KSNType)], nSize);

	memcpy(pOut, pPkt, sizeof(KSNType)+nSize);		// Return Cryptogram
	*pSize = sizeof(KSNType)+nSize;					// Return Cryptogram Size in Bytes

	/*
	** Clean Up
	*/
	FREE(pPkt);
	return true;
ABORT:
	FREE(pPkt);
	return false;
}

/******************************************************************************
** Create Device KSN and Update Transaction Counter
******************************************************************************/
bool oDUKPT::Update_KSN(u32 cnt)
{
	/*
	**	Build KSN
	*/
	m_iKSN = m_dftKSN;
	m_iKSN.CNTR = cnt;			// right-most 21b Transaction Counter

	return true;
}//end

/******************************************************************************
**	Derivation of Transaction Key (SK)
******************************************************************************/
bool oDUKPT::Derive_SK(KSNType *pKSN, u128 *pIPEK)
{
	Internal_Reg_Type *pREG;

	/*
	** Grab some Memory
	*/
	if ((pREG = (Internal_Reg_Type*)MALLOC(sizeof(Internal_Reg_Type))) == NULL)
		return false;
	else
		memset(pREG, 0x00, sizeof(Internal_Reg_Type));	// Initialize Registers

	memcpy(&pREG->IKEY, pIPEK, sizeof(u128));			// Load Initial Key
	memcpy(&pREG->KSNR, pKSN, sizeof(u64));				// LSB's

	/*
	**	Generate a Key
	*/
	memcpy(&pREG->CURKEY, &pREG->IKEY, sizeof(u128));		// Load
	pREG->R8 = pREG->KSNR;									// Load KSN
	pREG->R8 &= 0xFFFFFFFFFFE00000;							// Clear 21 LSB's
	pREG->R3 = pREG->KSNR;									// Load 21b
	pREG->SR = 0x100000;									// Set Shift Register

	for (u32 sr = pREG->SR; sr > 0; sr >>= 1)
	{
		if ((sr & pREG->R3) > 0)
		{
			u64 * p64;

			pREG->R8 |= sr;                                 // Set Bit

			p64 = reinterpret_cast<u64*>(pRH_CURKEY);
			pREG->R8A = pREG->R8 ^ *p64;                // XOR

			if (!cipher((u8*)&pREG->R8A,                   // DEA-Encrypt
				(u8*)&pREG->R8A,
				pLH_CURKEY, sizeof(u64), sizeof(u64)))
				return false;

			p64 = reinterpret_cast<u64*>(pRH_CURKEY);
			pREG->R8A ^= *p64;

			for (uint i = 0; i < sizeof(u128); i++)            // XOR MASK
				pREG->CURKEY[i] ^= KEYMASK[i];

			p64 = reinterpret_cast<u64*>(pRH_CURKEY);
			pREG->R8B = pREG->R8 ^ *p64;               // XOR

			if (!cipher((u8*)&pREG->R8B,                   // DEA-Encrypt
				(u8*)&pREG->R8B,
				pLH_CURKEY, sizeof(u64), sizeof(u64)))
				return false;

			p64 = reinterpret_cast<u64*>(pRH_CURKEY);
			pREG->R8B ^= *p64;                          // XOR

			memcpy(pRH_CURKEY, &pREG->R8A, sizeof(u64));   // Load
			memcpy(pLH_CURKEY, &pREG->R8B, sizeof(u64));   // Load

		}
	}

	/*
	**	Derive Keys
	**		Session Key (Transaction Key)
	*		MAC Key
	**		Data Encryption Key
	*/
	memcpy(m_tmp, &pREG->CURKEY, sizeof(u128));				// Save Derivation

	for (uint i = 0; i < sizeof(u128); i++)
		m_iMAC[i] = m_tmp[i] ^ MAC_MASK[i];					// Generate MAC Request Key 

	for (uint i = 0; i < sizeof(u128); i++)					// Generate PIN Key 
		pREG->CURKEY[i] ^= PEKMASK[i];

	memcpy(m_iSK, &pREG->CURKEY, sizeof(u128));				// Save SK Result

	/*
	**	Derive A Data Encryption Key
	*/
	for (uint i = 0; i < sizeof(u128); i++)					// Apply Mask	
		m_iDATA[i] = m_tmp[i] ^ DATA_MASK[i];

	memcpy(m_tmp, m_iDATA, sizeof(u128));						// set as key

	if (!cipher(m_iDATA, 									// TDES-CBC-Encrypt
		m_iDATA,
		m_tmp, sizeof(u64), sizeof(u128)))
		goto ABORT;

	if (!cipher(&m_iDATA[8], 								// TDES-CBC-Encrypt
		&m_iDATA[8],
		m_tmp, sizeof(u64), sizeof(u128)))
		goto ABORT;

	memset(m_tmp, 0x00, sizeof(u128));						// Destroy

	/*
	**	Clean Up
	*/
	memset(pREG, 0x00, sizeof(Internal_Reg_Type));		// Destroy Registers
	FREE(pREG);
	return true;
ABORT:
	memset(pREG, 0x00, sizeof(Internal_Reg_Type));		// Destroy Registers
	FREE(pREG);
	return false;
}

/******************************************************************************
**  HOST ONLY
**	Section A.6 Derivation of IPEK
**  All inputs LITTLE-ENDIAN
******************************************************************************/
bool oDUKPT::Derive_IPEK(u128 *pIPEK)
{
	u8 out[sizeof(u64)];
	KSNType  ksn;

	if (sizeof(KSNType) != 10)
		goto ABORT;
	/*
	**	Mask off 21b Transaction Counter
	*/
	memcpy((u8*)&ksn, &m_iKSN, sizeof(KSNType));	// cache a little-endian copy
	ksn.CNTR = 0x00;							// Mask Transaction Counter

	/*
	** take 8 MSB's of KSN and TDES using double-length version derivation key TECB mode of Ref 2
	*/
	if (!cipher(&((u8*)&ksn)[2], out, m_iBDK, sizeof(u64), sizeof(u128)))
		goto ABORT;

	/*
	** use ciphertext as left half of initial key
	*/
	memcpy(&m_iIPEK[sizeof(u64)], out, sizeof(u64));

	/*
	** take 8 MSB and TDES of derivation key XORed KEYMASK
	*/
	for (uint i = 0; i < sizeof(u128); i++)
		m_tmp[i] = m_iBDK[i] ^ KEYMASK[i];

	if (!cipher(&((u8*)&ksn)[2], out, m_tmp, sizeof(u64), sizeof(u128)))
		goto ABORT;

	/*
	** user ciphertext as right half of initial key
	*/
	memcpy(pIPEK, out, sizeof(u64));
	memset(m_tmp, 0x00, sizeof(u128));
	return true;

ABORT:
	return false;
} //end

/******************************************************************************
** DUPKT uses TDES encrypt-decrypt-encrypt CBC Mode
** and DES ECB encrypt
*****************************************************************************/
bool oDUKPT::cipher(const u8 *pIn, u8 *pOut, const u8 *pKey, size_t nBytes, size_t nKeySize)
{
	IDES *pDES = ICypher::GetInstance()->GetDES();
	u8 *ptIn, *ptOut, *ptKey, *pIV;

	// Allocate Memory
	if ((ptIn = (u8*)MALLOC(nBytes)) == NULL)
		return false;

	if ((ptOut = (u8*)MALLOC(nBytes)) == NULL)
		return false;

	if ((ptKey = (u8*)MALLOC(nKeySize)) == NULL)
		return false;

	if ((pIV = (u8*)MALLOC(AES_BLKSIZE)) == NULL)
		return false;
	memset(pIV, 0, AES_BLKSIZE);

	// Swap to BIG-Endian
	swap_bytes(pKey, ptKey, nKeySize);
	swap_bytes(pIn, ptIn, nBytes);

	switch (nKeySize)
	{
	case sizeof(u64) :		// DES_ECB
		if (!pDES->Encrypt(ptIn, ptOut, sizeof(u64), ptKey, NULL, NULL, pIV, DES_ECB))
			goto ABORT;
		break;

	case sizeof(u128) :		// 2-DES
		if (!pDES->Encrypt(ptIn, ptOut, (int)nBytes, ptKey, &ptKey[sizeof(u64)], ptKey, pIV, TDES_CBC))
			goto ABORT;
		break;
	default:
		goto ABORT;
	}

	// Swap to Little-Endian
	swap_bytes(ptOut, pOut, nBytes);

	// clean up
	FREE(ptIn); FREE(ptOut); FREE(ptKey); FREE(pIV);
	return true;
ABORT:
	FREE(ptIn); FREE(ptOut); FREE(ptKey); FREE(pIV);
	return false;
}

/******************************************************************************
**  Handle Endian
******************************************************************************/
bool oDUKPT::decipher(const u8 *pIn, u8 *pOut, const u8 *pKey, size_t nBytes, size_t nKeySize)
{
	IDES *pDES = ICypher::GetInstance()->GetDES();
	u8 *ptIn, *ptOut, *ptKey, *pIV;

	// Allocate Memory
	if ((ptIn = (u8*)MALLOC(nBytes)) == NULL)
		return false;

	if ((ptOut = (u8*)MALLOC(nBytes)) == NULL)
		return false;

	if ((ptKey = (u8*)MALLOC(nKeySize)) == NULL)
		return false;

	if ((pIV = (u8*)MALLOC(AES_BLKSIZE)) == NULL)
		return false;
	memset(pIV, 0, AES_BLKSIZE);

	// Swap to BIG-Endian
	swap_bytes(pKey, ptKey, nKeySize);
	swap_bytes(pIn, ptIn, nBytes);

	switch (nKeySize)
	{
	case sizeof(u64) :		// DES_CBC

		if (!pDES->Decrypt(ptIn, ptOut, sizeof(u64), ptKey, NULL, NULL, pIV, DES_ECB))
			goto ABORT;
		break;

	case sizeof(u128) :		// 2-DES

		if (!pDES->Decrypt(ptIn, ptOut, (int)nBytes, ptKey, &ptKey[sizeof(u64)], ptKey, pIV, TDES_CBC))
			goto ABORT;
		break;
	default:
		goto ABORT;
	}

	// Just copy the output (no swapping needed)
	memcpy(pOut, ptOut, nBytes);


	// clean up
	FREE(ptIn); FREE(ptOut); FREE(ptKey); FREE(pIV);
	return true;
ABORT:
	FREE(ptIn); FREE(ptOut); FREE(ptKey); FREE(pIV);
	return false;
}

/******************************************************************************
**	DEBUG, print key
******************************************************************************/
/*bool oDUKPT::print( u8 *pPtr, size_t nBytes, EndianType eType )
{
if ( eType == _LE_ )
for (int i = 0; i < nBytes ; i++ ) 		// LSB First
printf("%02x", pPtr[i]);

if ( eType == _BE_ )
for (int i = nBytes-1; i >= 0 ; i-- ) 	// MSB First
printf("%02x", pPtr[i]);

printf("\n");
return true;
}*/

/******************************************************************************
** byte reversal for vector
******************************************************************************/
void oDUKPT::swap_bytes(const u8 *pIn, u8 *pOut, size_t nBytes)
{
	u8 *pTmp = (u8*)MALLOC(nBytes);
	for (size_t i = 0; i < nBytes; i++)
		pTmp[i] = pIn[nBytes - 1 - i];

	memcpy(pOut, pTmp, nBytes);
	FREE(pTmp);
}

/******************************************************************************
**	Convert a Network (Big) Endian DUKPT Packet to Little Endian
******************************************************************************/
bool oDUKPT::ToLittleEndian(const u8 *pIn, u8 *pOut, size_t nBytes)
{
	if (nBytes < sizeof(KSNType))
		return false;
	swap_bytes(pIn, pOut, sizeof(KSNType));
	swap_bytes(&pIn[sizeof(KSNType)], &pOut[sizeof(KSNType)], nBytes - sizeof(KSNType));
	return true;
}

/******************************************************************************
** LRC - Longitudinal Redundancy Check
******************************************************************************/
bool oDUKPT::GenLRC(u8 *pIn, size_t nBytes, u8 *pLRC)
{
	u8 lrc = 0x00;

	for (size_t i = 0; i < nBytes; i++)
	{
		lrc = (lrc + pIn[i]) & 0xFF;
	}

	*pLRC = (((lrc ^ 0xFF) + 1) & 0xFF);
	return true;
}

/******************************************************************************
**
******************************************************************************/
oDUKPT::oDUKPT()
{
}//end

/******************************************************************************
**
******************************************************************************/
oDUKPT::~oDUKPT()
{
	ClearContext();
}//end

/******************************************************************************
**	Self Test DO NOT MODIFY INTERNAL STATES
**	Temporary Override of BDK, IPEK, and KSN with KAT values
******************************************************************************/
bool oDUKPT::Self_Test(bool verbose)
{
	u8 Out[8];
	u8 *pPktIn, *pPktOut, *pOut;
	size_t nCGSize, nBytesOut;

	/*
	** Grab Some Memory
	*/
	size_t nb = (sizeof(KSNType)+sizeof(tvec)) + MAX_BLKSIZE;


	if ((pPktIn = (u8*)MALLOC(nb)) == NULL)
		return false;

	if ((pPktOut = (u8*)MALLOC(nb)) == NULL)
		return false;

	if ((pOut = (u8*)MALLOC(nb)) == NULL)
		return false;

	/*
	**	Check KSN packing size, MUST Be 10 Bytes
	*/
	if (sizeof(KSNType) != DUPTK_KSN_SIZE)
		goto ABORT;

	/*
	**	Generate IPEK
	**  NOTE: THis is done at HSM
	*/
	memcpy(m_iBDK, (u8*)KAT.BDK, sizeof(u128)); 			// OVER-RIDE
	memcpy(&m_iKSN, (u8*)KAT.KSN, sizeof(KSNType));	 		// OVER-RIDE

	if (!Derive_IPEK(&m_iIPEK))
		goto ABORT;

	if (memcmp(m_iIPEK, KAT.IPEK, sizeof(u128)) != 0)		// Check Result
		goto ABORT;

	/*
	**	Create a Transaction Key (SK)
	*/
	memcpy(m_iIPEK, (u8*)KAT.IPEK, sizeof(u128)); 			// OVER-RIDE
	memcpy(&m_iKSN, (u8*)KAT.KSN, sizeof(KSNType)); 		// OVER-RIDE

	if (!Derive_SK(&m_iKSN, &m_iIPEK))
		goto ABORT;

	if (memcmp(m_iSK, KAT.SK, sizeof(u128)) != 0)			// Check Result
		goto ABORT;

	// Swap the IPEK to use with Big Endian Test Vectors
	u128 iPEK_BE;
	swap_bytes(m_iIPEK, iPEK_BE, sizeof(u128));

	/*
	**	Load known KSN and Create future Transaction Keys (SK)
	**  Compare Encryption Result vs. Known Answer Test (KAT)
	**  NOTE: in practice, one calls Encrypt/Decrypt Functions, but this
	**        permits over-riding KSN with KAT's
	*/
	for (uint i = 0; i < sizeof(katEPB) / sizeof(u64); i++)
	{
		memcpy(&m_iKSN, &katKSN[i][0], sizeof(KSNType));		// OVER-RIDE

		// Test with Little Endian Test Vector Inputs
		if (!Derive_SK(&m_iKSN, &m_iIPEK))
			goto ABORT;

#if USE_PIN_KEY
		if (!cipher(clearEPB, Out, m_iSK, sizeof(clearEPB), sizeof(u128)))
			goto ABORT;

		if (memcmp(Out, &katEPB[i][0], sizeof(u64)) != 0)
			goto ABORT;

		// Test with Big Endian Test Vector Inputs
		if (!Encrypt(clearEPB_BE, sizeof(clearEPB_BE), pOut, nb, &nCGSize))
			goto ABORT;

		if (memcmp(pOut + sizeof(KSNType), &katEPB_BE[i][0], sizeof(u64)) != 0)
			goto ABORT;

		if (!Decrypt(pOut, nCGSize, pPktOut, nCGSize, &nBytesOut, iPEK_BE, sizeof(m_iIPEK)))
			goto ABORT;

		/*
		** don't use nBytesOut because it's PADDED
		**/
		if (memcmp(pPktOut, clearEPB_BE, sizeof(clearEPB_BE)) != 0)
			goto ABORT;
#else
		if (!cipher(clearData, Out, m_iDATA, sizeof(clearData), sizeof(u128)))
			goto ABORT;

		if (memcmp(Out, &katEncryptedData[i][0], sizeof(u64)) != 0)
			goto ABORT;

		// Test with Big Endian Test Vector Inputs
		if (!Encrypt(clearData_BE, sizeof(clearData_BE), pOut, nb, &nCGSize))
			goto ABORT;

		if (memcmp(pOut + sizeof(KSNType), &katEncryptedData_BE[i][0], sizeof(u64)) != 0)
			goto ABORT;

		if (!Decrypt(pOut, nCGSize, pPktOut, nCGSize, &nBytesOut, iPEK_BE, sizeof(m_iIPEK)))
			goto ABORT;

		/*
		** don't use nBytesOut because it's PADDED
		**/
		if (memcmp(pPktOut, clearData_BE, sizeof(clearData_BE)) != 0)
			goto ABORT;
#endif
	}

	/*
	**	Simulation Device ===> Host
	**	Ciphers ClearText Input with Manageed SK
	**	Returns [KSN || Cryptogram]
	*/
	if (!Encrypt(tvec, sizeof(tvec), pPktOut, nb, &nCGSize))	// DEVICE SIDE
		goto ABORT;

	/*
	**	Simulate Host Decryption
	*/
	memcpy(pPktIn, pPktOut, nCGSize);
	memset(pPktOut, 0, nb);
	if (!Decrypt(pPktIn, nCGSize, pPktOut, nCGSize, &nBytesOut, iPEK_BE, sizeof(m_iIPEK)))
		goto ABORT;

	/*
	** don't use nBytesOut because it's PADDED
	**/
	if (memcmp(pPktOut, tvec, sizeof(tvec)) != 0)
		goto ABORT;

	/*
	**	Clean Up
	*/
	FREE(pPktIn); FREE(pPktOut);
	return true;

ABORT:
	FREE(pPktIn); FREE(pPktOut);
	return false;
}//end


