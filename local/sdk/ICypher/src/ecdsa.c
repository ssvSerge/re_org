/*
 *  Elliptic curve DSA
 *
 *  Copyright (C) 2006-2014, ARM Limited, All Rights Reserved
 *
 *  This file is part of mbed TLS (https://tls.mbed.org)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

/*
 * References:
 *
 * SEC1 http://www.secg.org/index.php?action=secg,docs_secg
 */

#if !defined(POLARSSL_CONFIG_FILE)
#include "polarssl/config.h"
#else
#include POLARSSL_CONFIG_FILE
#endif

#if defined(POLARSSL_ECDSA_C)

#include "polarssl/ecdsa.h"
#include "polarssl/asn1write.h"

#include <string.h>

#if defined(POLARSSL_ECDSA_DETERMINISTIC)
#include "polarssl/hmac_drbg.h"
#endif

// ============================================================================================================================
// SANG
#if defined(POLARSSL_SELF_TEST)
#if defined(POLARSSL_PLATFORM_C)
#include "polarssl/platform.h"
#else
#include <stdio.h>
#define polarssl_printf printf
#endif /* POLARSSL_PLATFORM_C */
#endif /* POLARSSL_SELF_TEST */
#include "lumi_random.h"
// SANG
// ============================================================================================================================


#if defined(POLARSSL_ECDSA_DETERMINISTIC)
/*
 * This a hopefully temporary compatibility function.
 *
 * Since we can't ensure the caller will pass a valid md_alg before the next
 * interface change, try to pick up a decent md by size.
 *
 * Argument is the minimum size in bytes of the MD output.
 */
static const md_info_t *md_info_by_size( size_t min_size )
{
    const md_info_t *md_cur, *md_picked = NULL;
    const int *md_alg;

    for( md_alg = md_list(); *md_alg != 0; md_alg++ )
    {
        if( ( md_cur = md_info_from_type( (md_type_t) *md_alg ) ) == NULL ||
            (size_t) md_cur->size < min_size ||
            ( md_picked != NULL && md_cur->size > md_picked->size ) )
            continue;

        md_picked = md_cur;
    }

    return( md_picked );
}
#endif /* POLARSSL_ECDSA_DETERMINISTIC */

/*
 * Derive a suitable integer for group grp from a buffer of length len
 * SEC1 4.1.3 step 5 aka SEC1 4.1.4 step 3
 */
static int derive_mpi( const ecp_group *grp, mpi *x,
                       const unsigned char *buf, size_t blen )
{
    int ret;
    size_t n_size = ( grp->nbits + 7 ) / 8;
    size_t use_size = blen > n_size ? n_size : blen;

    MPI_CHK( mpi_read_binary( x, buf, use_size ) );
    if( use_size * 8 > grp->nbits )
        MPI_CHK( mpi_shift_r( x, use_size * 8 - grp->nbits ) );

    /* While at it, reduce modulo N */
    if( mpi_cmp_mpi( x, &grp->N ) >= 0 )
        MPI_CHK( mpi_sub_mpi( x, x, &grp->N ) );

cleanup:
    return( ret );
}

/*
 * Compute ECDSA signature of a hashed message (SEC1 4.1.3)
 * Obviously, compared to SEC1 4.1.3, we skip step 4 (hash message)
 */
int ecdsa_sign( ecp_group *grp, mpi *r, mpi *s,
                const mpi *d, const unsigned char *buf, size_t blen,
                int (*f_rng)(void *, unsigned char *, size_t), void *p_rng )
{
    // ============================================================================================================================
    // SANG
    extern int lumi_random_test_mode_remain(void);
    int test_mode_remain = lumi_random_test_mode_remain();
    // SANG
    // ============================================================================================================================

    int ret, key_tries, sign_tries, blind_tries;
    ecp_point R;
    mpi k, e, t;

    /* Fail cleanly on curves such as Curve25519 that can't be used for ECDSA */
    if( grp->N.p == NULL )
        return( POLARSSL_ERR_ECP_BAD_INPUT_DATA );

    ecp_point_init( &R );
    mpi_init( &k ); mpi_init( &e ); mpi_init( &t );

    sign_tries = 0;
    do
    {
        /*
         * Steps 1-3: generate a suitable ephemeral keypair
         * and set r = xR mod n
         */
        key_tries = 0;
        do
        {
            MPI_CHK(ecp_gen_keypair(grp, &k, &R, f_rng, p_rng));
            MPI_CHK(mpi_mod_mpi(r, &R.X, &grp->N));

            if (key_tries++ > 10)
            {
                ret = POLARSSL_ERR_ECP_RANDOM_FAILED;
                goto cleanup;
            }
        }
        while( mpi_cmp_int( r, 0 ) == 0 );

        /*
         * Step 5: derive MPI from hashed message
         */
        MPI_CHK(derive_mpi(grp, &e, buf, blen));

        /*
         * Generate a random value to blind inv_mod in next step,
         * avoiding a potential timing leak.
         */
        // ============================================================================================================================
        // SANG
        if (test_mode_remain >= 0)
        {
            // avoid blinding for test mode
            mpi_lset(&t, 1);
        }
        // SANG
        // ============================================================================================================================

        else
        {
        
            blind_tries = 0;
            do
            {
                size_t n_size = (grp->nbits + 7) / 8;
                MPI_CHK(mpi_fill_random(&t, n_size, f_rng, p_rng));
                MPI_CHK(mpi_shift_r(&t, 8 * n_size - grp->nbits));

                /* See ecp_gen_keypair() */
                if (++blind_tries > 30)
                    return(POLARSSL_ERR_ECP_RANDOM_FAILED);
        }
        while( mpi_cmp_int( &t, 1 ) < 0 ||
                mpi_cmp_mpi(&t, &grp->N) >= 0);
        }

        /*
         * Step 6: compute s = (e + r * d) / k = t (e + rd) / (kt) mod n
         */
        MPI_CHK( mpi_mul_mpi( s, r, d ) );
        MPI_CHK( mpi_add_mpi( &e, &e, s ) );
        MPI_CHK( mpi_mul_mpi( &e, &e, &t ) );
        MPI_CHK( mpi_mul_mpi( &k, &k, &t ) );
        MPI_CHK( mpi_inv_mod( s, &k, &grp->N ) );
        MPI_CHK( mpi_mul_mpi( s, s, &e ) );
        MPI_CHK( mpi_mod_mpi( s, s, &grp->N ) );

        if( sign_tries++ > 10 )
        {
            ret = POLARSSL_ERR_ECP_RANDOM_FAILED;
            goto cleanup;
        }
    }
    while( mpi_cmp_int( s, 0 ) == 0 );

cleanup:
    ecp_point_free( &R );
    mpi_free( &k ); mpi_free( &e ); mpi_free( &t );

    return( ret );
}

#if defined(POLARSSL_ECDSA_DETERMINISTIC)
/*
 * Deterministic signature wrapper
 */
int ecdsa_sign_det( ecp_group *grp, mpi *r, mpi *s,
                    const mpi *d, const unsigned char *buf, size_t blen,
                    md_type_t md_alg )
{
    int ret;
    hmac_drbg_context rng_ctx;
    unsigned char data[2 * POLARSSL_ECP_MAX_BYTES];
    size_t grp_len = ( grp->nbits + 7 ) / 8;
    const md_info_t *md_info;
    mpi h;

    /* Temporary fallback */
    if( md_alg == POLARSSL_MD_NONE )
        md_info = md_info_by_size( blen );
    else
        md_info = md_info_from_type( md_alg );

    if( md_info == NULL )
        return( POLARSSL_ERR_ECP_BAD_INPUT_DATA );

    mpi_init( &h );
    memset( &rng_ctx, 0, sizeof( hmac_drbg_context ) );

    /* Use private key and message hash (reduced) to initialize HMAC_DRBG */
    MPI_CHK( mpi_write_binary( d, data, grp_len ) );
    MPI_CHK( derive_mpi( grp, &h, buf, blen ) );
    MPI_CHK( mpi_write_binary( &h, data + grp_len, grp_len ) );
    hmac_drbg_init_buf( &rng_ctx, md_info, data, 2 * grp_len );

    ret = ecdsa_sign( grp, r, s, d, buf, blen,
                      hmac_drbg_random, &rng_ctx );

cleanup:
    hmac_drbg_free( &rng_ctx );
    mpi_free( &h );

    return( ret );
}
#endif /* POLARSSL_ECDSA_DETERMINISTIC */

/*
 * Verify ECDSA signature of hashed message (SEC1 4.1.4)
 * Obviously, compared to SEC1 4.1.3, we skip step 2 (hash message)
 */
int ecdsa_verify( ecp_group *grp,
                  const unsigned char *buf, size_t blen,
                  const ecp_point *Q, const mpi *r, const mpi *s)
{
    int ret;
    mpi e, s_inv, u1, u2;
    ecp_point R, P;

    ecp_point_init( &R ); ecp_point_init( &P );
    mpi_init( &e ); mpi_init( &s_inv ); mpi_init( &u1 ); mpi_init( &u2 );

    /* Fail cleanly on curves such as Curve25519 that can't be used for ECDSA */
    if( grp->N.p == NULL )
        return( POLARSSL_ERR_ECP_BAD_INPUT_DATA );

    /*
     * Step 1: make sure r and s are in range 1..n-1
     */
    if( mpi_cmp_int( r, 1 ) < 0 || mpi_cmp_mpi( r, &grp->N ) >= 0 ||
        mpi_cmp_int( s, 1 ) < 0 || mpi_cmp_mpi( s, &grp->N ) >= 0 )
    {
        ret = POLARSSL_ERR_ECP_VERIFY_FAILED;
        goto cleanup;
    }

    /*
     * Additional precaution: make sure Q is valid
     */
    MPI_CHK( ecp_check_pubkey( grp, Q ) );

    /*
     * Step 3: derive MPI from hashed message
     */
    MPI_CHK( derive_mpi( grp, &e, buf, blen ) );

    /*
     * Step 4: u1 = e / s mod n, u2 = r / s mod n
     */
    MPI_CHK( mpi_inv_mod( &s_inv, s, &grp->N ) );

    MPI_CHK( mpi_mul_mpi( &u1, &e, &s_inv ) );
    MPI_CHK( mpi_mod_mpi( &u1, &u1, &grp->N ) );

    MPI_CHK( mpi_mul_mpi( &u2, r, &s_inv ) );
    MPI_CHK( mpi_mod_mpi( &u2, &u2, &grp->N ) );

    /*
     * Step 5: R = u1 G + u2 Q
     *
     * Since we're not using any secret data, no need to pass a RNG to
     * ecp_mul() for countermesures.
     */
    MPI_CHK( ecp_mul( grp, &R, &u1, &grp->G, NULL, NULL ) );
    MPI_CHK( ecp_mul( grp, &P, &u2, Q, NULL, NULL ) );
    MPI_CHK( ecp_add( grp, &R, &R, &P ) );

    if( ecp_is_zero( &R ) )
    {
        ret = POLARSSL_ERR_ECP_VERIFY_FAILED;
        goto cleanup;
    }

    /*
     * Step 6: convert xR to an integer (no-op)
     * Step 7: reduce xR mod n (gives v)
     */
    MPI_CHK( mpi_mod_mpi( &R.X, &R.X, &grp->N ) );

    /*
     * Step 8: check if v (that is, R.X) is equal to r
     */
    if( mpi_cmp_mpi( &R.X, r ) != 0 )
    {
        ret = POLARSSL_ERR_ECP_VERIFY_FAILED;
        goto cleanup;
    }

cleanup:
    ecp_point_free( &R ); ecp_point_free( &P );
    mpi_free( &e ); mpi_free( &s_inv ); mpi_free( &u1 ); mpi_free( &u2 );

    return( ret );
}

/*
 * RFC 4492 page 20:
 *
 *     Ecdsa-Sig-Value ::= SEQUENCE {
 *         r       INTEGER,
 *         s       INTEGER
 *     }
 *
 * Size is at most
 *    1 (tag) + 1 (len) + 1 (initial 0) + ECP_MAX_BYTES for each of r and s,
 *    twice that + 1 (tag) + 2 (len) for the sequence
 * (assuming ECP_MAX_BYTES is less than 126 for r and s,
 * and less than 124 (total len <= 255) for the sequence)
 */
#if POLARSSL_ECP_MAX_BYTES > 124
#error "POLARSSL_ECP_MAX_BYTES bigger than expected, please fix MAX_SIG_LEN"
#endif
#define MAX_SIG_LEN ( 3 + 2 * ( 3 + POLARSSL_ECP_MAX_BYTES ) )

/*
 * Convert a signature (given by context) to ASN.1
 */
static int ecdsa_signature_to_asn1( ecdsa_context *ctx,
                                    unsigned char *sig, size_t *slen )
{
    int ret;
    unsigned char buf[MAX_SIG_LEN];
    unsigned char *p = buf + sizeof( buf );
    size_t len = 0;

    ASN1_CHK_ADD( len, asn1_write_mpi( &p, buf, &ctx->s ) );
    ASN1_CHK_ADD( len, asn1_write_mpi( &p, buf, &ctx->r ) );

    ASN1_CHK_ADD( len, asn1_write_len( &p, buf, len ) );
    ASN1_CHK_ADD( len, asn1_write_tag( &p, buf,
                                       ASN1_CONSTRUCTED | ASN1_SEQUENCE ) );

    memcpy( sig, p, len );
    *slen = len;

    return( 0 );
}

/*
 * Compute and write signature
 */
int ecdsa_write_signature( ecdsa_context *ctx,
                           const unsigned char *hash, size_t hlen,
                           unsigned char *sig, size_t *slen,
                           int (*f_rng)(void *, unsigned char *, size_t),
                           void *p_rng )
{
    int ret;

    if( ( ret = ecdsa_sign( &ctx->grp, &ctx->r, &ctx->s, &ctx->d,
                            hash, hlen, f_rng, p_rng ) ) != 0 )
    {
        return( ret );
    }

    return( ecdsa_signature_to_asn1( ctx, sig, slen ) );
}

#if defined(POLARSSL_ECDSA_DETERMINISTIC)
/*
 * Compute and write signature deterministically
 */
int ecdsa_write_signature_det( ecdsa_context *ctx,
                               const unsigned char *hash, size_t hlen,
                               unsigned char *sig, size_t *slen,
                               md_type_t md_alg )
{
    int ret;

    if( ( ret = ecdsa_sign_det( &ctx->grp, &ctx->r, &ctx->s, &ctx->d,
                                hash, hlen, md_alg ) ) != 0 )
    {
        return( ret );
    }

    return( ecdsa_signature_to_asn1( ctx, sig, slen ) );
}
#endif /* POLARSSL_ECDSA_DETERMINISTIC */

/*
 * Read and check signature
 */
int ecdsa_read_signature( ecdsa_context *ctx,
                          const unsigned char *hash, size_t hlen,
                          const unsigned char *sig, size_t slen )
{
    int ret;
    unsigned char *p = (unsigned char *) sig;
    const unsigned char *end = sig + slen;
    size_t len;

    if( ( ret = asn1_get_tag( &p, end, &len,
                    ASN1_CONSTRUCTED | ASN1_SEQUENCE ) ) != 0 )
    {
        return( POLARSSL_ERR_ECP_BAD_INPUT_DATA + ret );
    }

    if( p + len != end )
        return( POLARSSL_ERR_ECP_BAD_INPUT_DATA +
                POLARSSL_ERR_ASN1_LENGTH_MISMATCH );

    if( ( ret = asn1_get_mpi( &p, end, &ctx->r ) ) != 0 ||
        ( ret = asn1_get_mpi( &p, end, &ctx->s ) ) != 0 )
        return( POLARSSL_ERR_ECP_BAD_INPUT_DATA + ret );

    if( ( ret = ecdsa_verify( &ctx->grp, hash, hlen,
                              &ctx->Q, &ctx->r, &ctx->s ) ) != 0 )
        return( ret );

    if( p != end )
        return( POLARSSL_ERR_ECP_SIG_LEN_MISMATCH );

    return( 0 );
}

/*
 * Generate key pair
 */
int ecdsa_genkey( ecdsa_context *ctx, ecp_group_id gid,
                  int (*f_rng)(void *, unsigned char *, size_t), void *p_rng )
{
    return( ecp_use_known_dp( &ctx->grp, gid ) ||
            ecp_gen_keypair( &ctx->grp, &ctx->d, &ctx->Q, f_rng, p_rng ) );
}

/*
 * Set context from an ecp_keypair
 */
int ecdsa_from_keypair( ecdsa_context *ctx, const ecp_keypair *key )
{
    int ret;

    if( ( ret = ecp_group_copy( &ctx->grp, &key->grp ) ) != 0 ||
        ( ret = mpi_copy( &ctx->d, &key->d ) ) != 0 ||
        ( ret = ecp_copy( &ctx->Q, &key->Q ) ) != 0 )
    {
        ecdsa_free( ctx );
    }

    return( ret );
}

/*
 * Initialize context
 */
void ecdsa_init( ecdsa_context *ctx )
{
    ecp_group_init( &ctx->grp );
    mpi_init( &ctx->d );
    ecp_point_init( &ctx->Q );
    mpi_init( &ctx->r );
    mpi_init( &ctx->s );
}

/*
 * Free context
 */
void ecdsa_free( ecdsa_context *ctx )
{
    ecp_group_free( &ctx->grp );
    mpi_free( &ctx->d );
    ecp_point_free( &ctx->Q );
    mpi_free( &ctx->r );
    mpi_free( &ctx->s );
}

#if defined(POLARSSL_SELF_TEST)
// ============================================================================================================================
// SANG
//int ecdsa_self_test( int verbose )
//{
//    ((void) verbose );
//    return( 0 );
//}
#define VERBOSE
#if defined(VERBOSE)
static void dump_buf(const char *title, unsigned char *buf, size_t len)
{
    size_t i;

    polarssl_printf("%s", title);
    for (i = 0; i < len; i++)
        polarssl_printf("%c%c", "0123456789ABCDEF"[buf[i] / 16],
        "0123456789ABCDEF"[buf[i] % 16]);
    polarssl_printf("\n");
}

static void dump_pubkey(const char *title, ecdsa_context *key)
{
    unsigned char buf[300];
    size_t len;

    if (ecp_point_write_binary(&key->grp, &key->Q,
        POLARSSL_ECP_PF_UNCOMPRESSED, &len, buf, sizeof buf) != 0)
    {
        polarssl_printf("internal error\n");
        return;
    }

    dump_buf(title, buf, len);
}
#else
#define dump_buf( a, b, c )
#define dump_pubkey( a, b )
#endif


/*
 * Checkup routine
 */
int ecdsa_self_test( int verbose )
{

    int ret;
    ecdsa_context ctx_sign, ctx_verify;
    //entropy_context entropy;
    //ctr_drbg_context ctr_drbg;
    unsigned char hash[] = "This should be the hash of a message.";
    unsigned char sig[512];
    size_t sig_len;
    //const char *pers = "ecdsa";

    ecdsa_init(&ctx_sign);
    ecdsa_init(&ctx_verify);

    memset(sig, 0, sizeof(sig));
    ret = 1;



    /*
    * Generate a key pair for signing
    */
    /*polarssl_printf("\n  . Seeding the random number generator...");
    fflush(stdout);

    entropy_init(&entropy);
    if ((ret = ctr_drbg_init(&ctr_drbg, entropy_func, &entropy,
        (const unsigned char *)pers,
        strlen(pers))) != 0)
    {
        polarssl_printf(" failed\n  ! ctr_drbg_init returned %d\n", ret);
        goto exit;
    }
*/
    polarssl_printf(" ok\n  . Generating key pair...");
    fflush(stdout);

    if ((ret = ecdsa_genkey(&ctx_sign, POLARSSL_ECP_DP_SECP192R1,
        lumi_random, NULL) != 0))
    {
        polarssl_printf(" failed\n  ! ecdsa_genkey returned %d\n", ret);
        goto exit;
    }

    polarssl_printf(" ok (key size: %d bits)\n", (int)ctx_sign.grp.pbits);

    dump_pubkey("  + Public key: ", &ctx_sign);

    /*
    * Sign some message hash
    */
    polarssl_printf("  . Signing message...");
    fflush(stdout);

    if ((ret = ecdsa_write_signature(&ctx_sign,
        hash, sizeof(hash),
        sig, &sig_len,
        lumi_random, NULL)) != 0)
    {
        polarssl_printf(" failed\n  ! ecdsa_genkey returned %d\n", ret);
        goto exit;
    }
    polarssl_printf(" ok (signature length = %u)\n", (unsigned int)sig_len);

    dump_buf("  + Hash: ", hash, sizeof hash);
    dump_buf("  + Signature: ", sig, sig_len);

    /*
    * Signature is serialized as defined by RFC 4492 p. 20,
    * but one can also access 'r' and 's' directly from the context
    */
#ifdef POLARSSL_FS_IO
    mpi_write_file( "    r = ", &ctx_sign.r, 16, NULL );
    mpi_write_file("    s = ", &ctx_sign.s, 16, NULL);
#endif

    /*
    * Transfer public information to verifying context
    *
    * We could use the same context for verification and signatures, but we
    * chose to use a new one in order to make it clear that the verifying
    * context only needs the public key (Q), and not the private key (d).
    */
    polarssl_printf("  . Preparing verification context...");
    fflush(stdout);

    if ((ret = ecp_group_copy(&ctx_verify.grp, &ctx_sign.grp)) != 0)
    {
        polarssl_printf(" failed\n  ! ecp_group_copy returned %d\n", ret);
        goto exit;
    }

    if ((ret = ecp_copy(&ctx_verify.Q, &ctx_sign.Q)) != 0)
    {
        polarssl_printf(" failed\n  ! ecp_copy returned %d\n", ret);
        goto exit;
    }

    ret = 0;

    /*
    * Verify signature
    */
    polarssl_printf(" ok\n  . Verifying signature...");
    fflush(stdout);

    if ((ret = ecdsa_read_signature(&ctx_verify,
        hash, sizeof(hash),
        sig, sig_len)) != 0)
    {
        polarssl_printf(" failed\n  ! ecdsa_read_signature returned %d\n", ret);
        goto exit;
    }

    polarssl_printf(" ok\n");

exit:



    ecdsa_free(&ctx_verify);
    ecdsa_free(&ctx_sign);
    //ctr_drbg_free(&ctr_drbg);
    //entropy_free(&entropy);

    return(ret);

    //((void) verbose );
    //return( 0 );
}
// SANG
// ============================================================================================================================

#endif /* POLARSSL_SELF_TEST */

#endif /* POLARSSL_ECDSA_C */
