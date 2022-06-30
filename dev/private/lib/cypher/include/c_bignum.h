/**
 * \file bignum.h
 *
 * \brief  Multi-precision integer library
 */
#pragma once
#include "c_bn_mul.h"
#include "IMemMgr.h"


#define POLARSSL_ERR_MPI_FILE_IO_ERROR                     -0x0002  /**< An error occurred while reading from or writing to a file. */
#define POLARSSL_ERR_MPI_BAD_INPUT_DATA                    -0x0004  /**< Bad input parameters to function. */
#define POLARSSL_ERR_MPI_INVALID_CHARACTER                 -0x0006  /**< There is an invalid character in the digit string. */
#define POLARSSL_ERR_MPI_BUFFER_TOO_SMALL                  -0x0008  /**< The buffer is too small to write to. */
#define POLARSSL_ERR_MPI_NEGATIVE_VALUE                    -0x000A  /**< The input arguments are negative or result in illegal output. */
#define POLARSSL_ERR_MPI_DIVISION_BY_ZERO                  -0x000C  /**< The input argument for division is zero, which is not allowed. */
#define POLARSSL_ERR_MPI_NOT_ACCEPTABLE                    -0x000E  /**< The input arguments are not acceptable. */
#define POLARSSL_ERR_MPI_MALLOC_FAILED                     -0x0010  /**< Memory allocation failed. */

#if 0
#define MPI_CHK(f) if( ( ret = f ) != 0 ) goto cleanup
#endif

/*
 * Maximum size MPIs are allowed to grow to in number of limbs.
 */
#define POLARSSL_MPI_MAX_LIMBS                             10000

/*
 * Maximum window size used for modular exponentiation. Default: 6
 * Minimum value: 1. Maximum value: 6.
 *
 * Result is an array of ( 2 << POLARSSL_MPI_WINDOW_SIZE ) MPIs used
 * for the sliding window calculation. (So 64 by default)
 *
 * Reduction in size, reduces speed.
 */
#define POLARSSL_MPI_WINDOW_SIZE                           6        /**< Maximum windows size used. */

#if 0
/*
 * Maximum size of MPIs allowed in bits and bytes for user-MPIs.
 * ( Default: 512 bytes => 4096 bits, Maximum tested: 2048 bytes => 16384 bits )
 *
 * Note: Calculations can results temporarily in larger MPIs. So the number
 * of limbs required (POLARSSL_MPI_MAX_LIMBS) is higher.
 */
#define POLARSSL_MPI_MAX_SIZE                              512      /**< Maximum number of bytes for usable MPIs. */
#endif


#define POLARSSL_MPI_MAX_BITS                              ( 8 * POLARSSL_MPI_MAX_SIZE )    /**< Maximum number of bits for usable MPIs. */


/**
 ** MPI structure
*/ 
#if 0
typedef struct
{
    int s;              /*!<  integer sign      */
    size_t n;           /*!<  total # of limbs  */
    t_uint *p;          /*!<  pointer to limbs  */
} mpi;
#endif

class oBIG : public IBIG, public MemoryBase
{
public:

	virtual	void 	mpi_init( mpi *X );
	virtual	void 	mpi_free( mpi *X );
	virtual	int 	mpi_grow( mpi *X, size_t nblimbs );
	virtual	int 	mpi_copy( mpi *X, mpi *Y );
	virtual	void 	mpi_swap( mpi *X, mpi *Y );
	virtual	int 	mpi_lset( mpi *X, t_sint z );
	virtual	int 	mpi_get_bit( mpi *X, size_t pos );
	virtual	int 	mpi_set_bit( mpi *X, size_t pos, u8 val );
	virtual	size_t 	mpi_lsb( mpi *X );
	virtual	size_t 	mpi_msb( mpi *X );
	virtual	size_t 	mpi_size( mpi *X );
	virtual	int 	mpi_read_string( mpi *X, int radix, const char *s );
	virtual	int 	mpi_write_string( mpi *X, int radix, char *s, size_t *slen );
	virtual	int 	mpi_read_binary( mpi *X, const u8 *buf, size_t buflen );
	virtual	int 	mpi_write_binary( mpi *X, u8 *buf, size_t buflen );
	virtual	int 	mpi_shift_l( mpi *X, size_t count );
	virtual	int 	mpi_shift_r( mpi *X, size_t count );
	virtual	int 	mpi_cmp_abs( mpi *X, mpi *Y );
	virtual	int 	mpi_cmp_mpi( mpi *X, mpi *Y );
	virtual	int 	mpi_cmp_int( mpi *X, t_sint z );
	virtual	int 	mpi_add_abs( mpi *X, mpi *A, mpi *B );
	virtual	int 	mpi_sub_abs( mpi *X, mpi *A, mpi *B );
	virtual	int 	mpi_add_mpi( mpi *X, mpi *A, mpi *B );
	virtual	int 	mpi_sub_mpi( mpi *X, mpi *A, mpi *B );
	virtual	int 	mpi_add_int( mpi *X, mpi *A, t_sint b );
	virtual	int 	mpi_sub_int( mpi *X, mpi *A, t_sint b );
	virtual	int 	mpi_mul_mpi( mpi *X, mpi *A, mpi *B );
	virtual	int 	mpi_mul_int( mpi *X, mpi *A, t_sint b );
	virtual	int 	mpi_div_mpi( mpi *Q, mpi *R, mpi *A, mpi *B );
	virtual	int 	mpi_div_int( mpi *Q, mpi *R, mpi *A, t_sint b );
	virtual	int 	mpi_mod_mpi( mpi *R, mpi *A, mpi *B );
	virtual	int 	mpi_mod_int( u32 *r, mpi *A, int b );
	virtual	int 	mpi_exp_mod( mpi *X, mpi *A, mpi *E, mpi *N, mpi *_RR );
	virtual	int 	mpi_fill_random( mpi *X, size_t size, void *p_rng );
	virtual	int 	mpi_gcd( mpi *G, mpi *A, mpi *B );
	virtual	int 	mpi_inv_mod( mpi *X, mpi *A, mpi *N );
	virtual	int 	mpi_is_prime( mpi *X, void *p_rng );
	virtual	int 	mpi_gen_prime( mpi *X, size_t nbits, int dh_flag, void *p_rng );
	virtual	bool 	mpi_self_test( void );

private:
	int  mpi_get_digit( t_uint *d, int radix, char c );
	int  mpi_write_hlp( mpi *X, int radix, char **p );
	void mpi_sub_hlp( size_t n, t_uint *s, t_uint *d );
	void mpi_mul_hlp( size_t i, t_uint *s, t_uint *d, t_uint b );
	void mpi_montg_init( t_uint *mm, mpi *N );
	void mpi_montmul( mpi *A, mpi *B, mpi *N, t_uint mm, mpi *T );
	void mpi_montred( mpi *A, mpi *N, t_uint mm, mpi *T );


//	mpi _B, TA, TB, Y, RR, T, W[ 2 << POLARSSL_MPI_WINDOW_SIZE ], Apos, X, Z, T1, T2, U;
	

//	mpi V, N, E, A, W1, V1, V2, R, TV, G, U2, U1, TU, TG ;	
};




