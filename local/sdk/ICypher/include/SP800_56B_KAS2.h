// ================================================================================================================================
//
// NOTES:
//
//      U == Initiator == Host
//      V == Responder == Device
//
// SEE ALSO:
//
//      http://csrc.nist.gov/publications/nistpubs/800-56B/sp800-56B.pdf
//      http://csrc.nist.gov/publications/nistpubs/800-108/sp800-108.pdf
//
// ================================================================================================================================
#pragma once
#include <cstring>
#include "IMemMgr.h"

extern bool SP800_56B_KAS2_self_test();

class SP800_56B_KAS2 : public MemoryBase
{
public:

    SP800_56B_KAS2()    :
        U_PRV_PEM(NULL),
        U_CRT_PEM(NULL),
        U_ROOT_CA_PEM(NULL),
        V_PRV_PEM(NULL),
        V_CRT_PEM(NULL),
        V_ROOT_CA_PEM(NULL)
    {
        Disable();
    }
    virtual ~SP800_56B_KAS2()
    {
        Disable();
    }

    enum RSA_MODE
    {
        MODE_RSA_RAW       = 0,
        MODE_RSA_PKCS1_V15 = 1,
        MODE_RSA_PKCS1_V21 = 2
    };

    virtual void Disable();

    // what padding shall we use for RSA encrypt/decrypt primitive?
    // SP800-56B KAS2 specifies RAW mode.
    // it is rumored that many HSMs only support PKCS1-padded modes.
    // we are supplying support for this here, even though it is not textbook-compliant.
    // note that if no Set_Mode_...() method is called, we default to raw.
    virtual void Set_Mode_Raw()
    { mode = MODE_RSA_RAW; }
    virtual void Set_Mode_PKCS1_v15()
    { mode = MODE_RSA_PKCS1_V15; }
    virtual void Set_Mode_PKCS1_v21()
    { mode = MODE_RSA_PKCS1_V21; }
    virtual void Set_Mode(RSA_MODE mode_in)
    { mode = mode_in; }

    virtual bool U_Init(const char * u_CRT_PEM,     size_t u_CRT_PEM_LEN,
                        const char * u_PRV_PEM,     size_t u_PRV_PEM_LEN,
                        const char * v_CRT_PEM,     size_t v_CRT_PEM_LEN,
                        const char * v_ROOT_CA_PEM, size_t v_ROOT_CA_PEM_LEN);
    virtual bool V_Init(const char * v_CRT_PEM,     size_t v_CRT_PEM_LEN,
                        const char * v_PRV_PEM,     size_t v_PRV_PEM_LEN,
                        const char * u_CRT_PEM,     size_t u_CRT_PEM_LEN,
                        const char * u_ROOT_CA_PEM, size_t u_ROOT_CA_PEM_LEN);

    // pp.85    NIST SP 800-56B: KAS2 U_CRT_PEM||\0||Cu
    virtual bool U_Phase_1(      uint8_t * cryptogram_out, size_t & out_len);
    // pp.85    NIST SP 800-56B: KAS2 U_CRT_PEM||\0||Cu
    virtual bool V_Phase_2(const uint8_t * cryptogram_in,  size_t   in_len);
    // pp.85,93 NIST SP 800-56B: KAS2 V_CRT_PEM||\0||Cv||MacTagV
    virtual bool V_Phase_3(      uint8_t * cryptogram_out, size_t & out_len);
    // pp.85,93 NIST SP 800-56B: KAS2 V_CRT_PEM||\0||Cv||MacTagV
    virtual bool U_Phase_4(const uint8_t * cryptogram_in,  size_t   in_len);
    // pp.93    NIST SP 800-56B: KAS2 MacTagU
    virtual bool U_Phase_5(      uint8_t * cryptogram_out, size_t & out_len);
    // pp.93    NIST SP 800-56B: KAS2 MacTagU
    virtual bool V_Phase_6(const uint8_t * cryptogram_in,  size_t   in_len);

    virtual bool Assert_Complete();

    virtual bool Get_Key_Material(size_t offset, uint8_t * buf, size_t len)
    {
        if (!Assert_Complete())
            return false;
        offset += 32;   // skip MacKey
        if (offset + len > sizeof(Ko))
            return false;
        if (len > 32)
            return false;
        memcpy(buf, Ko +  offset, len);
        return true;
    }

private:

    virtual bool Assert_U(int p);   // p == expected phase
    virtual bool Assert_V(int p);   // p == expected phase

    virtual void Clear_Intermediate_Keying_Material()
    {
        SecureClearBuffer(Zu, sizeof(Zu));
        SecureClearBuffer(Zv, sizeof(Zv));
        SecureClearBuffer(Cu, sizeof(Cu));
        SecureClearBuffer(Cv, sizeof(Cv));
        SecureClearBuffer(MacTagU, sizeof(MacTagU));
        SecureClearBuffer(MacTagV, sizeof(MacTagV));
        SecureClearBuffer(Ko + 0, 32);  // this clears the MacKey from the SP800 108 KDF output buffer, leaving other keys intact.
    }

    int phase;
    int mode;

    char       * U_PRV_PEM;
    size_t       U_PRV_PEM_LEN;
     char      * U_CRT_PEM;
    size_t       U_CRT_PEM_LEN;
     char      * U_ROOT_CA_PEM;
    size_t       U_ROOT_CA_PEM_LEN;
     char      * V_PRV_PEM;
    size_t       V_PRV_PEM_LEN;
     char      * V_CRT_PEM;
    size_t       V_CRT_PEM_LEN;
     char      * V_ROOT_CA_PEM;
    size_t       V_ROOT_CA_PEM_LEN;

          uint8_t Cu[256];      // Cu ciphertext of U's random number Zu
          uint8_t Cv[256];      // Cv ciphertext of V's random number Zv
          uint8_t Zu[256];      // Zu plaintext of U's random number - front-padded with zeros
          uint8_t Zv[256];      // Zv plaintext of V's random number - front-padded with zeros
          uint8_t MacTagU[32];  // MacTag calculated by U
          uint8_t MacTagV[32];  // MacTag calculated by V
          uint8_t Ko[7*32];     // Derived 256-bit (max) keys - KMAC, 4x CTR+HMAC, 2x GCM
};

// ================================================================================================================================
