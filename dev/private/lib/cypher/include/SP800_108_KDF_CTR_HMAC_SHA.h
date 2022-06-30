#include "polarssl/md.h"

extern int SP800_108_KDF_CTR_HMAC_SHA(md_type_t             hashid,
                                      const unsigned char * Ki_host,
                                      size_t                Ki_host_len,
                                      const unsigned char * Ki_dev,
                                      size_t                Ki_dev_len,
                                      const          char * Cert_host,
                                      size_t                Cert_host_len,
                                      const          char * Cert_dev,
                                      size_t                Cert_dev_len,
                                      const unsigned char * Labels,
                                      size_t                Labels_len,
                                            unsigned char * Ko,
                                      size_t                Ko_len);
extern bool SP800_108_KDF_CTR_HMAC_SHA_self_test();
