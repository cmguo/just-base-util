// Uuid.h

#ifndef _UTIL_SERIALIZATION_NUMBER_BITS_24_H_
#define _UTIL_SERIALIZATION_NUMBER_BITS_24_H_

#include "util/serialization/Serialization.h"

#include <framework/system/NumberBits24.h>

namespace util
{
    namespace serialization
    {

        template<
        >
        struct is_primitive<framework::system::UInt24>
            : boost::true_type
        {
        };

    }
}

#endif // _UTIL_SERIALIZATION_NUMBER_BITS_24_H_
