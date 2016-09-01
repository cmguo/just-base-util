// Ordered.h

#ifndef _UTIL_SERIALIZATION_ORDERED_H_
#define _UTIL_SERIALIZATION_ORDERED_H_

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
            class _Key, 
            typename _Ls, 
            typename _Cat
        >
        struct is_collection<Archive, framework::container::Ordered<_Ty, _Key, _Ls, _Cat> >
            : boost::true_type
        {
        };

    }  // namespace serialization
} // namespace util

namespace framework { namespace container {

    template<class Archive, class _Ty, class _Key, typename _Ls, typename _Cat>
    inline void serialize(
        Archive & ar,
        Ordered<_Ty, _Key, _Ls, _Cat> & t)
    {
        util::serialization::serialize_collection(ar, t);
    }

} }

#endif // _UTIL_SERIALIZATION_ORDERED_H_
