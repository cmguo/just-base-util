// Optional.h

#ifndef _UTIL_SERIALIZATION_OPTIONAL_H_
#define _UTIL_SERIALIZATION_OPTIONAL_H_

#include <boost/optional.hpp>

namespace util
{
    namespace serialization
    {

        template<
            class _T
        >
        struct is_primitive<boost::optional<_T> >
            : boost::true_type
        {
        };

    }  // namespace serialize
} // namespace util

#endif // _UTIL_SERIALIZATION_OPTIONAL_H_
