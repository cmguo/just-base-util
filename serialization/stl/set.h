// set.h

#ifndef _UTIL_SERIALIZATION_STL_SET_H_
#define _UTIL_SERIALIZATION_STL_SET_H_

#include "util/serialization/Collection.h"
#include "util/serialization/SplitMember.h"

#include <set>

namespace util
{
    namespace serialization
    {

        template<
            class Archive, 
            class _Kty, 
            class _Pr, 
            class _Alloc
        >
        struct is_collection<Archive, std::set<_Kty, _Pr, _Alloc> >
            : boost::true_type
        {
        };

    }  // namespace serialization
} // namespace util

namespace std
{

    template<class Archive, class _Kty, class _Pr, class _Alloc>
    inline void serialize(
        Archive & ar,
        std::set<_Kty, _Pr, _Alloc> & t)
    {
        util::serialization::serialize_collection(ar, t);
    }

} // namespace std

#endif // _UTIL_SERIALIZATION_STL_SET_H_
