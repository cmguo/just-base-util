// list.h

#ifndef _UTIL_SERIALIZATION_STL_LIST_H_
#define _UTIL_SERIALIZATION_STL_LIST_H_

#include "util/serialization/Collection.h"
#include "util/serialization/SplitMember.h"

#include <list>

namespace util
{
    namespace serialization
    {

        template<
            class Archive, 
            class _Ty, 
            class _Alloc
        >
        struct is_collection<Archive, std::list<_Ty, _Alloc> >
            : boost::true_type
        {
        };

    }  // namespace serialization
} // namespace util

namespace std
{

    template<class Archive, class _Ty, class _Alloc>
    inline void serialize(
        Archive & ar,
        std::list<_Ty, _Alloc> & t)
    {
        util::serialization::serialize_collection(ar, t);
    }

} // namespace std

#endif // _UTIL_SERIALIZATION_STL_LIST_H_
