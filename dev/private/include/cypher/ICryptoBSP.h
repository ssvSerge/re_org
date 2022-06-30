/*******************************************************************************
**
**	V300b Updated Board Support Package
**	$Id: IBSP.h 22877 2014-03-03 03:38:32Z spcorcoran $
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

/* No longer used
// MaxQ1850 Interface
class IMAXQ
{
	public:
	
		virtual bool Init(void) = 0;
		virtual bool Self_Test( bool verbose )=0;
		virtual u32  Get_Revision(void) = 0;
		
		virtual bool GetVersion( u32 *pVer )=0;
		virtual bool GetSerialNumber( u64 *pVal ) = 0;
		virtual bool GetStatus( void *S )=0;
		virtual bool Reset(void)=0;

		// ciphering		
		virtual bool Encrypt( u8 *in, u8 *out, int NBytes, int whichkey )=0;
		virtual bool Decrypt( u8 *in, u8 *out, int NBytes, int whichkey )=0;
		virtual bool UpdateNewSessionKey( u8 *pCG, int NBytes )=0;
		virtual bool UpdateMKDKey( u8 *pCG, int Size1, u8 *pDG, int Size2 )=0;

		// Random Number support		
		virtual bool SetANSOL( u8 *pRNC, int NBytes, bool bEncrypted )=0;
		virtual bool SetANBIO( u8 *pRNC, int NBytes, bool bEncrypted )=0;
		virtual bool GetANSOL( u8 *pRNC, int NBytes, bool bEncrypted )=0;
		virtual bool GetANBIO( u8 *pRNC, int NBytes, bool bEncrypted )=0;
		virtual bool GetRNC( u256 *pRNC )=0;

		// Manufacturing Support
		virtual bool GetIOSTATE( u8 *pOut )=0;
		virtual bool SetIOBit( u8 Bit, u8 Val )=0;
		
		// Com Port Bootloader Support
		virtual bool LockDevice( void )=0;
		virtual bool UnLockDevice( void )=0;
		virtual bool ProgramDevice( int Option, u8 *pFile, int iFileSize )=0;
		virtual bool BootLoaderReset(void)=0;
	
		// Key Management
		virtual bool GetKey( u256 *pKey, int idx )=0;
		virtual bool SetKey( u256 *pKey, int idx )=0;
		virtual bool Erase_CSP( void ) = 0;
		virtual bool SetEncryptionMode( int )=0;
		virtual bool Set_IV( u8 *pIV )=0;
		virtual bool SetPKIKey( void *pKey, int idx )=0;
		virtual bool GetPKIKey( void *pKey, int idx )=0;
		
		virtual bool GetMeshTamperLock()=0;
		virtual void SetMeshTamperLock( bool state )=0;
		virtual bool GetBottomCoverTamperLock()=0;
		virtual void SetBottomCoverTamperLock( bool state )=0;
		
};
// ATSHA-256 Interface
class IATSHA
{
	public:
	
		virtual bool Init( void )=0;
		virtual bool Self_Test( bool verbose )=0;
		virtual u32  Get_Revision(void) = 0;
		
		virtual bool ClearMemory( void )=0;
		virtual bool GetSerialNumber( u64B * )=0;
		virtual bool GetDeviceKey( void *pKey, int Mode, void *Challenge, int SlotNum  )=0;
		virtual bool GetRandomNumber( u8 *pRandom,  int )=0;
		virtual bool WriteSlotEncrypted( int SlotID, u8 *Key )=0;
		virtual bool ReadSlotEncrypted(  int SlotID, void *Key )=0;
		virtual bool GetOTP( u8 *pMem )=0;
		virtual bool CheckMAC( void *pDigSig, void *Challenge )=0;
		virtual bool Erase_CSP( void ) = 0;
};
*/


class ICryptoBSP
{
	public:
		virtual ~ICryptoBSP() {};
		// No longer used methods
		/*virtual bool Open(void)	= 0;
		virtual bool Close(void)= 0;
	 	virtual u32  Get_Revision(void)=0;
    	virtual u64	Timer_Start(void)=0;
		virtual u64 Timer_Stop(u64 timeIn)=0;
		virtual u32 Calc_Period( int freq )=0;
		// Instance Access
		virtual IMAXQ	 *GetMAXQ()   = 0;
		virtual IATSHA	 *GetATSHA()  = 0;
		*/	
		virtual u32  CRC32_Calc(u8 *buffer, u32 size, u32 seed )=0;
		virtual void CRC32_CreateTable( void )=0;
		virtual u32  CRC32_Reflect(u32 ref, char ch)=0;
		
		virtual u32 GetErrorCode(void)=0;
		/*
		**	Static Singleton
		*/
		static ICryptoBSP* GetInstance();
};

