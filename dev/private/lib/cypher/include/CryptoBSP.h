#include "ICryptoBSP.h"
//#include "v300b_atsha.h"
//#include "v300b_maxq.h"
#include "IMemMgr.h"

class CryptoBSP : public ICryptoBSP, public MemoryBase
{
	public:
		
		// methods
		virtual bool Open(void);
		virtual bool Close(void);
	 	virtual u32  Get_Revision(void);
    	virtual u64	Timer_Start(void);
		virtual u64 Timer_Stop(u64 timeIn);
		virtual u32 Calc_Period( int freq );
		// Instance Access
		//virtual IMAXQ	 *GetMAXQ();
		//virtual IATSHA	 *GetATSHA();
			
		virtual u32  CRC32_Calc(u8 *buffer, u32 size, u32 seed );
		virtual void CRC32_CreateTable( void );
		virtual u32  CRC32_Reflect(u32 ref, char ch);
		
		virtual u32 GetErrorCode(void);
		/*
		**	Static Singleton
		*/
private:
		//oATSHA	m_oATSHA;
		//oMAXQ	m_oMAXQ;
};

