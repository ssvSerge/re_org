#include <assert.h>
#include <cstdio>
#include <cstdlib>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <string>
#include <iostream>
#include <sys/mman.h>  
#include <sys/stat.h>  
#include <fcntl.h>
#include <unistd.h>
#include <thread>
#include <algorithm>
#include "AutoHeapBuffer.h"
#include "logging.h"
#include "ICypher.h"
#include "rsa.h"
#include "PrintKey.h"


// From Stackoverflow: https://stackoverflow.com/questions/53365538/how-to-determine-whether-to-use-filesystem-or-experimental-filesystem
#ifndef INCLUDE_STD_FILESYSTEM_EXPERIMENTAL

// Check for feature test macro for <filesystem>
#   if defined(__cpp_lib_filesystem)
#       define INCLUDE_STD_FILESYSTEM_EXPERIMENTAL 0

// Check for feature test macro for <experimental/filesystem>
#   elif defined(__cpp_lib_experimental_filesystem)
#       define INCLUDE_STD_FILESYSTEM_EXPERIMENTAL 1

// We can't check if headers exist...
// Let's assume experimental to be safe
#   elif !defined(__has_include)
#       define INCLUDE_STD_FILESYSTEM_EXPERIMENTAL 1

// Check if the header "<filesystem>" exists
#   elif __has_include(<filesystem>)

// If we're compiling on Visual Studio and are not compiling with C++17, we need to use experimental
#       ifdef _MSC_VER

// Check and include header that defines "_HAS_CXX17"
#           if __has_include(<yvals_core.h>)
#               include <yvals_core.h>

// Check for enabled C++17 support
#               if defined(_HAS_CXX17) && _HAS_CXX17
// We're using C++17, so let's use the normal version
#                   define INCLUDE_STD_FILESYSTEM_EXPERIMENTAL 0
#               endif
#           endif

// If the marco isn't defined yet, that means any of the other VS specific checks failed, so we need to use experimental
#           ifndef INCLUDE_STD_FILESYSTEM_EXPERIMENTAL
#               define INCLUDE_STD_FILESYSTEM_EXPERIMENTAL 1
#           endif

// Not on Visual Studio. Let's use the normal version
#       else // #ifdef _MSC_VER
#           define INCLUDE_STD_FILESYSTEM_EXPERIMENTAL 0
#       endif

// Check if the header "<filesystem>" exists
#   elif __has_include(<experimental/filesystem>)
#       define INCLUDE_STD_FILESYSTEM_EXPERIMENTAL 1

// Fail if neither header is available with a nice error message
#   else
#       error Could not find system header "<filesystem>" or "<experimental/filesystem>"
#   endif

// We priously determined that we need the exprimental version
#   if INCLUDE_STD_FILESYSTEM_EXPERIMENTAL
// Include it
#       include <experimental/filesystem>

// We need the alias from std::experimental::filesystem to std::filesystem
namespace std {
    namespace filesystem = experimental::filesystem;
}

// We have a decent compiler and can use the normal version
#   else
// Include it
#       include <filesystem>
#   endif

#endif // #ifndef INCLUDE_STD_FILESYSTEM_EXPERIMENTAL

// No ZMK/MK: Regenerate ZMK/MK                 -> OK (When called Execute_Get_Status)
// FW Root Key Hard Code W/Header               -> OK
// API change: reset secure element.            -> OK
// SECA cmdline to get different start mode.    -> OK
// Fix the client connect infinite loop         -> OK

#include "ICypher.h"
#include "VirtualSecureElement.h"

constexpr size_t RSA_BITS = 2048;
constexpr uint32_t RSA_EXPONENT = 0x10001;

#define  BLOCK_SIZE		16
#define  ZEROS_SIZE		32

constexpr const char *ProviKeysTag = "ProviKeys";
constexpr const char *ConfigKeysTag = "ConfigKeys";

ISecureElement *ISecureElement::GetInstance()
{
    static VirtualSecureElement vse;
    return &vse;
}

VirtualSecureElement::VirtualSecureElement()
{
    using namespace std;
    namespace fs = filesystem;
#ifdef __linux__
    string master_key_file = "/var/data/local/secure_keys/master_key";
    string FS_key_rand_file = "/var/data/local/secure_keys/nFSKey";
#elif defined _WIN32
    const string master_key_file = R"(C:\ProgramData\Lumidigm\SecKeys\master_key)";
    const string FS_key_rand_file = R"(C:\ProgramData\Lumidigm\SecKeys\nFSKey)";
#endif
    if (!fs::exists(master_key_file) && !fs::exists(FS_key_rand_file))
    {
        fprintf(stdout, "\nMaster key & rand key file doesn't exist!");
        fprintf(stdout, "\nGenerating Master key and nFSKey file...");
        Execute_Reset_Secure_Element();
    }
    m_current_context_slots.reserve(3);   // Hardcoded, currently maximum of 3 slots can be active
}

void VirtualSecureElement::Delete_Buffer(uint8_t **buffer)
{
    if (buffer && *buffer)
    {
        delete[] * buffer;
        *buffer = nullptr;
    }
}
#if ENABLE_DEBUG_MODE
CryptExecStatus VirtualSecureElement::Execute_Select_Debug_Mode(bool enable_static)
{
    debug_mode_flag = enable_static;
    return CryptExecStatus::Successful;
}
#endif

CryptExecStatus VirtualSecureElement::Execute_GetRandomBuffer(uint8_t *out_buffer, uint32_t out_buffer_len)
{
#if ENABLE_DEBUG_MODE
    memset(out_buffer, 0, out_buffer_len);
    return CryptExecStatus::Successful;
#endif

    if (!out_buffer_len)
        return CryptExecStatus::Successful;
    if (!out_buffer)
        return CryptExecStatus::Invalid_Argument;

    using namespace std;
    ifstream::sync_with_stdio(false);
    ifstream rand_in("/dev/urandom", ios::binary);
    rand_in.read((char *)out_buffer, out_buffer_len);
    return CryptExecStatus::Successful;
}

CryptExecStatus VirtualSecureElement::Execute_Select_Key(uint32_t key_slot)
{
    if (key_slot == 19 || key_slot == 20 || key_slot == 22)
    {
        return CryptExecStatus::Not_Supported;
    }

    if (key_slot == m_current_key_info.slot)
    {
        // Key already loaded, return here to reduce latency
        return CryptExecStatus::Successful;
    }

    InvalidateContext();

    if (IsKeyExist(key_slot))
    {
        m_current_key_info.slot = key_slot;
        const auto get_key_success = LocalGetKeySlot(m_current_key_info, &m_current_key_value);
        if (get_key_success != CryptExecStatus::Successful)
        {
            InvalidateContext();
        }
        m_current_context_slots.push_back(m_current_key_info.slot);
        //std::cout << "GetKey OK, Key Value: " <<std::endl;
        //PrintKey(m_current_key_value, m_current_key_info.key_size);
        return get_key_success;
    }
    else
        return CryptExecStatus::Not_Exist;
}

// Helper method to count number of set bits to conform to DUKPT transaction counter standards
uint Bitcount(int n)
{
    uint nCount = 0;
    while (n) {
        n &= (n - 1);
        nCount++;
    }
    return nCount;
}
// Helper method to load dukpt counter, increment, then update value stored in fs
CryptExecStatus VirtualSecureElement::IncActiveDUKPTCounter(uint32_t nSlot)
{
    KeyInfoStructure tmpInfo;
    tmpInfo.slot = nSlot;
    CryptExecStatus get_key_success = LocalGetKeySlot(tmpInfo, &m_dukpt_tc_value);
    if (get_key_success != CryptExecStatus::Successful)
    {
        return get_key_success;
    }
    u32 nCounter = *(u32*)m_dukpt_tc_value;
    do { nCounter++; } while (Bitcount(nCounter) > 10);
    if (nCounter >= 0x1FFFFF)
    {
        // Counter exceeded
        return CryptExecStatus::Execute_Error;
    }

    memcpy(m_dukpt_tc_value, &nCounter, sizeof(uint32_t));
    CryptExecStatus set_key_success = LocalSetKeySlot(tmpInfo, m_dukpt_tc_value);
    if (set_key_success != CryptExecStatus::Successful)
    {
        return get_key_success;
    }
    return CryptExecStatus::Successful;
}
CryptExecStatus VirtualSecureElement::Execute_Select_DUKPT_Keys(DUKPTKeySlots slots)
{
    // Read ipek
    // Read KSN
    // Read TC as number (note: TC algorithm).
    // Set DUKPT Context

    // TODO: Add validation that requested slot(s) actually contains DUKPT key(s)?

    if (std::find(m_current_context_slots.begin(), m_current_context_slots.end(), slots.DUKPT_IPEK_slot) != m_current_context_slots.end()
        && std::find(m_current_context_slots.begin(), m_current_context_slots.end(), slots.DUKPT_KSN_slot) != m_current_context_slots.end()
        && std::find(m_current_context_slots.begin(), m_current_context_slots.end(), slots.DUKPT_TC_slot) != m_current_context_slots.end())
    {
        // If context already loaded but increase TC is true, increment counter and return
        // Otherise context already loaded, return here to reduce latency
        if (slots.increase_transaction_counter)
            return IncActiveDUKPTCounter(slots.DUKPT_TC_slot);
        return CryptExecStatus::Successful;
    }

    InvalidateContext();

    CryptExecStatus get_key_success;
    if (IsKeyExist(slots.DUKPT_IPEK_slot))
    {
        m_current_key_info.slot = slots.DUKPT_IPEK_slot;
        get_key_success = LocalGetKeySlot(m_current_key_info, &m_current_key_value);
        if (get_key_success != CryptExecStatus::Successful)
        {
            InvalidateContext();
            return get_key_success;
        }
        m_current_context_slots.push_back(m_current_key_info.slot);
    }
    else
    {
        InvalidateContext();
        return CryptExecStatus::Not_Exist;
    }

    if (IsKeyExist(slots.DUKPT_KSN_slot))
    {
        KeyInfoStructure tmpInfo;
        tmpInfo.slot = slots.DUKPT_KSN_slot;
        get_key_success = LocalGetKeySlot(tmpInfo, &m_dukpt_ksn_value);
        if (get_key_success != CryptExecStatus::Successful)
        {
            InvalidateContext();
            return get_key_success;
        }
        m_current_context_slots.push_back(tmpInfo.slot);
    }
    else
    {
        InvalidateContext();
        return CryptExecStatus::Not_Exist;
    }

    if (IsKeyExist(slots.DUKPT_TC_slot))
    {
        if (slots.increase_transaction_counter)
        {
            // Note: IncDUKPTCounter will load transaction counter into memory and update FS with incremented value
            get_key_success = IncActiveDUKPTCounter(slots.DUKPT_TC_slot);
            if (get_key_success != CryptExecStatus::Successful)
            {
                InvalidateContext();
                return get_key_success;
            }
        }
        else
        {
            KeyInfoStructure tmpInfo;
            tmpInfo.slot = slots.DUKPT_TC_slot;
            get_key_success = LocalGetKeySlot(tmpInfo, &m_dukpt_tc_value);
            if (get_key_success != CryptExecStatus::Successful)
            {
                InvalidateContext();
                return get_key_success;
            }
        }
        m_current_context_slots.push_back(slots.DUKPT_TC_slot);
    }
    else
    {
        InvalidateContext();
        return CryptExecStatus::Not_Exist;
    }

    return get_key_success;
}

CryptExecStatus VirtualSecureElement::Execute_Select_Public_RSA_Keys(PublicRSAKeySlots slots)
{
    // Read public key
    // Read exp
    // encrypt

	// TODO: Add validation that slot(s) actually contains RSA key(s)?
    if (std::find(m_current_context_slots.begin(), m_current_context_slots.end(), slots.public_key_slot) != m_current_context_slots.end()
        && std::find(m_current_context_slots.begin(), m_current_context_slots.end(), slots.public_key_exp_slot) != m_current_context_slots.end())
    {
        // Context already loaded, return here to reduce latency
        return CryptExecStatus::Successful;
    }

    InvalidateContext();

    CryptExecStatus get_key_success;
    if (IsKeyExist(slots.public_key_slot))
    {
        m_current_key_info.slot = slots.public_key_slot;
        get_key_success = LocalGetKeySlot(m_current_key_info, &m_current_key_value);
        if (get_key_success != CryptExecStatus::Successful)
        {
            InvalidateContext();
            return get_key_success;
        }
        m_current_context_slots.push_back(m_current_key_info.slot);
    }
    else
    {
        InvalidateContext();
        return CryptExecStatus::Not_Exist;
    }

    if (IsKeyExist(slots.public_key_exp_slot))
    {
        KeyInfoStructure tmpInfo;
        tmpInfo.slot = slots.public_key_exp_slot;
        get_key_success = LocalGetKeySlot(tmpInfo, &m_rsa_public_exp);
        if (get_key_success != CryptExecStatus::Successful)
        {
            InvalidateContext();
            return get_key_success;
        }
        m_current_context_slots.push_back(tmpInfo.slot);
    }
    else
    {
        InvalidateContext();
        return CryptExecStatus::Not_Exist;
    }

    return get_key_success;
}

CryptExecStatus VirtualSecureElement::Execute_Encrypt(const uint8_t *in_buffer, uint32_t in_buffer_len,
                                                      uint8_t **encrypted_out_buffer,
                                                      uint32_t &encrypted_out_buffer_len, const uint8_t *iv_buffer,
                                                      uint32_t iv_len)
{
    if (!in_buffer || !in_buffer_len)
        return CryptExecStatus::Invalid_Argument;
    if (!encrypted_out_buffer)
        return CryptExecStatus::Invalid_Argument;

    ICRYPTOAlgo *crypt_algo;
    int n_algo_mode;
    uint32_t block_size;
    switch (m_current_key_info.key_mode)
    {
    case KeyModes::AES_256_CBC:
    {
        block_size = 128 / 8;
        crypt_algo = ICypher::GetInstance()->GetAES();
        n_algo_mode = AES_CBC;
    }
    break;
    case KeyModes::AES_128_CBC:
    {
        block_size = 128 / 8;
        crypt_algo = ICypher::GetInstance()->GetAES();
        n_algo_mode = AES_CBC;
    }
    break;
    case KeyModes::TDES_ABA_ECB:
    case KeyModes::TDES_ABC_ECB:
    {
        block_size = 64 / 8;
        crypt_algo = ICypher::GetInstance()->GetDES();
        n_algo_mode = TDES_ECB;
    }
    break;
    case KeyModes::TDES_ABA_CBC:
    case KeyModes::TDES_ABC_CBC:
    {
        block_size = 64 / 8;
        crypt_algo = ICypher::GetInstance()->GetDES();
        n_algo_mode = TDES_CBC;
    }
    break;
    case KeyModes::DUKPT_KSN_64:
    case KeyModes::DUKPT_IPEK_128:
    {
        if (!m_current_key_value || !m_dukpt_ksn_value || !m_dukpt_tc_value)
        {
            printf("\nDEBUGGING... ERROR! Missing dukpt context(s)\n");
            return CryptExecStatus::Encrypt_Error;
        }

        n_algo_mode = TDES_CBC;
        IDUKPT* pDUKPT = ICypher::GetInstance()->GetDUKPT();
        pDUKPT->SetContext(m_current_key_value, m_dukpt_ksn_value, *(u32*)m_dukpt_tc_value);
        crypt_algo = pDUKPT;
        
        block_size = 64/8;
    }break;
    case KeyModes::RSA_2048_v15: // For variable keyModes slot these are not used
    case KeyModes::RSA_2048_v21:
    {
        if (false == PKIEncrypt(in_buffer, in_buffer_len,encrypted_out_buffer, encrypted_out_buffer_len))
        {
            return CryptExecStatus::Encrypt_Error;
        }
        return CryptExecStatus::Successful;
    }break;
    default:
    {
        fprintf(stdout, "Execute_Decrypt:Invalid key Mode(%u)", (uint32_t)m_current_key_info.key_mode);
        return CryptExecStatus::Invalid_Command;
    }
    }
    if (in_buffer_len % block_size != 0)
    {
        encrypted_out_buffer_len = in_buffer_len + (block_size - (in_buffer_len % block_size));
    }
    else
    {
        encrypted_out_buffer_len = in_buffer_len;
    }
    if (m_current_key_info.key_mode == KeyModes::DUKPT_IPEK_128 || m_current_key_info.key_mode == KeyModes::DUKPT_KSN_64)
    {
        // Take ksn hdr into account
        encrypted_out_buffer_len += sizeof(KSNType);
    }
    *encrypted_out_buffer = new uint8_t[encrypted_out_buffer_len]{};

    uint8_t* iv_writable_buffer = nullptr;
    if (iv_len > 0)
    {
        iv_writable_buffer = new uint8_t[iv_len]{};
        memcpy(iv_writable_buffer, iv_buffer, iv_len);
    }
    const auto encrypt_successful =
        crypt_algo->EncryptData(in_buffer, *encrypted_out_buffer, in_buffer_len, encrypted_out_buffer_len,
            m_current_key_value, m_current_key_info.key_size, iv_writable_buffer, n_algo_mode);

    if (iv_writable_buffer)
    {
        delete[] iv_writable_buffer;
        iv_writable_buffer = nullptr;
    }
    if ( !encrypt_successful)
    {
        printf("\nDEBUGGING... EncryptData unsuccessful\n");
        delete[] * encrypted_out_buffer;
        *encrypted_out_buffer = nullptr;
        return CryptExecStatus::Encrypt_Error;
    }
    return CryptExecStatus::Successful;
}

bool VirtualSecureElement::PKIEncrypt(const u8* in_buffer, u32 in_len, u8** out_buffer, u32& out_bytes)
{
    // Check if key slot is device pub/private key?
    // If not, check m_rsa_public_exp is null or not. Otherwise return false.
    // If null, return false. Otherwise encrypt the data with current_key(rsa public key)

    IRSA* pRSA = ICypher::GetInstance()->GetRSA();
    uint nMode = 0;
    AutoHeapBuffer Auto_rw_buf(in_len);
    u8* in_rw_buf = Auto_rw_buf.u8Ptr();
    memcpy(in_rw_buf, in_buffer, in_len);

    if (m_current_key_info.slot == 17 || m_current_key_info.slot == 18)
    {
        // Device RSA keys
        return false;
    }
    else
    {
        if (!m_current_key_value || !m_rsa_public_exp)
        {
            // RSA key and exponent not loaded
            return false;
        }

        
        if (m_current_key_info.key_mode == KeyModes::RSA_2048_v15)
            nMode = RSA_PKCS_V15;
        else if (m_current_key_info.key_mode == KeyModes::RSA_2048_v21)
            nMode = RSA_PKCS_V21;
        else
            return false;

        if (!pRSA->Init(nMode, m_current_key_info.rsa_sign_mode))
        {
            goto ABORT;
        }
        if (!pRSA->SetRandTestMode(false, NULL))
        {
            goto ABORT;
        }
        if (!pRSA->SetContextPublic(m_current_key_value, m_current_key_info.key_size, *(u32*)m_rsa_public_exp))
        {
            goto ABORT;
        }

        *out_buffer = new uint8_t[m_current_key_info.key_size]{};
        if (!pRSA->Encrypt(NULL, RSA_PUBLIC, in_len, in_rw_buf, *out_buffer))
        {
            goto ABORT;
        }
        out_bytes = m_current_key_info.key_size;
        
        pRSA->Clear();

        return true;
    }

ABORT:
    pRSA->Clear();
    if (*out_buffer)
    {
        memset(*out_buffer, 0, m_current_key_info.key_size);
        delete[] *out_buffer;
        *out_buffer = nullptr;
    }

    return false;
}


CryptExecStatus VirtualSecureElement::Execute_Decrypt(const uint8_t *in_buffer, uint32_t in_buffer_len,
                                                      uint8_t **decrypted_out_buffer,
                                                      uint32_t &decrypted_out_buffer_len, const uint8_t *iv_buffer,
                                                      uint32_t iv_len)
{
    //PrintKey(m_current_key_value, m_current_key_info.key_size);
    if (!in_buffer || !in_buffer_len)
        return CryptExecStatus::Invalid_Argument;
    if (!decrypted_out_buffer)
        return CryptExecStatus::Invalid_Argument;

    ICRYPTOAlgo *crypt_algo;
    int nAlgoMode = 0;
    switch (m_current_key_info.key_mode)
    {
    case KeyModes::AES_256_CBC:
    case KeyModes::AES_128_CBC:
    {
        crypt_algo = ICypher::GetInstance()->GetAES();
        nAlgoMode = AES_CBC;
    }
    break;
    case KeyModes::TDES_ABA_ECB:
    case KeyModes::TDES_ABC_ECB:
    {
        crypt_algo = ICypher::GetInstance()->GetDES();
        nAlgoMode = TDES_ECB;
    }
    break;
    case KeyModes::TDES_ABA_CBC:
    case KeyModes::TDES_ABC_CBC:
    {
        crypt_algo = ICypher::GetInstance()->GetDES();
        nAlgoMode = TDES_CBC;
    }
    break;
    case KeyModes::RSA_2048_v15: // For variable keyModes slot these are not used
    case KeyModes::RSA_2048_v21:
    {
        // If device key, continue otherwise return false
        if (m_current_key_info.slot == 18) // Device private key
        {
            if (false == DeviceRSAKeyDecrypt(in_buffer, in_buffer_len, decrypted_out_buffer, decrypted_out_buffer_len))
            {
                return CryptExecStatus::Decrypt_Error;
            }
            return CryptExecStatus::Successful;
        }
        // Note: Non Device RSA decryption not supported at this time
        return CryptExecStatus::Not_Supported;
    }
    case KeyModes::DUKPT_IPEK_128:
    case KeyModes::DUKPT_KSN_64:
    {
        // NOTE: DUKPT decryption currently not needed for secure fw configurations, partial implementation is here but not validated.
        //  Likely need to modify expected output size to account for KSN header
        return CryptExecStatus::Not_Supported;
        if (!m_current_key_value || !m_dukpt_ksn_value || !m_dukpt_tc_value)
        {
            return CryptExecStatus::Invalid_Command;
        }

        nAlgoMode = TDES_CBC;
        IDUKPT* pDUKPT = ICypher::GetInstance()->GetDUKPT();
        pDUKPT->SetContext(m_current_key_value, m_dukpt_ksn_value, (u32)*m_dukpt_tc_value);
        crypt_algo = pDUKPT;
    }
    default:
    {
        fprintf(stdout, "Execute_Decrypt:Invalid key Mode (%u)", (uint32_t)m_current_key_info.key_mode);
        return CryptExecStatus::Invalid_Command;
    }
    }
    fprintf(stdout, "in_buffer_len = %d", in_buffer_len);
    u8 *decrypt_buffer = new u8[in_buffer_len]{};
    u8* iv_buf_writable = nullptr;
    if (iv_len != 0)
    {
        iv_buf_writable = new u8[iv_len]{};
        memcpy(iv_buf_writable, iv_buffer, iv_len);
    }
    decrypted_out_buffer_len = in_buffer_len;
    const auto decrypt_successful =
        crypt_algo->DecryptData(in_buffer, decrypt_buffer, in_buffer_len, decrypted_out_buffer_len, m_current_key_value,
                                m_current_key_info.key_size, iv_buf_writable, nAlgoMode);
    if (iv_buf_writable != nullptr)
    {
        delete[] iv_buf_writable;
        iv_buf_writable = nullptr;
    }
    if (!decrypt_successful)
    {
        delete[] decrypt_buffer;
        decrypt_buffer = nullptr;
        *decrypted_out_buffer = nullptr;
        return CryptExecStatus::Decrypt_Error;
    }
    fprintf(stdout, "\nDecrypted_Buf = \n");
    //PrintKey(decrypt_buffer, decrypted_out_buffer_len );
    fprintf(stdout, "\n\n");
    *decrypted_out_buffer = new uint8_t[decrypted_out_buffer_len]{};
    memcpy(*decrypted_out_buffer, decrypt_buffer, decrypted_out_buffer_len);
    if (decrypt_buffer)
    {
        delete[] decrypt_buffer;
        decrypt_buffer = nullptr;
    }
    return CryptExecStatus::Successful;
}

bool VirtualSecureElement::DeviceRSAKeyDecrypt(const u8* in_buffer, u32 in_len, u8** out_buffer, u32& out_bytes)
{
    IRSA* pRSA = ICypher::GetInstance()->GetRSA();
    AutoHeapBuffer Auto_Exponent(sizeof(u32));
    u8* Exponent = Auto_Exponent.u8Ptr();
    AutoHeapBuffer Auto_pQ(sizeof(u2048));
    u8* pQ = Auto_pQ.u8Ptr();
    AutoHeapBuffer Auto_pP(sizeof(u2048));
    u8* pP = Auto_pP.u8Ptr();
    KeyInfoStructure kis{};
    AutoHeapBuffer Auto_rw_buf(in_len);
    u8* in_rw_buf = Auto_rw_buf.u8Ptr();
    memcpy(in_rw_buf, in_buffer, in_len);
    uint32_t max_bytes = m_current_key_info.key_size; // RSA Max decryptable bytes = key length.
    size_t out_bytes2 = out_bytes;
    bool ret = false;

    u16 nKeyVer;
    u8  nKCV[KCV_SIZE];
    u32 nKeySize, nExpSize;
    KeyModes KeyMode;
    auto rsa_mode = RSA_PKCS_V15;

    if (in_len > max_bytes)
    {
        goto ABORT;
    }

    if (!in_buffer)
    {
        goto ABORT;
    }
    /*
    **	Restore Device Contex
    */
    kis.slot = 19;
    if (LocalGetKeySlot(kis, &pP) != CryptExecStatus::Successful)
    {
        err("PKIDecryptWithDevicePrivKey:Getting Key %d returned error", 19);
        goto ABORT;
    }

    memset(&kis, 0, sizeof(KeyInfoStructure));
    kis.slot = 20;
    if (LocalGetKeySlot(kis, &pQ) != CryptExecStatus::Successful)
    {
        err("PKIDecryptWithDevicePrivKey:Getting Key %d returned error", 20);
        goto ABORT;
    }
    nKeySize = kis.key_size;
    KeyMode = kis.key_mode;

    if (KeyMode == KeyModes::RSA_2048_v15)
        rsa_mode = RSA_PKCS_V15;
    if (KeyMode == KeyModes::RSA_2048_v21)
        rsa_mode = RSA_PKCS_V21;

    if (!pRSA->Init(rsa_mode, SIG_RSA_SHA1))
    {
        err("PKIDecryptWithDevicePrivKey:Initializing RSA returned error");
        goto ABORT;
    }

    if (!pRSA->SetRandTestMode(false, NULL))		// Ensure Test Mode is Off
    {
        err("PKIDecryptWithDevicePrivKey:Turning RSA test mode off returned error");
        goto ABORT;
    }

    memset(&kis, 0, sizeof(KeyInfoStructure));
    kis.slot = 22;


    if (LocalGetKeySlot(kis, &Exponent) != CryptExecStatus::Successful)
    {
        err("PKIDecryptWithDevicePrivKey:Getting Key %d returned error", 22);
        goto ABORT;
    }
    nExpSize = kis.key_size;

    /*
    **	This reqenerates Keys and restores context
    */
    if (!pRSA->SetContext(pP, pQ, *(u32*)Exponent, (u32)nKeySize))
    {
        err("PKIDecryptWithDevicePrivKey:Setting RSA context returned error");
        goto ABORT;
    }

    memset(pP, 0x00, sizeof(u1024));
    memset(pQ, 0x00, sizeof(u1024));

    // std::cout << "Encrypted buffer: " << std::endl;
    // PrintKey(in_rw_buf, in_len,32);
    // std::cout << std::endl;

    *out_buffer = new uint8_t[max_bytes]{};

    /*
    **	Perform Decryption
    */

    ret = pRSA->Decrypt(RSA_PRIVATE, &out_bytes2, in_rw_buf, *out_buffer, max_bytes);
    out_bytes = (u32)out_bytes2;
    if (!ret)
    {
        err("PKIDecryptWithDevicePrivKey:Decryption step returned error");
        goto ABORT;
    }
    // std::cout <<"\nDecrypted Bytes:" << std::endl;
    // PrintKey(*out_buffer, out_bytes2, 32);
    // std::cout << std::endl;

    /*
    **	Clean UP
    */
    pRSA->Clear();
    return true;

ABORT:
    pRSA->Clear();
    if (*out_buffer)
    {
        memset(*out_buffer, 0, max_bytes);
        delete[] * out_buffer;
        *out_buffer = nullptr;
    }
    return false;

}

CryptExecStatus VirtualSecureElement::Execute_Verify_And_Decrypt(const uint8_t* map_filename, uint32_t map_filename_len,
                                                                 const uint8_t *iv_buffer, uint32_t iv_len)
{
    // Open memory maps.
    int fd = open(kUpdateShmFileName, O_RDWR, 0666);
    struct stat update_file_stat{}, state_stat{};
    if (fd == -1)
    {
        err("\nError: Cannot update mmap file!");
        return CryptExecStatus::Mem_Error;
    }
    if (fstat(fd, &update_file_stat) == -1)
    {
        err("\nError: Cannot stat the update mmap file!");
        return CryptExecStatus::Not_Exist;
    }
    
    uint8_t* mmap_content = (uint8_t*)mmap(nullptr, update_file_stat.st_size, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0 );
    if (mmap_content == (void*)-1)
    {
        err("\nError: mmap failed!");
        return CryptExecStatus::Execute_Error;
    }
    close(fd);

    fd = shm_open(kUpdateStateShmName, O_RDWR, 0666);
    char* state_content = (char*)mmap(nullptr, sizeof(AsyncExecResult), PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);

    std::thread t_worker([=] {
        u512 hash_result;
        int hash_result_len = sizeof(u512);
        AsyncExecResult async_result{};
        uint32_t buffer_len = update_file_stat.st_size;
        uint32_t data_len = buffer_len - sizeof(u512);
        uint8_t* decrypted_buffer = new uint8_t[buffer_len]{};
        uint8_t* iv_rw_buf = new uint8_t[iv_len]{};
        uint8_t* fw_rw_key = new uint8_t[kFwKeyLen]{};
        int validate_result = -1;
        bool operation_ok;
        memcpy(fw_rw_key, kFwKey, kFwKeyLen);
        
        // Run HMAC SHA512
        operation_ok = ICypher::GetInstance()->GetHMAC()->HMAC(mmap_content, buffer_len - sizeof(u512), fw_rw_key, kFwKeyLen, hash_result, &hash_result_len, SHA512_MODE);
        if (!operation_ok)
        {
            memset(mmap_content, 0, update_file_stat.st_size);
            async_result.async_result = AsyncExecStates::Exec_Fail;
            async_result.exec_result = CryptExecStatus::Hash_Error;
            goto QUIT_AND_CLEANUP;
        }
        // Validate HMAC value
        validate_result = memcmp(mmap_content + (buffer_len - sizeof(u512)), hash_result, hash_result_len);
        if (validate_result != 0)
        {
            memset(mmap_content, 0, update_file_stat.st_size);
            async_result.async_result = AsyncExecStates::Exec_Fail;
            async_result.exec_result = CryptExecStatus::Validation_Error;
            goto QUIT_AND_CLEANUP;
        }

        memcpy(iv_rw_buf, iv_buffer, iv_len);

        operation_ok = ICypher::GetInstance()->GetAES()->DecryptData(mmap_content, decrypted_buffer, data_len, data_len, kFwKey, kFwKeyLen, iv_rw_buf, AES_CBC);

        if (!operation_ok)
        {
            memset(mmap_content, 0, buffer_len);
            async_result.async_result = AsyncExecStates::Exec_Fail;
            async_result.exec_result = CryptExecStatus::Decrypt_Error;
            goto QUIT_AND_CLEANUP;
        }

        // AES decrypt doesn't change the buffer length, so no mremap needed.
        memcpy(mmap_content, decrypted_buffer, buffer_len);
        async_result.async_result = AsyncExecStates::Exec_OK;
        async_result.exec_result = CryptExecStatus::Successful;

    QUIT_AND_CLEANUP:
        delete[] fw_rw_key;
        fw_rw_key = nullptr;
        if (decrypted_buffer)
        {
            delete[] decrypted_buffer;
            decrypted_buffer = nullptr;
        }
        if (iv_rw_buf)
        {
            delete[] iv_rw_buf;
            iv_rw_buf = nullptr;
        }
        //close(fd);
        memcpy(state_content, &async_result, sizeof(AsyncExecResult));
        munmap(mmap_content, buffer_len);
        munmap(state_content, sizeof(AsyncExecResult));
        return;
        });
        // End of thread.
        

    t_worker.detach();
    return CryptExecStatus::Successful;
}

CryptExecStatus VirtualSecureElement::Execute_Hash_Data(const uint8_t *in_buffer, uint32_t in_buffer_len,
                                                        HashAlgorithms hash_algorithm, uint8_t **out_hash_result,
                                                        uint32_t &out_hash_result_length)
{
    if (!in_buffer || !out_hash_result)
        return CryptExecStatus::Invalid_Argument;

    out_hash_result_length = 0;
    bool success = false;
    switch (hash_algorithm)
    {
    case HashAlgorithms::HMAC_SHA512:
        return CryptExecStatus::Not_Supported;
    default:
        break;
    }
    if (hash_algorithm == HashAlgorithms::SHA1)
    {
        auto pSHA = ICypher::GetInstance()->GetSHA1();
        out_hash_result_length = 160 / 8;
        *out_hash_result = new u8[out_hash_result_length]{};
        success = pSHA->Hash(in_buffer, in_buffer_len, *out_hash_result);
    }
    else if (hash_algorithm == HashAlgorithms::SHA2_256 || hash_algorithm == HashAlgorithms::SHA2_224)
    {
        auto pSHA = ICypher::GetInstance()->GetSHA256();
        out_hash_result_length = 256 / 8;
        *out_hash_result = new u8[out_hash_result_length]{};
        success = pSHA->Hash(in_buffer, in_buffer_len, *out_hash_result,
                             (hash_algorithm == HashAlgorithms::SHA2_224 ? true : false));
    }
    else if (hash_algorithm == HashAlgorithms::SHA2_512 || hash_algorithm == HashAlgorithms::SHA2_384)
    {
        auto pSHA = ICypher::GetInstance()->GetSHA512();
        out_hash_result_length = 512 / 8;
        *out_hash_result = new u8[out_hash_result_length]{};
        success = pSHA->Hash(in_buffer, in_buffer_len, *out_hash_result,
                             (hash_algorithm == HashAlgorithms::SHA2_384 ? true : false));
    }
    else if (hash_algorithm == HashAlgorithms::HMAC_SHA1)
    {
        auto pHMAC = ICypher::GetInstance()->GetHMAC();
        u160 HMACResult{};
        int MACSize = sizeof(u160);
        success = pHMAC->HMAC(const_cast<uint8_t *>(in_buffer), in_buffer_len, m_current_key_value,
                              m_current_key_info.key_size, HMACResult, &MACSize, SHA1_MODE);
        out_hash_result_length = sizeof(u160);
        *out_hash_result = new uint8_t[out_hash_result_length]{};
        memcpy(*out_hash_result, &HMACResult, out_hash_result_length);
    }
    else if (hash_algorithm == HashAlgorithms::HMAC_SHA256)
    {
        auto pHMAC = ICypher::GetInstance()->GetHMAC();
        u256 HMACResult{};
        int MACSize = sizeof(u256);
        success = pHMAC->HMAC(const_cast<uint8_t *>(in_buffer), in_buffer_len, m_current_key_value,
                              m_current_key_info.key_size, HMACResult, &MACSize, SHA256_MODE);
        out_hash_result_length = sizeof(u256);
        *out_hash_result = new uint8_t[out_hash_result_length]{};
        memcpy(*out_hash_result, &HMACResult, out_hash_result_length);
    }
    else if (hash_algorithm == HashAlgorithms::HMAC_SHA512)
    {
        auto pHMAC = ICypher::GetInstance()->GetHMAC();
        u512 HMACResult {};
        int MACSize = sizeof(u512);
        success = success = pHMAC->HMAC(const_cast<uint8_t *>(in_buffer), in_buffer_len, m_current_key_value,
                              m_current_key_info.key_size, HMACResult, &MACSize, SHA512_MODE);
        out_hash_result_length = sizeof(u512);
        *out_hash_result = new uint8_t[out_hash_result_length]{};
        memcpy(*out_hash_result, &HMACResult, out_hash_result_length);
    }
    else
        return CryptExecStatus::Not_Supported;

    if (!success)
    {
        return CryptExecStatus::Execute_Error;
    }
    else
    {
        return CryptExecStatus::Successful;
    }
}

CryptExecStatus VirtualSecureElement::Execute_Generate_RSA_Key()
{
    using namespace std;

    uint32_t key_size = 0;
    AsymmetricKeyFormat asm_key, asm_temp_key;
    KeyInfoStructure kvs;
    CryptExecStatus exec_status = CryptExecStatus::Execute_Error;
    u1024 p{}, q{}, exp_not_needed{};
    auto pRSA = ICypher::GetInstance()->GetRSA();
    auto op_successuful = pRSA->Generate_Keypair(nullptr, RSA_BITS, RSA_EXPONENT);
    fprintf(stdout, "\n Generate RSA Keypair success");
    if (!op_successuful)
    {
        fprintf(stdout, "Gen_RSA_KEY Failed!");
        pRSA->Clear();
        return CryptExecStatus::Execute_Error;
    }

    // Get public key
    op_successuful = pRSA->GetPublicKey(asm_key.key, &key_size);
    fprintf(stdout, "\nGetPubKey OK");
    if (!op_successuful)
    {
        fprintf(stdout, "GetPublicKey Failed!");
        pRSA->Clear();
        return CryptExecStatus::Execute_Error;
    }

    // cout << "Public Key :" << endl;
    // PrintKey(asm_key.key, key_size, 16);
    // cout << "======================" << endl;

    op_successuful = CreateKCV(asm_key.key, key_size, ZEROS_SIZE, KCV_SIZE, kvs.key_check_value, KCV_SHA_256_NONE);
    fprintf(stdout, "\nCreate KCV OK");
    if (!op_successuful)
    {
        fprintf(stdout, "CreateKCV (PrivKey) Failed!");
        pRSA->Clear();
        return CryptExecStatus::Execute_Error;
    }

    // DEVICE_PUBLIC = 17,
    // DEVICE_PRIVATE = 18,
    // DEVICE_P = 19,
    // DEVICE_Q = 20,
    // DEVICE_EXP = 22

    kvs.slot = 17;
    kvs.key_ver = 0xAAAA;
    kvs.key_mode = KeyModes::RSA_2048_v15;
    kvs.rsa_sign_mode = SIG_RSA_SHA1;
    kvs.key_size = key_size;
    exec_status = LocalSetKeySlot(kvs, asm_key.key);
    memset(kvs.key_check_value, 0, 4);

    if (exec_status != CryptExecStatus::Successful)
    {
        fprintf(stdout, "SetKey (PubKey) failed!");
        pRSA->Clear();
        return exec_status;
    }

    // validate public key
    uint8_t *key = nullptr;
    exec_status = Execute_Get_RSA_PublicKey(&key, key_size);
    fprintf(stdout, "\nValidate PubKey");
    if (key)
    {
        if (exec_status == CryptExecStatus::Successful)
            memcpy(asm_temp_key.key, key, key_size);
        delete[] key;
    }
    if (exec_status != CryptExecStatus::Successful)
    {
        fprintf(stdout, "Get public key for testing failed!");
        pRSA->Clear();
        return exec_status;
    }

    if (memcmp(asm_key.key, asm_temp_key.key, key_size) != 0)
    {
        fprintf(stdout, "Validation of already set key failed!");
        pRSA->Clear();
        return CryptExecStatus::Execute_Error;
    }
    fprintf(stdout, "\nValidate PubKey OK");

    memset(&asm_key, 0, sizeof(AsymmetricKeyFormat));
    key_size = 0;
    // Get private key
    op_successuful = pRSA->GetPrivateKey(asm_key.key, &key_size);
    if (!op_successuful)
    {
        fprintf(stdout, "GetPrivateKey Failed!");
        pRSA->Clear();
        return CryptExecStatus::Execute_Error;
    }

    // fprintf(stdout, "Private Key: \n");
    // PrintKey(asm_key.key, key_size, 32);
    // fprintf(stdout, "\n");

    op_successuful = CreateKCV(asm_key.key, key_size, ZEROS_SIZE, KCV_SIZE, kvs.key_check_value, KCV_SHA_256_NONE);
    if (!op_successuful)
    {
        fprintf(stdout, "CreateKCV (PrivKey) Failed!");
        pRSA->Clear();
        return CryptExecStatus::Execute_Error;
    }
    kvs.slot = 18;
    kvs.key_ver = 0xAAAA;
    kvs.key_mode = KeyModes::RSA_2048_v15;
    kvs.key_size = key_size;
    exec_status = LocalSetKeySlot(kvs, asm_key.key);
    if (exec_status != CryptExecStatus::Successful)
    {
        fprintf(stdout, "SetKey (PrivKey) failed!");
        pRSA->Clear();
        return exec_status;
    }

    memset(&asm_key, 0, sizeof(AsymmetricKeyFormat));
    key_size = sizeof(u1024);
    // Get Primes
    op_successuful = pRSA->GetPrimes(p, q, (u32 *)exp_not_needed, sizeof(u1024));
    if (!op_successuful)
    {
        fprintf(stdout, "GetPrimes Failed!");
        pRSA->Clear();
        return CryptExecStatus::Execute_Error;
    }

    // fprintf(stdout, "P: \n");
    // PrintKey(p, sizeof(u1024), 32);
    // fprintf(stdout, "\n");
    
    // fprintf(stdout, "Q: \n");
    // PrintKey(q, sizeof(u1024), 32);
    // fprintf(stdout, "\n");
    
    // fprintf(stdout, "Exp: \n");
    // PrintKey((uint8_t*)&RSA_EXPONENT, sizeof(uint32_t), 32);
    // fprintf(stdout, "\n");

    kvs.slot = 19;
    kvs.key_ver = 0xAAAA;
    kvs.key_mode = KeyModes::RSA_2048_v15;
    kvs.key_size = sizeof(u1024);
    memset(kvs.key_check_value, 0, sizeof(uint8_t) * 4);
    exec_status = LocalSetKeySlot(kvs, p);
    if (exec_status != CryptExecStatus::Successful)
    {
        fprintf(stdout, "SetKey (P) failed!");
        pRSA->Clear();
        return exec_status;
    }

    kvs.slot = 20;
    exec_status = LocalSetKeySlot(kvs, q);
    if (exec_status != CryptExecStatus::Successful)
    {
        fprintf(stdout, "SetKey (Q) failed!");
        pRSA->Clear();
        return exec_status;
    }

    kvs.slot = 22;
    kvs.key_size = sizeof(uint32_t);
    exec_status = LocalSetKeySlot(kvs, (u8 *)&RSA_EXPONENT);
    if (exec_status != CryptExecStatus::Successful)
    {
        fprintf(stdout, "SetKey (Exp) failed!");
        pRSA->Clear();
        return exec_status;
    }
    pRSA->Clear();
    Execute_Select_Key(17);

    return CryptExecStatus::Successful;
}

CryptExecStatus VirtualSecureElement::Execute_Reset_Secure_Element()
{
    using namespace std;
    namespace fs = std::filesystem;

    InvalidateContext();

#ifdef __linux__
    string master_key_file = "/var/data/local/secure_keys/master_key";
    string FS_key_rand_file = "/var/data/local/secure_keys/nFSKey";
#elif defined _WIN32
    string master_key_file = R"(C:\ProgramData\Lumidigm\SecKeys\master_key)";
    string FS_key_rand_file = R"(C:\ProgramData\Lumidigm\SecKeys\nFSKey)";
#endif

    fs::path master_key_filename_path(master_key_file);
    fs::path rng_fs_path(FS_key_rand_file);
#if INCLUDE_STD_FILESYSTEM_EXPERIMENTAL
    if (!fs::exists(master_key_filename_path.parent_path()))
#else
    if (!fs::directory_entry(master_key_filename_path.parent_path()).exists())
#endif        
    {
        fs::create_directories(fs::directory_entry(master_key_filename_path.parent_path()));
    }
    
    ofstream master_key_writer(master_key_filename_path, ios::binary);
    ofstream nFS_writer(rng_fs_path, ios::binary);
    uint8_t rand_buffer_mk[32]{}, rand_buffer_fsk[32]{};
    uint32_t rand_buffer_len = 32;
    Execute_GetRandomBuffer(rand_buffer_mk, rand_buffer_len);
    master_key_writer.write((char *)rand_buffer_mk, rand_buffer_len);

    Execute_GetRandomBuffer(rand_buffer_fsk, rand_buffer_len);
    nFS_writer.write((char *)rand_buffer_fsk, rand_buffer_len);

    uint8_t out_enc_fsk[32]{}, *out_fsk_hash = nullptr;
    uint32_t out_fsk_hash_len = 0;
    ICypher::GetInstance()->GetAES()->Encrypt(rand_buffer_fsk, out_enc_fsk, sizeof(u256), rand_buffer_mk, 32, nullptr, AES_CBC );
    Execute_Hash_Data(out_enc_fsk, 32, HashAlgorithms::SHA2_512, &out_fsk_hash, out_fsk_hash_len);
    nFS_writer.write((char*)out_fsk_hash, out_fsk_hash_len);
    nFS_writer.flush();
    
    master_key_writer.flush();
    nFS_writer.flush();
    if (out_fsk_hash)
    {
        delete[] out_fsk_hash;
        out_fsk_hash = nullptr;
    }

    return CryptExecStatus::Successful;
}

CryptExecStatus VirtualSecureElement::Execute_Get_RSA_PublicKey(uint8_t **buffer, uint32_t &buffer_len)
{
    if(!buffer)
        return CryptExecStatus::Invalid_Argument;

    KeyInfoStructure key_info;
    key_info.slot = 17;
    const auto read_success = LocalGetKeySlot(key_info, buffer);
    if (read_success == CryptExecStatus::Successful)
    {
        buffer_len = key_info.key_size;
        assert(key_info.key_size > 0);
    }
    else
    {
        *buffer = nullptr;
        buffer_len = 0;
    }
    return read_success;
}

CryptExecStatus VirtualSecureElement::Execute_Get_Key_Info(uint32_t key_slot, KeyInfoStructure &out_key_info)
{
    KeyInfoStructure target_key_info;
    u8* dummy_key_value = nullptr;

    if (key_slot == 19 || key_slot == 20 || key_slot == 22)
    {
        memset(&out_key_info, 0, sizeof(KeyInfoStructure));
        return CryptExecStatus::Not_Supported;
    }

    if (IsKeyExist(key_slot))
    {
        out_key_info.slot = key_slot;
        const auto get_key_success = LocalGetKeySlot(out_key_info, &dummy_key_value);
        if (get_key_success != CryptExecStatus::Successful)
        {
            memset(&out_key_info, 0, sizeof(KeyInfoStructure));
        }
        if (dummy_key_value)
        {
            delete[] dummy_key_value;
            dummy_key_value = nullptr;
        }
        return get_key_success;
    }
    else
    {
        memset(&out_key_info, 0, sizeof(KeyInfoStructure));
        return CryptExecStatus::Not_Exist;
    }
}


CryptExecStatus VirtualSecureElement::Execute_Set_Key(const KeyInfoStructure &key_info, const uint8_t *in_key_buffer,
                                                      uint32_t in_key_buffer_len)
{
    assert(in_key_buffer_len >= key_info.key_size);
    if (in_key_buffer_len < key_info.key_size)
    {
        fprintf(stdout, "SetKey:Initial input arguments validation failed");
        return CryptExecStatus::Invalid_Command;
    }

    if (in_key_buffer == nullptr || key_info.key_size == 0)
    {
        fprintf(stdout, "SetKey:Initial input arguments validation failed");
        return CryptExecStatus::Invalid_Command;
    }

    switch (key_info.key_mode)
    {
    case KeyModes::MODE_NONE:
    case KeyModes::AES_256_CBC:
    case KeyModes::AES_128_CBC:
    case KeyModes::TDES_ABA_ECB:
    case KeyModes::TDES_ABA_CBC:
    case KeyModes::TDES_ABC_ECB:
    case KeyModes::TDES_ABC_CBC:
    case KeyModes::RSA_2048_v15:
    case KeyModes::RSA_2048_v21:
    case KeyModes::DUKPT_IPEK_128:
    case KeyModes::DUKPT_KSN_64:
        break;
    default:
        return CryptExecStatus::Invalid_Argument;
    }

    std::cout << "Set Key Value: " <<std::endl;
    //PrintKey(in_key_buffer, key_info.key_size);
    std::cout<<std::endl;

    const auto exec_set_successful = LocalSetKeySlot(key_info, in_key_buffer);
    if (exec_set_successful != CryptExecStatus::Successful)
    {
        fprintf(stdout, "SetKey:Setting Key %d returned error", key_info.slot);
        return CryptExecStatus::Execute_Error;
    }

    // If set key is in active context, invalidate context otherwise we have stale values
    if (std::find(m_current_context_slots.begin(), m_current_context_slots.end(), key_info.slot) != m_current_context_slots.end())
    {
        InvalidateContext();
    }

    return CryptExecStatus::Successful;
}

CryptExecStatus VirtualSecureElement::Execute_Erase_Key(uint32_t key_slot)
{
    const auto success = EraseKeySlot(key_slot);
    if (!success)
    {
        return CryptExecStatus::Execute_Error;
    }
    if (std::find(m_current_context_slots.begin(), m_current_context_slots.end(), key_slot) != m_current_context_slots.end())
    {
        // Erased key is in active context, invalidate context
        InvalidateContext();
    }
    return CryptExecStatus::Successful;
}

CryptExecStatus VirtualSecureElement::Execute_Set_Property(const uint8_t *key, uint32_t key_length,
                                                           const uint8_t *value, uint32_t value_length)
{
    if (!key || !key_length)
        return CryptExecStatus::Invalid_Argument;
    if (!value || !value_length)
        return CryptExecStatus::Invalid_Argument;

    using namespace std;
    namespace fs = std::filesystem;
    string file_name = GetHashStrFromBuffer(key, key_length);
#ifdef __linux__
    const string dir_path("/var/data/local/sec_data_stor/");
#elif defined _WIN32
    const string dir_path(R"(C:\ProgramData\Lumidigm\SecDataStor\)");
#endif
#if INCLUDE_STD_FILESYSTEM_EXPERIMENTAL
    if (!fs::exists(dir_path))
#else
    if (!fs::directory_entry(dir_path).exists())
#endif
    {
        fs::create_directories(dir_path);
    }
    file_name = dir_path + file_name;

    // Check Current Key
    // if (m_current_key_info.key_size == 0)
    // {
    //     fprintf(stdout, "\nError: no key to encrypt is selected!");
    //     return CryptExecStatus::Invalid_Command;
    // }
    u256 FS_key {};
    auto success = GetFSKey(FS_key);
    if (!success)
    {
        return CryptExecStatus::Execute_Error;
    }

    // Hash the value
    u256 value_hash{};
    ICypher::GetInstance()->GetSHA256()->Hash(value, value_length, value_hash);

    // Pad and encrypt
    uint32_t padded_data_size = 0, encrypted_data_size = 0;
    uint8_t *padded_data = nullptr;
    uint8_t *encrypted_data = nullptr;
    bool encrypt_success = false;

    padded_data_size = sizeof(uint32_t) + sizeof(u256) + value_length;
    if (padded_data_size % BLOCK_SIZE) // If nKeySize is not aligned to BLOCK_SIZE
    {
        padded_data_size =
            padded_data_size + (BLOCK_SIZE - padded_data_size % BLOCK_SIZE); // Padded to AES-256 blocksize
    }

    padded_data = new uint8_t[padded_data_size]{};

    memcpy(padded_data, &value_length, sizeof(uint32_t));
    memcpy(padded_data + sizeof(uint32_t), value_hash, sizeof(u256));
    memcpy(padded_data + sizeof(uint32_t) + sizeof(u256), value, value_length);

    encrypt_success = Encrypt(FS_key, sizeof(u256), (ushort)KeyModes::AES_256_CBC,
                              padded_data, padded_data_size, &encrypted_data, encrypted_data_size);
    if (!encrypt_success)
    {
        fprintf(stdout, "\nEncrypt value failed. Cannot storage.");
        delete[] padded_data;
        padded_data = nullptr;
        if (encrypted_data)
        {
            delete[] encrypted_data;
            encrypted_data = nullptr;
        }
        return CryptExecStatus::Execute_Error;
    }

    ofstream writer(file_name, ios::binary);
    writer.write((char *)encrypted_data, encrypted_data_size);
    if (writer.bad())
    {
        fprintf(stdout, "\nWrite file failed. ofstream cannot write file. ");
        delete[] padded_data;
        padded_data = nullptr;
        if (encrypted_data)
        {
            delete[] encrypted_data;
            encrypted_data = nullptr;
        }
        return CryptExecStatus::Execute_Error;
    }
    writer.flush();
    if (writer.bad())
    {
        fprintf(stdout, "\nWrite file flush failed. ofstream cannot write file. ");
        delete[] padded_data;
        padded_data = nullptr;
        if (encrypted_data)
        {
            delete[] encrypted_data;
            encrypted_data = nullptr;
        }
        return CryptExecStatus::Execute_Error;
    }
    if (padded_data)
    {
        delete[] padded_data;
        padded_data = nullptr;
    }
    if (encrypted_data)
    {
        delete[] encrypted_data;
        encrypted_data = nullptr;
    }
    return CryptExecStatus::Successful;
}

CryptExecStatus VirtualSecureElement::Execute_Get_Property(const uint8_t *key, uint32_t key_length, uint8_t **value,
                                                           uint32_t &value_length)
{
    if (!key || !key_length)
        return CryptExecStatus::Invalid_Argument;
    if (!value)
        return CryptExecStatus::Invalid_Argument;

    using namespace std;
    namespace fs = std::filesystem;
    string file_name = GetHashStrFromBuffer(key, key_length);
#ifdef __linux__
    const string dir_path("/var/data/local/sec_data_stor/");
#elif defined _WIN32
    const string dir_path(R"(C:\ProgramData\Lumidigm\SecDataStor\)");
#endif
#if INCLUDE_STD_FILESYSTEM_EXPERIMENTAL
    if (!fs::exists(dir_path))
#else
    if (!fs::directory_entry(dir_path).exists())
    #endif
    {
        fs::create_directories(dir_path);
    }
    file_name = dir_path + file_name;
    // Check if the property file exists
    if (!fs::exists(file_name))
    {
        fprintf(stdout, "\nError: property doesn't exist!");
        return CryptExecStatus::Not_Exist;
    }

    // Check CurrAlso, ent Key
    // if (m_current_key_info.key_size == 0)
    // {
    //     fprintf(stdout, "\nError: no key to decrypt the property is selected!");
    //     return CryptExecStatus::Invalid_Command;
    // }
    u256 FS_Key {};

    auto success = GetFSKey(FS_Key);
    if (!success)
    {
        return CryptExecStatus::Execute_Error;
    }

    uint32_t padded_data_size = 0, encrypted_data_size = 0;
    uint8_t *padded_data = nullptr;
    uint8_t *encrypted_data = nullptr;
    bool decrypt_success = false;
    u256 value_hash;
    u256 tmp_value_hash;

    ifstream reader(file_name, ios::binary);
    reader.seekg(0, ios::end);
    encrypted_data_size = reader.tellg();
    reader.seekg(0, ios::beg);
    encrypted_data = new uint8_t[encrypted_data_size];
    reader.read((char *)encrypted_data, encrypted_data_size);
    padded_data = new uint8_t[encrypted_data_size]{};
    padded_data_size = encrypted_data_size;

    // Decrypt the value
    decrypt_success = Decrypt(FS_Key, sizeof(u256), (u16)KeyModes::AES_256_CBC,
                              encrypted_data, encrypted_data_size, padded_data, &padded_data_size);
    if (!decrypt_success)
    {
        value_length = 0;
        delete[] padded_data;
        padded_data_size = 0;
        delete[] encrypted_data;
        encrypted_data = nullptr;
        return CryptExecStatus::Decrypt_Error;
    }

    delete[] encrypted_data;
    encrypted_data = nullptr;
    memcpy(&value_length, padded_data, sizeof(uint32_t));
    *value = new uint8_t[value_length]{};
    memcpy(&value_hash, padded_data + sizeof(uint32_t), sizeof(u256));
    memcpy(*value, padded_data + sizeof(uint32_t) + sizeof(u256), value_length);

    // Validate value hash
    ICypher::GetInstance()->GetSHA256()->Hash(*value, value_length, tmp_value_hash);
    if (memcmp(tmp_value_hash, value_hash, sizeof(u256)) != 0)
    {
        value_length = 0;
        delete[] * value;
        *value = nullptr;
        delete[] padded_data;
        padded_data = nullptr;
        return CryptExecStatus::Hash_Error;
    }
    if (padded_data != nullptr)
    {
        delete[] padded_data;
        padded_data = nullptr;
    }
    return CryptExecStatus::Successful;
}

CryptExecStatus VirtualSecureElement::Execute_Remove_Property(const uint8_t *key, uint32_t key_length)
{
    if(!key || !key_length)
        return CryptExecStatus::Invalid_Argument;

    using namespace std;
    namespace fs = std::filesystem;
    string file_name = GetHashStrFromBuffer(key, key_length);
#ifdef __linux__
    const string dir_path("/var/data/local/sec_data_stor/");
#elif defined _WIN32
    const string dir_path(R"(C:\ProgramData\Lumidigm\SecDataStor\)");
#endif
#if INCLUDE_STD_FILESYSTEM_EXPERIMENTAL
    if (!fs::exists(dir_path))
#else
    if (!fs::directory_entry(dir_path).exists())
#endif
    {
        fs::create_directories(dir_path);
    }
    file_name = dir_path + file_name;
    // Check if the property file exists
    if (!fs::exists(file_name))
    {
        fprintf(stdout, "\nError: property doesn't exist!");
        return CryptExecStatus::Not_Exist;
    }

    const auto remove_success = fs::remove(file_name);
    if (!remove_success)
    {
        fprintf(stdout, "\nError: cannot remove this file!");
        return CryptExecStatus::Execute_Error;
    }
    return CryptExecStatus::Successful;
}

CryptExecStatus VirtualSecureElement::Execute_Get_Status()
{
    using namespace std;
    namespace fs = filesystem;
#ifdef __linux__
    string master_key_file = "/var/data/local/secure_keys/master_key";
    string FS_key_rand_file = "/var/data/local/secure_keys/nFSKey";
#elif defined _WIN32
    const string master_key_file = R"(C:\ProgramData\Lumidigm\SecKeys\master_key)";
    const string FS_key_rand_file = R"(C:\ProgramData\Lumidigm\SecKeys\nFSKey)";
#endif
    if (!fs::exists(master_key_file) && fs::exists(FS_key_rand_file))
    {
        fprintf(stdout, "\nMaster key doesn't exist! Tamper!");
        return CryptExecStatus::Tamper;
    }
    if (!fs::exists(master_key_file) && !fs::exists(FS_key_rand_file))
    {
        fprintf(stdout, "\nMaster key & rand key file doesn't exist! Not configured!");
        fprintf(stdout, "\nGenerating Master key and nFSKey file...");
        //Execute_Reset_Secure_Element();
        return CryptExecStatus::Not_Configured;
    }
    if (fs::exists(master_key_file) && !fs::exists(FS_key_rand_file))
    {
        fprintf(stdout, "\nRand key file doesn't exist! Not configured!");
        return CryptExecStatus::Not_Configured;
    }
    return CryptExecStatus::Successful;
}

std::string VirtualSecureElement::GetHashStrFromBuffer(const uint8_t *buffer, uint32_t buffer_len)
{
    using namespace std;
    stringstream str_buf;
    auto hash_algo = ICypher::GetInstance()->GetSHA256();
    u256 hash_buf{};
    hash_algo->Hash(buffer, buffer_len, hash_buf);
    for (int index = 0; index < sizeof(u256); index++)
    {
        str_buf << setw(2) << setfill('0') << hex << hash_buf[index];
    }
    return str_buf.str();
}

CryptExecStatus VirtualSecureElement::LocalSetKeySlot(const KeyInfoStructure &key_info, const u8 *key_value)
{
    u32 key_blob_size = 0;
    u32 padded_key_blob_size = 0; // nKBSize + padding
    u8 *key_data = nullptr;
    u8 *encrypted_key_data = nullptr;
    
    KeyInfoStructure internal_key_info = key_info;

    if (key_value == nullptr)
    {
        fprintf(stdout, "SetFSKey:NULL input Key buffer");
        return CryptExecStatus::Execute_Error;
    }

    /**
     * Clear data contains:
     * [KeyInfoStructure][Key]
     * padded_key_blob_size = (sizeof(KeyInfoStructure) + key_size) align to BLOCK_SIZE
     */
    key_blob_size = key_info.key_size + sizeof(KeyInfoStructure);
    if (key_blob_size % BLOCK_SIZE) // If nKeySize is not aligned to BLOCK_SIZE
    {
        padded_key_blob_size = key_blob_size + (BLOCK_SIZE - key_blob_size % BLOCK_SIZE); // Padded to AES-256 blocksize
    }
    else
    {
        padded_key_blob_size = key_blob_size;
    }
    key_data = (u8 *)malloc(padded_key_blob_size);

    if (key_data == nullptr)
    {
        fprintf(stdout, "SetFSKey:Failed to allocate memory");
        return CryptExecStatus::Execute_Error;
    }
    // Calculate key hash (SHA-512)
    ICypher::GetInstance()->GetSHA512()->Hash(key_value, key_info.key_size, internal_key_info.key_hash);

    // Copy the data into memory
    memset(key_data, 0, padded_key_blob_size);
    memcpy(key_data, &internal_key_info, sizeof(KeyInfoStructure));
    memcpy(key_data + sizeof(KeyInfoStructure), key_value, internal_key_info.key_size);

    u256 FSCryptoKey;

    // Derive key_FS
    const auto derive_fs_key_success = GetFSKey(FSCryptoKey);
    if (!derive_fs_key_success)
    {
        fprintf(stdout, "\nGetFSKey: derive FS Key failed.");
        return CryptExecStatus::Execute_Error;
    }

    // Allocate CG buffer for Encryption
    encrypted_key_data = (u8 *)malloc(padded_key_blob_size);
    if (encrypted_key_data == nullptr)
    {
        fprintf(stdout, "SetFSKey:Failed to allocate memory");
        return CryptExecStatus::Mem_Error;
    }

    // Encrypt data with AES-256
    if (!ICypher::GetInstance()->GetAES()->EncryptData(key_data, encrypted_key_data, padded_key_blob_size,
                                                       padded_key_blob_size, FSCryptoKey, sizeof(u256), nullptr,
                                                       AES_CBC))
    {
        free(key_data);
        free(encrypted_key_data);
        fprintf(stdout, "SetFSKey:EncryptData step returned error");
        return CryptExecStatus::Encrypt_Error;
    }
    else
    {
        if (!WriteKeySlot(encrypted_key_data, padded_key_blob_size, internal_key_info.slot))
        {
            free(key_data);
            free(encrypted_key_data);
            fprintf(stdout, "SetFSKey:Writing Key %d returned error", internal_key_info.slot);
            return CryptExecStatus::Execute_Error;
        }
    }

    memset(key_data, 0, padded_key_blob_size);
    memset(encrypted_key_data, 0, padded_key_blob_size);
    free(key_data);
    free(encrypted_key_data);
    return CryptExecStatus::Successful;
}

void VirtualSecureElement::InvalidateContext()
{
    if (m_current_key_value)
    {
        delete[] m_current_key_value;
        m_current_key_value = nullptr;
    }
    if (m_dukpt_ksn_value)
    {
        delete[] m_dukpt_ksn_value;
        m_dukpt_ksn_value = nullptr;
    }
    if (m_dukpt_tc_value)
    {
        delete[] m_dukpt_tc_value;
        m_dukpt_tc_value = nullptr;
    }
    if (m_rsa_public_exp)
    {
        delete[] m_rsa_public_exp;
        m_rsa_public_exp = nullptr;
    }
    memset(&m_current_key_info, 0, sizeof(KeyInfoStructure));
    m_current_context_slots.clear();
}

CryptExecStatus VirtualSecureElement::LocalGetKeySlot(KeyInfoStructure &key_info, u8 **key_value)
{
    u256 FSCryptoKey;
    bool hash_success = false;
    bool is_external_key_value_buffer = false;
    if (*key_value != nullptr)
    {
        is_external_key_value_buffer = true;
    }
    // Derive key_FS
    const auto derive_fs_key_success = GetFSKey(FSCryptoKey);
    if (!derive_fs_key_success)
    {
        fprintf(stdout, "\nLocalGetKeySlot: derive FS Key failed.");
        return CryptExecStatus::Execute_Error;
    }

    // Read the encrypted Key data from file
    u8 *pCGKeyData = nullptr;
    uint8_t *key_hash = nullptr;
    
    uint nKeyDataSize = 0; // padded
    KeyInfoStructure internal_key_info{};
    if (!ReadKeySlot(&pCGKeyData, &nKeyDataSize, key_info.slot)) // ReadKeySlot allocates the buffer for pCGKeyData
    {
        if (pCGKeyData != nullptr)
        {
            free(pCGKeyData);
        }
        fprintf(stdout, "\nLocalGetKeySlot:Reading Key %d returned error", key_info.slot);
        return CryptExecStatus::Execute_Error;
    }

    // Decrypt the Key data
    u8 *pKeyData = new u8[nKeyDataSize]{};
    if (pKeyData == nullptr)
    {
        fprintf(stdout, "\nLocalGetKeySlot:Failed to allocate memory");
        return CryptExecStatus::Mem_Error;
    }

    if (!ICypher::GetInstance()->GetAES()->DecryptData(pCGKeyData, pKeyData, nKeyDataSize, nKeyDataSize, FSCryptoKey,
                                                       sizeof(u256), nullptr, AES_CBC))
    {
        fprintf(stdout, "\nLocalGetKeySlot:DecryptData step returned error");
        goto ABORT;
    }

    memcpy(&internal_key_info, pKeyData, sizeof(KeyInfoStructure));
    if (internal_key_info.key_size > 1024)
    {
        fprintf(stdout, "\nLocalGetKeySlot: Incorrect key info storage. Corrupted key storage?");
        goto ABORT;
    }
    
    // get the decrypted key value
    if (*key_value == nullptr)
    {
        *key_value = new uint8_t[internal_key_info.key_size]{};
    }
    memcpy(*key_value, pKeyData + sizeof(KeyInfoStructure), internal_key_info.key_size);
    // Calculate hash
    key_hash = new uint8_t[sizeof(u512)]{};
    hash_success = ICypher::GetInstance()->GetSHA512()->Hash(*key_value, internal_key_info.key_size, key_hash);
    if (memcmp(key_hash, internal_key_info.key_hash, sizeof(u512)) != 0)
    {
        fprintf(stdout, "\nLocalGetKeySlot: Hash mismatch. Corrupted key storage?");
        memset(key_value, 0, internal_key_info.key_size);
        goto ABORT;
    }

    key_info = internal_key_info;

    memset(pKeyData, 0, nKeyDataSize);
    memset(pCGKeyData, 0, nKeyDataSize);
    delete[] pCGKeyData;
    if (pKeyData)
    {
        delete[] pKeyData;
        pKeyData = nullptr;
    }
    if (key_hash)
    {
        memset(key_hash, 0, sizeof(u512));
        delete[] key_hash;
        key_hash = nullptr;
    }
    return CryptExecStatus::Successful;
ABORT:
    if (!is_external_key_value_buffer)
    {
        delete[] * key_value;
        *key_value = nullptr;
    }
    memset(pKeyData, 0, nKeyDataSize);
    memset(pCGKeyData, 0, nKeyDataSize);
    delete[] pCGKeyData;
    if (pKeyData)
    {
        delete[] pKeyData;
        pKeyData = nullptr;
    }
    if (key_hash)
    {
        memset(key_hash, 0, sizeof(u512));
        delete[] key_hash;
        key_hash = nullptr;
    }
    return CryptExecStatus::Execute_Error;
}

bool VirtualSecureElement::CreateKCV(u8 *pKey, uint nKeySize, uint nZeros, uint nVals, u8 *pKCV, int nKCVType)
{
    IDES *pDES = ICypher::GetInstance()->GetDES();
    IAES *pAES = ICypher::GetInstance()->GetAES();
    ISHA256 *pSHA256 = ICypher::GetInstance()->GetSHA256();

    if (pKey == nullptr)
    {
        return false;
    }

    switch (nKCVType)
    {
    case KCV_AES_128_CBC:
    case KCV_AES_192_CBC:
    case KCV_AES_256_CBC:
    case KCV_TDES_128_CBC:
    case KCV_TDES_192_CBC:
    case KCV_SHA_256_NONE:
    case KCV_TDES_128_ECB:
    case KCV_TDES_192_ECB:
        // Only KCV modes supported
        break;
    default:
    {
        fprintf(stdout, "CreateKCV:Invalid KCV type %d", nKCVType);
        return false;
    }
    }

    if ((nZeros % 16 != 0) || (nVals > 32))
    {
        fprintf(stdout, "CreateKCV:nZeros %d is not multiple of 16 or nVals %d exceeds 32", nZeros, nVals);
        return false;
    }

    u8 *zeros = (u8 *)malloc(nZeros);
    u8 *tmp = (u8 *)malloc(nZeros);
    uint8_t ZERO_IV[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

    if ((zeros == nullptr) || (tmp == nullptr))
    {
        fprintf(stdout, "CreateKCV:Failed to allocate memory");
        goto ABORT;
    }

    memset(zeros, 0, nZeros);

    switch (nKCVType)
    {
    case KCV_TDES_128_CBC:
        if (!pDES->Encrypt(zeros, tmp, nZeros, pKey, &pKey[8], pKey, ZERO_IV, TDES_CBC))
        {
            fprintf(stdout, "CreateKCV:Encryption step returned error with KCV type %d", nKCVType);
            goto ABORT;
        }
        break;
    case KCV_TDES_192_CBC:
        if (!pDES->Encrypt(zeros, tmp, nZeros, pKey, &pKey[8], &pKey[16], ZERO_IV, TDES_CBC))
        {
            fprintf(stdout, "CreateKCV:Encryption step returned error with KCV type %d", nKCVType);
            goto ABORT;
        }
        break;
    case KCV_TDES_128_ECB:
        if (!pDES->Encrypt(zeros, tmp, nZeros, pKey, &pKey[8], pKey, ZERO_IV, TDES_ECB))
        {
            fprintf(stdout, "CreateKCV:Encryption step returned error with KCV type %d", nKCVType);
            goto ABORT;
        }
        break;

    case KCV_TDES_192_ECB:
        if (!pDES->Encrypt(zeros, tmp, nZeros, pKey, &pKey[8], &pKey[16], ZERO_IV, TDES_ECB))
        {
            fprintf(stdout, "CreateKCV:Encryption step returned error with KCV type %d", nKCVType);
            goto ABORT;
        }
        break;
    case KCV_AES_128_CBC:
    case KCV_AES_192_CBC:
    case KCV_AES_256_CBC:
        if (!pAES->Encrypt(zeros, tmp, nZeros, pKey, nKeySize, ZERO_IV, AES_CBC))
        {
            fprintf(stdout, "CreateKCV:Encryption step returned error with KCV type %d", nKCVType);
            goto ABORT;
        }
        break;
    case KCV_SHA_256_NONE:
        if (!pSHA256->Hash(pKey, nKeySize, tmp))
        {
            fprintf(stdout, "CreateKCV:Hash step returned error with KCV type %d", nKCVType);
            goto ABORT;
        }
        break;
    default:
    {
        fprintf(stdout, "CreateKCV:Invalid KCV type %d", nKCVType);
        goto ABORT;
    }
    }

    memcpy(pKCV, tmp, nVals);

    free(zeros);
    free(tmp);
    return true;
ABORT:
    free(zeros);
    free(tmp);
    return false;
}

bool VirtualSecureElement::WriteKeySlot(uint8_t *pKeyData, uint nSize, uint nKeySlot)
{
    using namespace std;
    namespace fs = filesystem;
    ofstream::sync_with_stdio(false);
    string file_path_str;
#ifdef __linux__
    const string path_prefix = "/var/data/local/secure_keys/Key_";
#elif defined _WIN32
    const string path_prefix = R"(C:\ProgramData\Lumidigm\SecKeys\Key_)";
#endif
    file_path_str = path_prefix + to_string(nKeySlot) + ".bin";
    const auto file_path = fs::path(file_path_str);
#if INCLUDE_STD_FILESYSTEM_EXPERIMENTAL
    if (!fs::exists(file_path.parent_path()))
#else
    if (!fs::directory_entry(file_path.parent_path()).exists())
#endif
    {
        fs::create_directories(file_path.parent_path());
    }
    ofstream output_bin(file_path_str, ios::binary);
    output_bin.write((const char *)pKeyData, nSize);
    output_bin.flush();
    output_bin.close();
    return true;
}

bool VirtualSecureElement::ReadKeySlot(uint8_t **pKeyData, uint *nSize, uint nKeySlot)
{
    using namespace std;
    namespace fs = filesystem;
    ifstream::sync_with_stdio(false);
    auto pAES = ICypher::GetInstance()->GetAES();
    string file_path_str;
#ifdef __linux__
    const string path_prefix = "/var/data/local/secure_keys/Key_";
#elif defined _WIN32
    const string path_prefix = R"(C:\ProgramData\Lumidigm\SecKeys\Key_)";
#endif
    file_path_str = path_prefix + to_string(nKeySlot) + ".bin";
    const auto file_path = fs::path(file_path_str);
//#if INCLUDE_STD_FILESYSTEM_EXPERIMENTAL
//    if (!fs::exists(file_path.parent_path()))
//#else
//    if (!fs::directory_entry(file_path.parent_path()).exists())
//#endif
//    {
//        fs::create_directories(file_path.parent_path());
//    }
    ifstream input_bin(file_path_str, ios::binary);
    if(!input_bin.is_open())
       return false;

    input_bin.seekg(0, ios::end);
    const size_t size = input_bin.tellg();
    *nSize = (uint)size;
    input_bin.seekg(0, ios::beg);
    *pKeyData = new uint8_t[size]{};

    bool succeeded = !!input_bin.read((char *)(*pKeyData), size);
    if(!succeeded)
    {
        delete[] *pKeyData;
        *pKeyData = nullptr;
    }
    return succeeded;
}

bool VirtualSecureElement::EraseKeySlot(uint nKeySlot)
{
    using namespace std;
    namespace fs = filesystem;
    string file_path_str;
    fprintf(stdout, "Erasing slot #%d", nKeySlot);
#ifdef __linux__
    const string path_prefix = "/var/data/local/secure_keys/Key_";
#elif defined _WIN32
    const string path_prefix = R"(C:\ProgramData\Lumidigm\SecKeys\Key_)";
#endif
    file_path_str = path_prefix + to_string(nKeySlot) + ".bin";
    const auto file_path = fs::path(file_path_str);
    if (fs::exists(file_path))
    {
        return fs::remove(file_path);
    }
    return true;
}

bool VirtualSecureElement::GetFSKey(u256 &pKey)
{
    using namespace std;
    namespace fs = filesystem;

    IAES *p_AES = ICypher::GetInstance()->GetAES();

#ifdef __linux__
    string master_key_file = "/var/data/local/secure_keys/master_key";
    string FS_key_rand_file = "/var/data/local/secure_keys/nFSKey";
#elif defined _WIN32
    string master_key_file = R"(C:\ProgramData\Lumidigm\SecKeys\master_key)";
    string FS_key_rand_file = R"(C:\ProgramData\Lumidigm\SecKeys\nFSKey)";
#endif
    if (!fs::exists(master_key_file))
    {
        fprintf(stdout, "\nMaster key doesn't exist!");
        return false;
    }
    if (!fs::exists(FS_key_rand_file))
    {
        fprintf(stdout, "\nMaster random number doesn't exist!");
        return false;
    }

    ifstream FS_key_reader(FS_key_rand_file, ios::in | ios::binary);
    ifstream master_key_reader(master_key_file, ios::in | ios::binary);

    if (!FS_key_reader)
    {
        fprintf(stdout, "\nOpen master random number doesn't exist!");
        return false;
    }
    // char* rnd_clear_buf = new char[32]{};
    u256 rnd_clear_buf{};
    u256 enc_fs_key{};
    u256 master_key_buf{};
    u512 key_sha512{};

    master_key_reader.read((char *)&master_key_buf, sizeof(u256));
    FS_key_reader.read((char *)rnd_clear_buf, sizeof(u256));
    FS_key_reader.read((char *)key_sha512, sizeof(u512));
    p_AES->Encrypt(rnd_clear_buf, enc_fs_key, sizeof(u256), master_key_buf, sizeof(u256), nullptr,
                   AES_BLOCK_Modes_Type::AES_CBC);

    uint8_t *hash_buffer = nullptr;
    uint32_t hash_buffer_len = 0;
    Execute_Hash_Data(enc_fs_key, sizeof(u256), HashAlgorithms::SHA2_512, &hash_buffer, hash_buffer_len);
    assert(hash_buffer_len == sizeof(u512));

    if (memcmp(hash_buffer, key_sha512, sizeof(u512)) != 0)
    {
        fprintf(stdout, "\nError: SHA512 result does not match the original hash! Wrong key!");
        return false;
    }
    memcpy(pKey, enc_fs_key, sizeof(u256));
    delete[] hash_buffer;
    hash_buffer = nullptr;
    return true;
}

bool VirtualSecureElement::IsSEKeyLoaded(u16 nSlot)
{
    using namespace std;
    namespace fs = filesystem;
    
#ifdef __linux__
    const string path_prefix = "/var/data/local/secure_keys/SEK_";
#elif defined _WIN32
    const string path_prefix = R"(C:\ProgramData\Lumidigm\SecKeys\SEK_)";
#endif
    const string path = path_prefix + to_string(nSlot) + ".bin";
    if (fs::exists(path))
    {
        return true;
    }
    return false;
}

bool VirtualSecureElement::IsKeyExist(u16 nSlot)
{
    using namespace std;
    namespace fs = filesystem;
    ifstream::sync_with_stdio(false);
    
    string file_path_str;
#ifdef __linux__
    const string path_prefix = "/var/data/local/secure_keys/Key_" ;
#elif defined _WIN32
    const string path_prefix = R"(C:\ProgramData\Lumidigm\SecKeys\Key_)";
#endif
    file_path_str = path_prefix  + to_string(nSlot) + ".bin";
    const auto file_path = fs::path(file_path_str);
#if INCLUDE_STD_FILESYSTEM_EXPERIMENTAL
    if (!fs::exists(file_path.parent_path()))
#else
    if (!fs::directory_entry(file_path.parent_path()).exists())
#endif
    {
        fs::create_directories(file_path.parent_path());
    }
    const ifstream input_bin(file_path_str, ios::binary);
    return input_bin.is_open();
}

bool VirtualSecureElement::Encrypt(u8 *pKey, uint nKeySize, u16 nKeyMode, u8 *pIn, uint nInSize, u8 **pOutCG,
                                   uint &nOutCGSize)
{
    ICRYPTOAlgo *pCryptoAlgo = nullptr;
    ICypher *pCypher = ICypher::GetInstance();
    int nAlgoMode;

    if ((nullptr == pIn) || (0 == nInSize))
    {
        fprintf(stdout, "Encrypt:Initial input arugment validation failed");
        return false;
    }

    if (false == GetCryptoAlgo(nKeyMode, nAlgoMode, &pCryptoAlgo))
    {
        fprintf(stdout, "Encrypt:Getting Crypto Algo step returned error");
        return false;
    }

    // Alloc in Buffer with N_Block aligned and copy
    uint nInAligSz = nInSize;
    if (nInAligSz % BLOCK_SIZE)
    {
        const int nAdj = BLOCK_SIZE - (nInAligSz % BLOCK_SIZE);
        nInAligSz += nAdj;
    }

    u8 *pInBuff = new u8[nInAligSz]{};
    if (nullptr == pInBuff)
    {
        pInBuff = nullptr;
        fprintf(stdout, "Encrypt:Failed to allocate memory.");
        return false;
    }

    // AutoFreePtr afp_InBuff(pInBuff);
    memset(pInBuff, 0, nInAligSz);
    memcpy(pInBuff, pIn, nInSize);

    // Alloc out buffer
    const uint nOutSz = nInAligSz;

    *pOutCG = new u8[nOutSz];
    if (nullptr == *pOutCG)
    {
        fprintf(stdout, "Encrypt:Failed to allocate memory.");
        delete[] pInBuff;
        pInBuff = nullptr;
        delete[] pOutCG;
        pOutCG = nullptr;
        return false;
    }
    if (false ==
        pCryptoAlgo->EncryptData(pInBuff, *pOutCG, (int)nInAligSz, (int)nOutSz, pKey, (int)nKeySize, nullptr,
                                 nAlgoMode))
    {
        fprintf(stdout, "Encrypt:Encrypting input Data returned error");
        delete[] pInBuff;
        pInBuff = nullptr;
        delete[] pOutCG;
        pOutCG = nullptr;
        return false;
    }
    delete[] pInBuff;
    pInBuff = nullptr;
    nOutCGSize = nOutSz;
    return true;
}

bool VirtualSecureElement::Decrypt(u8 *pKey, uint nKeySize, u16 nKeyMode, u8 *pInCG, uint nInCGSize, u8 *pOut,
                                   uint *nOutSize)
{
    ICRYPTOAlgo *pCryptoAlgo = nullptr;

    int nAlgoMode;

    if ((nullptr == pInCG) || (nullptr == pOut) || (0 == nInCGSize))
    {
        fprintf(stdout, "Decrypt:Initial input arugment validation failed");
        return false;
    }

    if (false == GetCryptoAlgo(nKeyMode, nAlgoMode, &pCryptoAlgo))
    {
        fprintf(stdout, "Decrypt:Getting Crypto Algo step returned error");
        return false;
    }

    const uint nActualOutSz = nInCGSize;

    if (false ==
        pCryptoAlgo->DecryptData(pInCG, pOut, (int)nInCGSize, (int)nActualOutSz, pKey, (int)nKeySize, nullptr,
                                 nAlgoMode))
    {
        fprintf(stdout, "Decrypt:Decrypting input Data returned error");
        return false;
    }

    *nOutSize = nActualOutSz;
    return true;
}

bool VirtualSecureElement::GetCryptoAlgo(u16 nKeyMode, int &nAlgoMode, ICRYPTOAlgo **pCryptoAlgo)
{
    ICypher *pCypher = ICypher::GetInstance();
    *pCryptoAlgo = nullptr;
    // Get the Algo and mode w.r.t KeyMode.
    switch ((KeyModes)nKeyMode)
    {
    case KeyModes::AES_256_CBC:
    {
        *pCryptoAlgo = pCypher->GetAES();
        nAlgoMode = AES_CBC;
    }
    break;
    case KeyModes::AES_128_CBC:
    {
        *pCryptoAlgo = pCypher->GetAES();
        nAlgoMode = AES_CBC;
    }
    break;
    case KeyModes::TDES_ABA_ECB:
    case KeyModes::TDES_ABC_ECB:
    {
        *pCryptoAlgo = pCypher->GetDES();
        nAlgoMode = TDES_ECB;
    }
    break;
    case KeyModes::TDES_ABA_CBC:
    case KeyModes::TDES_ABC_CBC:
    {
        *pCryptoAlgo = pCypher->GetDES();
        nAlgoMode = TDES_CBC;
    }
    break;
    case KeyModes::RSA_2048_v15: // For variable keyModes slot these are not used
    case KeyModes::RSA_2048_v21:
    case KeyModes::DUKPT_IPEK_128:
    case KeyModes::DUKPT_KSN_64:
    default:
    {
        fprintf(stdout, "GetCryptoAlgo:Invalid key Mode(%u)", (uint32_t)nKeyMode);
        return false;
    }
    }

    return true;
}

CryptExecStatus VirtualSecureElement::Execute_Check_Key_Exist(uint32_t key_slot)
{
    return IsKeyExist(key_slot) ? CryptExecStatus::Successful : CryptExecStatus::Not_Exist;
}

CryptExecStatus VirtualSecureElement::Execute__test(int32_t pod_in1, double pod_in2,
                                                    const std::pair<uint32_t, int32_t> &pod_in3, uint32_t &pod_out1,
                                                    const uint8_t *buffer_in, uint32_t buffer_in_len,
                                                    uint8_t *buffer_out, uint32_t buffer_out_len,
                                                    uint8_t **allocated_buffer_out, uint32_t &allocated_buffer_out_len,
                                                    const uint32_t *buffer_in2, uint32_t buffer_in2_len)
{
    assert(pod_in3 == std::make_pair(10u, -10));
    if (pod_in3 != std::make_pair(10u, -10))
        return CryptExecStatus::Execute_Error;

    pod_out1 = 7890;

    assert(buffer_in_len == 8);
    if (buffer_in_len != 8)
        return CryptExecStatus::Execute_Error;
    for (uint32_t i = 0; i < 8; i++)
    {
        assert(buffer_in[i] == (100 + i));
        if (buffer_in[i] != (100 + i))
            return CryptExecStatus::Execute_Error;
    }

    for (uint32_t i = 0; i < buffer_out_len; i++)
        buffer_out[i] = 200 + i;

    allocated_buffer_out_len = 100;
    *allocated_buffer_out = new uint8_t[allocated_buffer_out_len];
    for (uint32_t i = 0; i < allocated_buffer_out_len; i++)
        (*allocated_buffer_out)[i] = 50 + i;

    assert(buffer_in2_len == 128);
    if (buffer_in2_len != 128)
        return CryptExecStatus::Execute_Error;
    for (uint32_t i = 0; i < 128; i++)
    {
        assert(buffer_in2[i] == (100000 + i));
        if (buffer_in2[i] != (100000 + i))
            return CryptExecStatus::Execute_Error;
    }

    if ((pod_in1 == 1999) && (pod_in2 == 11.11))
        *(volatile int *)0 = 0; // crash deliberately

    assert(pod_in1 == 1234);
    if (pod_in1 != 1234)
        return CryptExecStatus::Execute_Error;
    assert(pod_in2 == 56.78);
    if (pod_in2 != 56.78)
        return CryptExecStatus::Execute_Error;

    return CryptExecStatus::Successful;
}
void VirtualSecureElement::Disconnect(){
    
}