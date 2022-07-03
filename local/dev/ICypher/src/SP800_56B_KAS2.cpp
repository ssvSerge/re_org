// ================================================================================================================================
// ================================================================================================================================

#include "ICypher.h"
#include "Auto_Crypto_Objects.h"
#include "PlatformDev.h"
#include "lumi_random.h"
#include "c_x509.h"
#include "SP800_56B_KAS2.h"
#include "SP800_108_KDF_CTR_HMAC_SHA.h"

// --------------------------------------------------------------------------------------------------------------------------------
//
#define DISABLE_FAIL do { Disable(); return false; } while (0)

// --------------------------------------------------------------------------------------------------------------------------------
//
#define WRITE_PTR(out, in, len) \
    memcpy(out, in, len);       \
    out += len;

// --------------------------------------------------------------------------------------------------------------------------------
// method to clear a buffer that will not be optimized out
//void SecureClearBuffer(void * p, size_t l)
//{
//    volatile unsigned char *v = reinterpret_cast<volatile unsigned char *>(p);
//    while (l--)
//        *v++ = 0;
//}

// --------------------------------------------------------------------------------------------------------------------------------
//
static bool Assert_Zero(const uint8_t * buf, size_t len)
{
    uint8_t a = 0;
    for (size_t i = 0; i < len; i++)
    {
        a |= buf[i];
    }
    return (a == 0);
}

// --------------------------------------------------------------------------------------------------------------------------------
//
const size_t MacTag_len    =  32;
const size_t entropy_bytes =  64;
const size_t modulus_bytes = 256;
const char * KDF_labels    = "KMAC" "EH2D" "ED2H" "MH2D" "MD2H" "GH2D" "GD2H";

// --------------------------------------------------------------------------------------------------------------------------------
// by altering the calling parameters, this method can be used to compute MacTagU or MacTagV
// output is a 32-byte MacTag
static bool Compute_MacTag(uint8_t       * output, const char    * label,
                           const char    * ID1,    size_t ID1_len,
                           const char    * ID2,    size_t ID2_len,
                           const uint8_t * C1,     const uint8_t * C2,
                           const uint8_t * MacKey, size_t          MacKey_len)
{
    // COMPUTE MacTag DATA
    size_t label_len = strlen(label);
    AutoHeapBuffer HMAC_Data(label_len + ID1_len + ID2_len + modulus_bytes * 2);
    if (!HMAC_Data.u8Ptr())
        return false;
    uint8_t * HMAC_Data_cursor = HMAC_Data.u8Ptr();
    WRITE_PTR(HMAC_Data_cursor, label, label_len);
    WRITE_PTR(HMAC_Data_cursor, ID1, ID1_len);
    WRITE_PTR(HMAC_Data_cursor, ID2, ID2_len);
    WRITE_PTR(HMAC_Data_cursor, C1, modulus_bytes);
    WRITE_PTR(HMAC_Data_cursor, C2, modulus_bytes);
    if (HMAC_Data_cursor - HMAC_Data.u8Ptr() != static_cast<ssize_t>(HMAC_Data.Len()))
        return false;
    // COMPUTE MacTag
    int rc_hmac = sha256_hmac(MacKey,            MacKey_len,
                              HMAC_Data.u8Ptr(), HMAC_Data.Len(),
                              output,            0/*is_224*/);
    return (rc_hmac == 0);
}

// --------------------------------------------------------------------------------------------------------------------------------
// NOTE: both plaintext and ciphertext are "modulus-bytes" buffers.
//       plaintext is always "entropy_bytes" of entropy, right justified in the buffer.
//       when using raw RSA, this is the preferred format.
//       when using the various PKCS #1 padding modes, we have to modify the passed plaintext pointer(s)
//       ciphertext should be a buffer to receive "modulus_bytes" of data.
static int rsa_public_op(int mode, rsa_context * ctx, int (*f_rng)(void *, unsigned char *, size_t), void *p_rng,
                         const uint8_t * plaintext, uint8_t * ciphertext)
{
    switch (mode)
    {
        case SP800_56B_KAS2::MODE_RSA_RAW:
            return rsa_public(ctx, plaintext, ciphertext);
        case SP800_56B_KAS2::MODE_RSA_PKCS1_V15:
            //DEBUG("EC: ");
            //DEBUG_HEXDUMP((plaintext + modulus_bytes - entropy_bytes), entropy_bytes);
            //fflush(stdout);
            rsa_set_padding(ctx, RSA_PKCS_V15, POLARSSL_MD_SHA256);
            return rsa_rsaes_pkcs1_v15_encrypt(ctx, f_rng, p_rng, RSA_PUBLIC, entropy_bytes,
                                               plaintext + modulus_bytes - entropy_bytes, ciphertext);
        case SP800_56B_KAS2::MODE_RSA_PKCS1_V21:
            rsa_set_padding(ctx, RSA_PKCS_V21, POLARSSL_MD_SHA256);
            return rsa_rsaes_oaep_encrypt(ctx, f_rng, p_rng, RSA_PUBLIC, NULL/*label*/, 0/*label_len*/, entropy_bytes,
                                          plaintext + modulus_bytes - entropy_bytes, ciphertext);
    }
    return -1;
}

// --------------------------------------------------------------------------------------------------------------------------------
// NOTE: both plaintext and ciphertext are "modulus-bytes" buffers.
//       plaintext is always "entropy_bytes" of entropy, right justified in the buffer.
//       when using raw RSA, this is the preferred format.
//       when using the various PKCS #1 padding modes, we have to modify the passed plaintext pointer(s)
//       ciphertext should be a buffer to receive "modulus_bytes" of data.
static int rsa_private_op(int mode, rsa_context * ctx, int (*f_rng)(void *, unsigned char *, size_t), void *p_rng,
                          const uint8_t * ciphertext, uint8_t * plaintext)
{
    size_t olen = modulus_bytes;
    switch (mode)
    {
        case SP800_56B_KAS2::MODE_RSA_RAW:
            return rsa_private(ctx, f_rng, p_rng, ciphertext, plaintext);
        case SP800_56B_KAS2::MODE_RSA_PKCS1_V15:
            rsa_set_padding(ctx, RSA_PKCS_V15, POLARSSL_MD_SHA256);
            if (rsa_rsaes_pkcs1_v15_decrypt(ctx, f_rng, p_rng, RSA_PRIVATE, &olen, ciphertext, plaintext, modulus_bytes) != 0)
                return -1;
            if (olen != entropy_bytes)
                return -1;
            //DEBUG("DC: ");
            //DEBUG_HEXDUMP(plaintext, entropy_bytes);
            //fflush(stdout);
            // move entropy into proper location that caller expects.
            memcpy(plaintext + modulus_bytes - entropy_bytes, plaintext, entropy_bytes);
            // clear leading bytes before entropy
            memset(plaintext, 0, modulus_bytes - entropy_bytes);
            return 0;
        case SP800_56B_KAS2::MODE_RSA_PKCS1_V21:
            rsa_set_padding(ctx, RSA_PKCS_V21, POLARSSL_MD_SHA256);
            if (rsa_rsaes_oaep_decrypt(ctx, f_rng, p_rng, RSA_PRIVATE, NULL/*label*/, 0/*label_len*/, &olen,
                                       ciphertext, plaintext, modulus_bytes) != 0)
                return -1;
            if (olen != entropy_bytes)
                return -1;
            // move entropy into proper location that caller expects.
            memcpy(plaintext + modulus_bytes - entropy_bytes, plaintext, entropy_bytes);
            // clear leading bytes before entropy
            memset(plaintext, 0, modulus_bytes - entropy_bytes);
            return 0;
    }
    return -1;
}

void SP800_56B_KAS2::Disable()
{
    mode = MODE_RSA_RAW;
    phase = -1;
    if (U_PRV_PEM)
    {
        memset(U_PRV_PEM, 0, U_PRV_PEM_LEN);
        FREE(U_PRV_PEM);
    }
    U_PRV_PEM = NULL;
    if (U_CRT_PEM)
        FREE(U_CRT_PEM);
    U_CRT_PEM = NULL;
    if (U_ROOT_CA_PEM)
        FREE(U_ROOT_CA_PEM);
    U_ROOT_CA_PEM = NULL;
    if (V_PRV_PEM)
    {
        memset(V_PRV_PEM, 0, V_PRV_PEM_LEN);
        FREE(V_PRV_PEM);
    }
    V_PRV_PEM = NULL;
    if (V_CRT_PEM)
        FREE(V_CRT_PEM);
    V_CRT_PEM = NULL;
    if (V_ROOT_CA_PEM)
        FREE(V_ROOT_CA_PEM);
    V_ROOT_CA_PEM = NULL;
    Clear_Intermediate_Keying_Material();
    SecureClearBuffer(Ko, sizeof(Ko));
}

// --------------------------------------------------------------------------------------------------------------------------------
//
bool SP800_56B_KAS2::U_Init(const char * u_CRT_PEM, size_t u_CRT_PEM_LEN,
    const char * u_PRV_PEM, size_t u_PRV_PEM_LEN,
    const char * v_CRT_PEM, size_t v_CRT_PEM_LEN,
    const char * v_ROOT_CA_PEM, size_t v_ROOT_CA_PEM_LEN)
{
    Disable();
    phase = 0;

    if (!u_CRT_PEM || u_CRT_PEM_LEN < 1)
        return false;
    if (!u_PRV_PEM || u_PRV_PEM_LEN < 1)
        return false;
    if (!v_CRT_PEM || v_CRT_PEM_LEN < 1)
        return false;
    if (!v_ROOT_CA_PEM || v_ROOT_CA_PEM_LEN < 1)
        return false;

    U_CRT_PEM = (char*)MALLOC(u_CRT_PEM_LEN + 1);
    U_PRV_PEM = (char*)MALLOC(u_PRV_PEM_LEN + 1);
    V_CRT_PEM = (char*)MALLOC(v_CRT_PEM_LEN + 1);
    V_ROOT_CA_PEM = (char*)MALLOC(v_ROOT_CA_PEM_LEN + 1);

    if (!U_CRT_PEM || !U_PRV_PEM || !V_CRT_PEM || !V_ROOT_CA_PEM)
        return false;

    memcpy(U_CRT_PEM, u_CRT_PEM, u_CRT_PEM_LEN);
    U_CRT_PEM[u_CRT_PEM_LEN] = '\0';
    U_CRT_PEM_LEN = strlen(U_CRT_PEM);

    memcpy(U_PRV_PEM, u_PRV_PEM, u_PRV_PEM_LEN);
    U_PRV_PEM[u_PRV_PEM_LEN] = '\0';
    U_PRV_PEM_LEN = strlen(U_PRV_PEM);

    memcpy(V_CRT_PEM, v_CRT_PEM, v_CRT_PEM_LEN);
    V_CRT_PEM[v_CRT_PEM_LEN] = '\0';
    V_CRT_PEM_LEN = strlen(V_CRT_PEM);

    memcpy(V_ROOT_CA_PEM, v_ROOT_CA_PEM, v_ROOT_CA_PEM_LEN);
    V_ROOT_CA_PEM[v_ROOT_CA_PEM_LEN] = '\0';
    V_ROOT_CA_PEM_LEN = strlen(V_ROOT_CA_PEM);

    // perform...
    // XXX - consider question: shall we always trust that a pinned cert is verified before pinning?
    //       if answer is yes, we can skip this next step...
    // VERIFY V'S CERT AGAINST PINNED DEVICE ROOT CA
    oX509* poCertProcessor = new oX509();
    AutoDestructObject<oX509> pCert(poCertProcessor);

    bool rc_verify_chain = poCertProcessor->VerifyCertChain(V_ROOT_CA_PEM, V_ROOT_CA_PEM_LEN,
                                                            V_CRT_PEM,     V_CRT_PEM_LEN);
    return rc_verify_chain;
}

// --------------------------------------------------------------------------------------------------------------------------------
//
bool SP800_56B_KAS2::V_Init(const char * v_CRT_PEM,     size_t v_CRT_PEM_LEN,
                            const char * v_PRV_PEM,     size_t v_PRV_PEM_LEN,
                            const char * u_CRT_PEM,     size_t u_CRT_PEM_LEN,
                            const char * u_ROOT_CA_PEM, size_t u_ROOT_CA_PEM_LEN)
{
    Disable();
    phase = 0;

    if (!v_CRT_PEM || v_CRT_PEM_LEN < 1)
        return false;
    if (!v_PRV_PEM || v_PRV_PEM_LEN < 1)
        return false;
    if (!u_CRT_PEM || u_CRT_PEM_LEN < 1)
        return false;
    if (!u_ROOT_CA_PEM || u_ROOT_CA_PEM_LEN < 1)
        return false;

    V_CRT_PEM = (char*)MALLOC(v_CRT_PEM_LEN + 1);
    V_PRV_PEM = (char*)MALLOC(v_PRV_PEM_LEN + 1);
    U_CRT_PEM = (char*)MALLOC(u_CRT_PEM_LEN + 1);
    U_ROOT_CA_PEM = (char*)MALLOC(u_ROOT_CA_PEM_LEN + 1);

    if (!V_CRT_PEM || !V_PRV_PEM || !U_CRT_PEM || !U_ROOT_CA_PEM)
        return false;

    memcpy(V_CRT_PEM, v_CRT_PEM, v_CRT_PEM_LEN);
    V_CRT_PEM[v_CRT_PEM_LEN] = '\0';
    V_CRT_PEM_LEN = strlen(V_CRT_PEM);

    memcpy(V_PRV_PEM, v_PRV_PEM, v_PRV_PEM_LEN);
    V_PRV_PEM[v_PRV_PEM_LEN] = '\0';
    V_PRV_PEM_LEN = strlen(V_PRV_PEM);

    memcpy(U_CRT_PEM, u_CRT_PEM, u_CRT_PEM_LEN);
    U_CRT_PEM[u_CRT_PEM_LEN] = '\0';
    U_CRT_PEM_LEN = strlen(U_CRT_PEM);

    memcpy(U_ROOT_CA_PEM, u_ROOT_CA_PEM, u_ROOT_CA_PEM_LEN);
    U_ROOT_CA_PEM[u_ROOT_CA_PEM_LEN] = '\0';
    U_ROOT_CA_PEM_LEN = strlen(U_ROOT_CA_PEM);

    // XXX - consider question: shall we always trust that a pinned cert is verified before pinning?
    //       if answer is yes, we can skip this next step...
    // VERIFY U'S CERT AGAINST PINNED HOST ROOT CA
    oX509* poCertProcessor = new oX509();
    AutoDestructObject<oX509> pCert(poCertProcessor);
    bool rc_verify_chain = poCertProcessor->VerifyCertChain(U_ROOT_CA_PEM, U_ROOT_CA_PEM_LEN,
                                                            U_CRT_PEM,     U_CRT_PEM_LEN);
    return rc_verify_chain;
}

// --------------------------------------------------------------------------------------------------------------------------------
//
bool SP800_56B_KAS2::Assert_Complete()
{
    bool bRC = true;

    if (!Assert_Zero(Zu, sizeof(Zu)))
        bRC = false;
    if (!Assert_Zero(Zv, sizeof(Zv)))
        bRC = false;
    if (!Assert_Zero(Cu, sizeof(Cu)))
        bRC = false;
    if (!Assert_Zero(Cv, sizeof(Cv)))
        bRC = false;
    if (!Assert_Zero(Ko, 32))    // MacKey
        bRC = false;

    if (U_PRV_PEM)
    {
        if (!Assert_U(5))
            bRC = false;
    }
    else if (V_PRV_PEM)
    {
        if (!Assert_V(6))
            bRC = false;
    }
    else
        bRC = false;

    if (!bRC)
        Disable();

    return bRC;
}

// --------------------------------------------------------------------------------------------------------------------------------
//
bool SP800_56B_KAS2::Assert_U(int p)
{
    bool bRC = true;
    if (phase != p)
        bRC = false;
    if (!U_PRV_PEM)
        bRC = false;
    if (!U_CRT_PEM)
        bRC = false;
    if ( V_PRV_PEM)     // we shouldn't have this...
        bRC = false;
    if (!V_CRT_PEM)
        bRC = false;
    if (!V_ROOT_CA_PEM)
        bRC = false;
    if (!bRC)
        Disable();
    return bRC;
}

// --------------------------------------------------------------------------------------------------------------------------------
//
bool SP800_56B_KAS2::Assert_V(int p)
{
    bool bRC = true;
    if (phase != p)
        bRC = false;
    if (!V_PRV_PEM)
        bRC = false;
    if (!V_CRT_PEM)
        bRC = false;
    if ( U_PRV_PEM)     // we shouldn't have this...
        bRC = false;
    if (!U_CRT_PEM)
        bRC = false;
    if (!U_ROOT_CA_PEM)
        bRC = false;
    if (!bRC)
        Disable();
    return bRC;
}

// --------------------------------------------------------------------------------------------------------------------------------
// pp.85    NIST SP 800-56B: KAS2 U_CRT_PEM||\0||Cu
bool SP800_56B_KAS2::U_Phase_1(uint8_t * cryptogram_out, size_t & out_len)
{
    size_t out_len_copy = out_len;
    out_len = 0;

    // incoming assertions
    if (!Assert_U(0))
        DISABLE_FAIL;
    if (!cryptogram_out)
        DISABLE_FAIL;
    size_t U_CRT_PEM_len = U_CRT_PEM_LEN;
    size_t required = 1/*mode byte*/ + U_CRT_PEM_len + 1/*nul*/ + sizeof(Cu);
    if (out_len_copy < required)
        DISABLE_FAIL;

    // EXTRACT V's PUBLIC KEY FROM CERTIFICATE
    AutoHeapBuffer auto_V_PUB_PEM(IENVELOPE::RSA_2048_PEM_PUB_SIZE);
    if (!auto_V_PUB_PEM.u8Ptr())
        DISABLE_FAIL;
    Auto_x509_crt auto_x509_crt;
    if (!auto_x509_crt.Ptr())
        DISABLE_FAIL;
    int rc_parse_crt = x509_crt_parse(auto_x509_crt.Ptr(), reinterpret_cast<const unsigned char *>(V_CRT_PEM), V_CRT_PEM_LEN);
    if (rc_parse_crt)
        DISABLE_FAIL;
    // LOCATE LEAF CERT
    x509_crt * leaf = auto_x509_crt.Ptr();
    while (leaf->next)
        leaf = leaf->next;
    // INSURE LEAF CERT KEY USAGE IS KU_KEY_ENCIPHERMENT
    if ( (leaf->key_usage & KU_KEY_ENCIPHERMENT) == 0 )
        DISABLE_FAIL;
    int rc_write_pem = pk_write_pubkey_pem(&(leaf->pk), auto_V_PUB_PEM.u8Ptr(), auto_V_PUB_PEM.Len());
    if (rc_write_pem)
        DISABLE_FAIL;
    // LOAD V's PEM PUBLIC KEY INTO A PK CONTEXT
    Auto_pk_context  auto_pk_context;
    if (!auto_pk_context.Ptr())
        DISABLE_FAIL;
    int rc_key_parse = pk_parse_public_key(auto_pk_context.Ptr(), auto_V_PUB_PEM.u8Ptr(), auto_V_PUB_PEM.Len());
    if (rc_key_parse)
        DISABLE_FAIL;
    if (auto_pk_context.Ptr()->pk_info->type != POLARSSL_PK_RSA)
        DISABLE_FAIL;
    // GET Zu, U's RANDOM NUMBER
    // NOTE: buffer is same size as RSA modulus, and in big-endian.
    //       place entropy at the far side of the buffer - i.e. smaller number than possible.
    SecureClearBuffer(Zu, sizeof(Zu) - entropy_bytes);
    uint8_t * Zu_entropy = Zu + sizeof(Zu) - entropy_bytes;
    int rc_rand = lumi_random(NULL/*p_rng*/, Zu_entropy, entropy_bytes);
    if (rc_rand)
        DISABLE_FAIL;

    //DEBUG("Zu: ");
    //DEBUG_HEXDUMP(Zu_entropy, entropy_bytes);
    //fflush(stdout);

    // RAW RSA OF Zu->Cu USING V's PUBLIC KEY
    int rc_public = rsa_public_op(mode, reinterpret_cast<rsa_context *>(auto_pk_context.Ptr()->pk_ctx),
                                  lumi_random/*f_rng*/, NULL/*p_rng*/, Zu, Cu);
    if (rc_public)
        DISABLE_FAIL;

    // SERIALIZE OUT
    uint8_t * cryptogram_out_cursor = cryptogram_out;
    uint8_t m8 = mode;
    WRITE_PTR(cryptogram_out_cursor, &m8, 1);
    WRITE_PTR(cryptogram_out_cursor, U_CRT_PEM, U_CRT_PEM_len);
    WRITE_PTR(cryptogram_out_cursor, "", 1);
    WRITE_PTR(cryptogram_out_cursor, Cu, sizeof(Cu));
    if (cryptogram_out_cursor - cryptogram_out != static_cast<ssize_t>(required))
        DISABLE_FAIL;

    out_len = required;

    phase = 1;
    return true;
}

// --------------------------------------------------------------------------------------------------------------------------------
// pp.85    NIST SP 800-56B: KAS2 U_CRT_PEM||\0||Cu
bool SP800_56B_KAS2::V_Phase_2(const uint8_t * cryptogram_in, size_t in_len)
{
    // incoming assertions
    if (!Assert_V(0))
        DISABLE_FAIL;
    if (!cryptogram_in)
        DISABLE_FAIL;

    // perform:
    // PARSE RSA MODE
    const uint8_t * in_cursor = cryptogram_in;
    const uint8_t m8 = *in_cursor;
    in_cursor++; in_len--;
    mode = m8;
    // LOCATE NUL BYTE
    const uint8_t * crt_begin = in_cursor;
    while (in_len)
    {
        if (!(*in_cursor))
            break;
        in_cursor++;
        in_len--;
    }
    if (*in_cursor != '\0')
        DISABLE_FAIL;
    size_t xpct_cert_len = U_CRT_PEM_LEN;
    // DO WE HAVE A PINNED CERT FOR U?
    if (xpct_cert_len > 0)
    {
        // VERIFY THAT SENT CERT MATCHES PINNED CERT
        size_t rcvd_cert_len = in_cursor - crt_begin;
        if (rcvd_cert_len != xpct_cert_len)
            DISABLE_FAIL;
        if (strncmp(reinterpret_cast<const char *>(crt_begin), U_CRT_PEM, rcvd_cert_len))
            DISABLE_FAIL;
    }
    in_cursor++;
    in_len--;
    if (in_len != modulus_bytes)
        DISABLE_FAIL;
    memcpy(Cu, in_cursor, modulus_bytes);
    // EXTRACT V's PRIVATE KEY INTO A PK CONTEXT
    Auto_pk_context auto_pk_context;
    if (!auto_pk_context.Ptr())
        DISABLE_FAIL;
    int rc_key_parse = pk_parse_key(auto_pk_context.Ptr(), reinterpret_cast<const unsigned char *>(V_PRV_PEM), V_PRV_PEM_LEN,
                                    NULL/*pw*/, 0/*pwlen*/);
    if (rc_key_parse)
        DISABLE_FAIL;
    if (auto_pk_context.Ptr()->pk_info->type != POLARSSL_PK_RSA)
        DISABLE_FAIL;
    // RAW RSA OF Cu->Zu USING V's PRIVATE KEY
    Auto_rsa_context auto_rsa_context;
    if (!auto_rsa_context.Ptr())
        DISABLE_FAIL;
    rsa_copy(auto_rsa_context.Ptr(), reinterpret_cast<rsa_context *>(auto_pk_context.Ptr()->pk_ctx));
    int rc_private = rsa_private_op(mode, auto_rsa_context.Ptr(), lumi_random/*f_rng*/, NULL/*p_rng*/, in_cursor/*Cu*/, Zu);
    if (rc_private)
        DISABLE_FAIL;
    // ASSURE PREFIX OF Zu IS ALL ZEROS
    uint8_t nz = 0;
    uint8_t * Zu_cursor = Zu;
    for (size_t i = 0; i < sizeof(Zu) - entropy_bytes; i++, Zu_cursor++)
        nz |= *Zu_cursor;
    if (nz)
        DISABLE_FAIL;

    //DEBUG("Zu: ");
    //DEBUG_HEXDUMP((Zu + sizeof(Zu) - entropy_bytes), entropy_bytes);
    //fflush(stdout);

    phase = 2;
    return true;
}

// --------------------------------------------------------------------------------------------------------------------------------
// pp.85,93 NIST SP 800-56B: KAS2 V_CRT_PEM||\0||Cv||MacTagV
bool SP800_56B_KAS2::V_Phase_3(uint8_t * cryptogram_out, size_t & out_len)
{
    size_t out_len_copy = out_len;
    out_len = 0;

    // incoming assertions
    if (!Assert_V(2))
        DISABLE_FAIL;
    if (!cryptogram_out)
        DISABLE_FAIL;
    size_t V_CRT_PEM_len = V_CRT_PEM_LEN;
    size_t required = V_CRT_PEM_len + 1/*nul*/ + sizeof(Cu) + sizeof(MacTagV);
    if (out_len_copy < required)
        DISABLE_FAIL;

    // perform...
    // EXTRACT U's PUBLIC KEY FROM CERTIFICATE
    AutoHeapBuffer auto_U_PUB_PEM(IENVELOPE::RSA_2048_PEM_PUB_SIZE);
    if (!auto_U_PUB_PEM.u8Ptr())
        DISABLE_FAIL;
    Auto_x509_crt auto_x509_crt;
    if (!auto_x509_crt.Ptr())
        DISABLE_FAIL;
    size_t U_CRT_PEM_len = U_CRT_PEM_LEN;
    int rc_parse_crt = x509_crt_parse(auto_x509_crt.Ptr(), reinterpret_cast<const unsigned char *>(U_CRT_PEM), U_CRT_PEM_len);
    if (rc_parse_crt)
        DISABLE_FAIL;
    x509_crt * leaf = auto_x509_crt.Ptr();
    while (leaf->next)
        leaf = leaf->next;
    // INSURE LEAF CERT KEY USAGE IS KU_KEY_ENCIPHERMENT
    if ( (leaf->key_usage & KU_KEY_ENCIPHERMENT) == 0)
    {
        DISABLE_FAIL;
    }
    int rc_write_pem = pk_write_pubkey_pem(&(leaf->pk), auto_U_PUB_PEM.u8Ptr(), auto_U_PUB_PEM.Len());
    if (rc_write_pem)
        DISABLE_FAIL;
    // LOAD V's PEM PUBLIC KEY INTO A PK CONTEXT
    Auto_pk_context  auto_pk_context;
    if (!auto_pk_context.Ptr())
        DISABLE_FAIL;
    int rc_key_parse = pk_parse_public_key(auto_pk_context.Ptr(), auto_U_PUB_PEM.u8Ptr(), auto_U_PUB_PEM.Len());
    if (rc_key_parse)
        DISABLE_FAIL;
    if (auto_pk_context.Ptr()->pk_info->type != POLARSSL_PK_RSA)
        DISABLE_FAIL;
    // GET Zv, V's RANDOM NUMBER
    // NOTE: buffer is same size as RSA modulus, and in big-endian.
    //       place entropy at the far side of the buffer - i.e. smaller number than possible.
    SecureClearBuffer(Zv, sizeof(Zv) - entropy_bytes);
    uint8_t * Zv_entropy = Zv + sizeof(Zv) - entropy_bytes;
    int rc_rand = lumi_random(NULL/*p_rng*/, Zv_entropy, entropy_bytes);
    if (rc_rand)
        DISABLE_FAIL;

    //DEBUG("Zv: ");
    //DEBUG_HEXDUMP(Zv_entropy, entropy_bytes);
    //fflush(stdout);

    // RAW RSA OF Zv->Cv USING V's PUBLIC KEY
    int rc_public = rsa_public_op(mode, reinterpret_cast<rsa_context *>(auto_pk_context.Ptr()->pk_ctx),
                                  lumi_random/*f_rng*/, NULL/*p_rng*/, Zv, Cv);
    if (rc_public)
        DISABLE_FAIL;
    // run the SP800 108 KDF CTR HMAC SHA algorithm to derive actual KAS2 MAC key and session keys
    int rc_kdf = SP800_108_KDF_CTR_HMAC_SHA(POLARSSL_MD_SHA256, Zu, sizeof(Zu), Zv, sizeof(Zv),
                                            U_CRT_PEM, U_CRT_PEM_LEN, V_CRT_PEM, V_CRT_PEM_LEN,
                                            reinterpret_cast<const unsigned char *>(KDF_labels), strlen(KDF_labels),
                                            Ko, sizeof(Ko));
    if (rc_kdf)
        DISABLE_FAIL;
    // WE DO NOT NEED Zu OR Zv anymore...
    SecureClearBuffer(Zu, sizeof(Zu));
    SecureClearBuffer(Zv, sizeof(Zv));
    // COMPUTE MacTagV
    bool rc_mactag = Compute_MacTag(MacTagV, "KC_2_V", V_CRT_PEM, V_CRT_PEM_LEN, U_CRT_PEM, U_CRT_PEM_LEN, Cv, Cu, Ko/*location of MacKey*/, 32);
    if (!rc_mactag)
        DISABLE_FAIL;

    // SERIALIZE OUT
    uint8_t * cryptogram_out_cursor = cryptogram_out;
    WRITE_PTR(cryptogram_out_cursor, V_CRT_PEM, V_CRT_PEM_len);
    WRITE_PTR(cryptogram_out_cursor, "", 1);
    WRITE_PTR(cryptogram_out_cursor, Cv, sizeof(Cv));
    WRITE_PTR(cryptogram_out_cursor, MacTagV, sizeof(MacTagV));
    if (cryptogram_out_cursor - cryptogram_out != static_cast<ssize_t>(required))
        DISABLE_FAIL;

    out_len = required;

    phase = 3;
    return true;
}

// --------------------------------------------------------------------------------------------------------------------------------
// pp.85,93 NIST SP 800-56B: KAS2 V_CRT_PEM||\0||Cv||MacTagV
bool SP800_56B_KAS2::U_Phase_4(const uint8_t * cryptogram_in,  size_t   in_len)
{
    // incoming assertions
    if (!Assert_U(1))
        DISABLE_FAIL;
    if (!cryptogram_in)
        DISABLE_FAIL;

    oX509 oCertProcessor;

    // perform:
    // LOCATE NUL BYTE
    const uint8_t * in_cursor = cryptogram_in;
    while (in_len)
    {
        if (!(*in_cursor))
            break;
        in_cursor++;
        in_len--;
    }
    if (*in_cursor != '\0')
        DISABLE_FAIL;
    size_t xpct_cert_len = V_CRT_PEM_LEN;
    // DO WE HAVE A PINNED CERT FOR V?
    if (xpct_cert_len > 0)
    {
        // VERIFY THAT SENT CERT MATCHES PINNED CERT
        size_t rcvd_cert_len = in_cursor - cryptogram_in;
        if (rcvd_cert_len != xpct_cert_len)
            DISABLE_FAIL;
        if (strncmp(reinterpret_cast<const char *>(cryptogram_in), V_CRT_PEM, rcvd_cert_len))
            DISABLE_FAIL;
    }
    in_cursor++;
    in_len--;
    if (in_len != modulus_bytes + MacTag_len)
        DISABLE_FAIL;
    memcpy(Cv, in_cursor, modulus_bytes);
    // EXTRACT U's PRIVATE KEY INTO A PK CONTEXT
    Auto_pk_context auto_pk_context;
    if (!auto_pk_context.Ptr())
        DISABLE_FAIL;
    int rc_key_parse = pk_parse_key(auto_pk_context.Ptr(), reinterpret_cast<const unsigned char *>(U_PRV_PEM), U_PRV_PEM_LEN,
                                    NULL/*pw*/, 0/*pwlen*/);
    if (rc_key_parse)
        DISABLE_FAIL;
    if (auto_pk_context.Ptr()->pk_info->type != POLARSSL_PK_RSA)
        DISABLE_FAIL;
    // RAW RSA OF Cv->Zv USING U's PRIVATE KEY
    Auto_rsa_context auto_rsa_context;
    if (!auto_rsa_context.Ptr())
        DISABLE_FAIL;
    rsa_copy(auto_rsa_context.Ptr(), reinterpret_cast<rsa_context *>(auto_pk_context.Ptr()->pk_ctx));
    int rc_private = rsa_private_op(mode, auto_rsa_context.Ptr(), lumi_random/*f_rng*/, NULL/*p_rng*/, in_cursor/*Cv*/, Zv);
    if (rc_private)
        DISABLE_FAIL;
    // ASSURE PREFIX OF Zv IS ALL ZEROS
    uint8_t nz = 0;
    uint8_t * Zv_cursor = Zv;
    for (size_t i = 0; i < sizeof(Zv) - entropy_bytes; i++, Zv_cursor++)
        nz |= *Zv_cursor;
    if (nz)
        DISABLE_FAIL;

    //DEBUG("Zv: ");
    //DEBUG_HEXDUMP((Zv + sizeof(Zv) - entropy_bytes), entropy_bytes);
    //fflush(stdout);

    // run the SP800 108 KDF CTR HMAC SHA algorithm to derive actual KAS2 MAC key and session keys
    int rc_kdf = SP800_108_KDF_CTR_HMAC_SHA(POLARSSL_MD_SHA256, Zu, sizeof(Zu), Zv, sizeof(Zv),
                                            U_CRT_PEM, U_CRT_PEM_LEN, V_CRT_PEM, V_CRT_PEM_LEN,
                                            reinterpret_cast<const unsigned char *>(KDF_labels), strlen(KDF_labels),
                                            Ko, sizeof(Ko));
    if (rc_kdf)
        DISABLE_FAIL;
    // WE DO NOT NEED Zu OR Zv anymore...
    SecureClearBuffer(Zu, sizeof(Zu));
    SecureClearBuffer(Zv, sizeof(Zv));
    // COMPUTE MacTagV
    bool rc_mactag = Compute_MacTag(MacTagV, "KC_2_V", V_CRT_PEM, V_CRT_PEM_LEN, U_CRT_PEM, U_CRT_PEM_LEN, Cv, Cu, Ko/*location of MacKey*/, 32);
    if (!rc_mactag)
        DISABLE_FAIL;
    in_cursor += modulus_bytes;
    if (SecureCompareBuffer(in_cursor, MacTagV, MacTag_len))
        DISABLE_FAIL;
    phase = 4;
    return true;
}

// --------------------------------------------------------------------------------------------------------------------------------
// pp.93    NIST SP 800-56B: KAS2 MacTagU
bool SP800_56B_KAS2::U_Phase_5(uint8_t * cryptogram_out, size_t & out_len)
{
    size_t out_len_copy = out_len;
    out_len = 0;

    if (!Assert_U(4))
        DISABLE_FAIL;
    if (!cryptogram_out)
        DISABLE_FAIL;
    size_t required = MacTag_len;
    if (out_len_copy < required)
        DISABLE_FAIL;

    // perform...
    // COMPUTE MacTagU
    bool rc_mactag = Compute_MacTag(MacTagU, "KC_2_U", U_CRT_PEM, U_CRT_PEM_LEN, V_CRT_PEM, V_CRT_PEM_LEN, Cu, Cv, Ko, 32);
    if (!rc_mactag)
        DISABLE_FAIL;
    // WE DO NOT NEED Cu OR Cv anymore...
    SecureClearBuffer(Cu, sizeof(Cu));
    SecureClearBuffer(Cv, sizeof(Cv));

    // SERIALIZE OUT
    uint8_t * cryptogram_out_cursor = cryptogram_out;
    WRITE_PTR(cryptogram_out_cursor, MacTagU, sizeof(MacTagU));
    if (cryptogram_out_cursor - cryptogram_out != static_cast<ssize_t>(required))
        DISABLE_FAIL;

    out_len = required;

    // clear MacKey
    Clear_Intermediate_Keying_Material();
    phase = 5;  // complete...
    return true;
}

// --------------------------------------------------------------------------------------------------------------------------------
// pp.93    NIST SP 800-56B: KAS2 MacTagU
bool SP800_56B_KAS2::V_Phase_6(const uint8_t * cryptogram_in, size_t in_len)
{
    if (!Assert_V(3))
        DISABLE_FAIL;
    if (!cryptogram_in)
        DISABLE_FAIL;
    if (in_len != MacTag_len)
        DISABLE_FAIL;

    // perform...
    // COMPUTE MacTagU
    bool rc_mactag = Compute_MacTag(MacTagU, "KC_2_U", U_CRT_PEM, U_CRT_PEM_LEN, V_CRT_PEM, V_CRT_PEM_LEN, Cu, Cv, Ko, 32);
    if (!rc_mactag)
        DISABLE_FAIL;
    // WE DO NOT NEED Cu OR Cv anymore...
    SecureClearBuffer(Cu, sizeof(Cu));
    SecureClearBuffer(Cv, sizeof(Cv));
    if (SecureCompareBuffer(cryptogram_in, MacTagU, MacTag_len))
        DISABLE_FAIL;

    // clear MacKey
    Clear_Intermediate_Keying_Material();
    phase = 6;  // complete...
    return true;
}

// ================================================================================================================================
