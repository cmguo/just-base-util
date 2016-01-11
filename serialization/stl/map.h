// map.h

#ifndef _UTIL_SERIALIZATION_STL_MAP_H_
#define _UTIL_SERIALIZATION_STL_MAP_H_

#include "util/serialization/Collection.h"
#include "util/serialization/SplitMember.h"
#include "util/serialization/stl/utility.h"

#include <map>

namespace util
{
    namespace serialization
    {

        template<
            class Archive, 
            class _Kty, 
            class _Ty, 
            class _Pr, 
            class _Alloc
        >
        struct is_collection<Archive, std::map<_Kty, _Ty, _Pr, _Alloc> >
            : boost::true_type
        {
        };

    }  // namespace serialization
} // namespace util

namespace std
{

    template<
        class Archive, 
        class _Kty, 
        class _Ty, 
        class _Pr, 
        class _Alloc
    >
    inline void serialize(
        Archive & ar,
        std::map<_Kty, _Ty, _Pr, _Alloc> & t)
    {
        util::serialization::serialize_collection(ar, t);
    }

} // namespace std

#endif // _UTIL_SERIALIZATION_STL_MAP_H_
