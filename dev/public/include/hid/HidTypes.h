#ifndef __HIDTYPES_H__
#define __HIDTYPES_H__

#include <vector>
#include <string>

#ifndef UNUSED
    #define UNUSED(x)                  (void) (x);
#endif

namespace hid {

namespace types {

    #define IN
    #define OUT
    #define INOUT
    #define MANDATORY
    #define OPTIONAL

    typedef std::string                string_t;
    typedef std::vector<uint8_t>       storage_t;

}

}

#endif
