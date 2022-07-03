#ifndef __DEVICEKEYMANAGER_H__
#define __DEVICEKEYMANAGER_H__

#include <hid_stdint.h>
#include <V100_enc_types.h>
#include <HostCryptoMgr.h>
#include <hid_ret_val.h>

#define BSK_ENTROPY_SIZE                ( 14)
#define MAX_CRYPTO_KEY_SIZE                (304)
#define MAX_KEY_SIZE                    (256)

#define IN
#define OUT
#define INOUT


class ClientKeyInfo {

    public:
        u16 slot;
        u16 nkKeyVersion;
        u16 nKeyMode;
        u16 KeyLen;
        u8* KeyVal;
};

class HostKeyManager {

    public:
        HostKeyManager();
       ~HostKeyManager();

    public:
        void     SetDeviceInfo       ( u16 nVendorID, u16 nProductID, u64 nSerialNum );
        void     InitializeKey       ( _V100_ENC_KEY_TYPE type, _V100_ENC_KEY_MODE mode, bool nUseSimpleKeys = false );
        hidres_t GenerateRsaKeyBlock ( u256& ANBIO, u256& ANSOL, ClientKeyInfo& parentKey, ClientKeyInfo& childKey, u8* pOutBlock, uint& iOutBlockLen );
        hidres_t GenerateAesKeyBlock ( u256& ANBIO, u256& ANSOL, ClientKeyInfo& parentKey, ClientKeyInfo& childKey, u8* pOutBlock, uint& iOutBlockLen );
        hidres_t SetAnbio            ( u128 nANBIO );   // SetRandomNumberDevice
        hidres_t GetAnsol            (  );              // GetRandomNumberHost

    private:
        hidres_t do_activate_cmpk   ( u16& nKeyMode );
        hidres_t do_read_key_file   ( u16 nKeySlot, u16& nKeyMode, u16& nKeySize, u8* pKeyVal );
        hidres_t do_get_rand        ( u8* pDstBuff, u16 len );
        hidres_t do_build_key_block ( const u128 pANBIO, const u128 pANSOL, const ClientKeyInfo& keyInfo, u8* const pOutBlock, uint& iOutBlockLen );

    private:
        const char* GetKeySlotStr             ( u16 nKeySlot );
        u32         GetKeySizeForVariableMode ( u16 nKeyMode );
        u16         GetKeySizeFromSlot        ( u16 nSlot, u16 nKeyMode );

    private:
       u128             m_pSID;                     //
       HostCryptoMgr*    m_pCryptoMgr;                // Crypto Manager
       char             m_strOutDir[256];           // Output directory for logs, saved records, etc.
       u128             m_ANBIO;                    //
       u128             m_ANSOL;                    //
};

#endif

