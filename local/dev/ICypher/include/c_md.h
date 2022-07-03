/**
 * \file md.h
 *
 * \brief Generic message digest wrapper
 *
 */
#pragma once

#include <string.h>
#include "IMemMgr.h"

class IMD
{
    public:
     virtual const int *md_list(void)=0;
    virtual const md_info_t *md_info_from_string(char *md_name)=0;
    virtual const md_info_t *md_info_from_type(md_type_t md_type)=0;
    virtual int md_init_ctx( md_context_t *ctx, md_info_t *md_info )=0;
    virtual int md_free_ctx( md_context_t *ctx )=0;
    virtual int md_starts( md_context_t *ctx )=0;
    virtual int md_update( md_context_t *ctx,  u8 *input, size_t ilen )=0;
    virtual int md_finish( md_context_t *ctx, u8 *output )=0;
    virtual int md(  md_info_t *md_info,  u8 *input, size_t ilen, u8 *output )=0;
    virtual int md_file(  md_info_t *md_info,  char *path, u8 *output )=0;
    virtual int md_hmac_starts( md_context_t *ctx,  u8 *key, size_t keylen )=0;
    virtual int md_hmac_update( md_context_t *ctx,  u8 *input, size_t ilen )=0;
    virtual int md_hmac_finish( md_context_t *ctx, u8 *output)=0;
    virtual int md_hmac_reset( md_context_t *ctx )=0;
    virtual int md_hmac(  md_info_t *md_info,  u8 *key, size_t keylen, u8 *input, size_t ilen, u8 *output )=0;
    virtual u8 md_get_size(  md_info_t *md_info )=0;
    virtual md_type_t md_get_type(  md_info_t *md_info )=0;
    virtual const char * md_get_name(md_info_t *md_info)=0;
};

class oMD : public IMD,  public MemoryBase
{

public:
                                oMD() {}
    virtual                     ~oMD() {}

     virtual const int *         md_list( void )
                                { return ::md_list(); }

    virtual const md_info_t *   md_info_from_string( char *md_name )
                                { return ::md_info_from_string(md_name); }
    virtual const md_info_t *   md_info_from_type( md_type_t md_type )
                                { return ::md_info_from_type(md_type); }

    virtual int                 md_init_ctx( md_context_t *ctx, md_info_t *md_info )
                                { return ::md_init_ctx(ctx, md_info); }
    virtual int                 md_free_ctx( md_context_t *ctx )
                                { return ::md_free_ctx(ctx); }
    virtual int                 md_starts( md_context_t *ctx )
                                { return ::md_starts(ctx); }
    virtual int                 md_update( md_context_t *ctx,  u8 *input, size_t ilen )
                                { return ::md_update(ctx, input, ilen ); }
    virtual int                 md_finish( md_context_t *ctx, u8 *output )
                                { return ::md_finish(ctx, output); }
    virtual int                 md(  md_info_t *md_info,  u8 *input, size_t ilen, u8 *output )
                                { return ::md(md_info, input, ilen, output); }
    virtual int                 md_file(  md_info_t *md_info,  char *path, u8 *output )
                                { return ::md_file(md_info, path, output ); }
    virtual int                 md_hmac_starts( md_context_t *ctx,  u8 *key, size_t keylen )
                                { return ::md_hmac_starts(ctx, key, keylen ); }
    virtual int                 md_hmac_update( md_context_t *ctx,  u8 *input, size_t ilen )
                                { return ::md_hmac_update(ctx, input, ilen); }
    virtual int                 md_hmac_finish( md_context_t *ctx, u8 *output)
                                { return ::md_hmac_finish(ctx, output); }
    virtual int                 md_hmac_reset( md_context_t *ctx )
                                { return ::md_hmac_reset(ctx ); }
    virtual int                 md_hmac(  md_info_t *md_info,  u8 *key, size_t keylen, u8 *input, size_t ilen, u8 *output )
                                { return ::md_hmac(md_info, key, keylen, input, ilen, output); }

    virtual u8                  md_get_size(  md_info_t *md_info )
                                { return ::md_get_size(md_info); }
    virtual md_type_t           md_get_type(  md_info_t *md_info )
                                { return ::md_get_type(md_info); }
    virtual const char *        md_get_name(  md_info_t *md_info )
                                { return ::md_get_name(md_info); }


private:

};

