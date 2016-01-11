// deque.h

#ifndef _UTIL_SERIALIZATION_STL_DEQUE_H_
#define _UTIL_SERIALIZATION_STL_DEQUE_H_

#include "util/serialization/Collection.h"
#include "util/serialization/SplitMember.h"

#include <deque>

namespace util
{
    namespace serialization
    {

        template<
            class Archive, 
            class _Ty, 
            class _Alloc
        >
        struct is_collection<Archive, std::deque<_Ty, _Alloc> >
            : boost::true_type
        {
        };

    }  // namespace serialization
} // namespace util

namespace std
{

    template<
        class Archive, 
        class _Ty, 
        class _Alloc
    >
    inline void serialize(
        Archive & ar,
        std::deque<_Ty, _Alloc> & t)
    {
        util::serialization::serialize_collection(ar, t);
    }

} // namespace std

#endif // _UTIL_SERIALIZATION_STL_DEQUE_H_
