#ifndef BASE64_H_
#define BASE64_H_

#include <string.h>
#include <string>
#include <vector>
#include <deque>

using namespace std;


class Base64
{
public:
    Base64();
    virtual ~Base64();
    bool is_base64(unsigned char c);
    size_t decoded_size(const char *in);
    int decode(const char *in, unsigned char *out, size_t outlen);
    int isvalidchar(char c);
};

#endif
