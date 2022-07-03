#include "c_null.h"


bool oNULL::EncryptData(const u8 *in, u8 *out, int NBytesIn, int NBytesOut, const u8 *Key, int KeyLength, u8 *IV, int Mode)
{
    if (NBytesIn != NBytesOut){
        return false;
    }
    if(out == NULL || in == NULL) {
        return false;
    }
    memcpy(out, in, NBytesIn);
    return true;
}
bool oNULL::DecryptData(const u8 *in, u8 *out, int NBytesIn, int NBytesOut, const u8 *Key, int KeyLength, u8 *IV, int Mode)
{
    if (NBytesIn != NBytesOut){
        return false;
    }
    if(out == NULL || in == NULL) {
        return false;
    }
    memcpy(out, in, NBytesIn);
    return true;
}