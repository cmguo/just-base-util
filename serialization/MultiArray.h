// MultiArray.h

#ifndef _UTIL_SERIALIZATION_MULTI_ARRAY_H_
#define _UTIL_SERIALIZATION_MULTI_ARRAY_H_

#include "util/serialization/Collection.h"
#include "util/serialization/SplitMember.h"

#include <framework/container/MultiArray.h>

namespace util
{
    namespace serialization
    {

        template<typename Archive, 
            typename Elem, typename Dim, typename Alloc
        >
        struct is_collection<Archive, framework::container::MultiArray<Elem, Dim, Alloc> >
            : boost::true_type
        {
        };

    }  // namespace serialization
} // namespace util

namespace framework { namespace container {

    template<typename Archive, 
        typename Elem, typename Dim, typename Alloc>
    inline void serialize(
        Archive & ar,
        MultiArray<Elem, Dim, Alloc> & t)
    {
        util::serialization::serialize_collection(ar, t);
    }

    namespace detail {
        
        template <typename Archive, 
            typename Elem, typename Bfs, typename Alloc, size_t BF_SHIFT>
        inline void serialize(
            Archive & ar,
            Array<Elem, Bfs, Alloc, BF_SHIFT> & t)
        {
            ar & SERIALIZATION_NVP_3(t, live)
                & SERIALIZATION_NVP_3(t, full)
                & SERIALIZATION_NVP_3(t, nget)
                & SERIALIZATION_NVP_3(t, nset)
                & SERIALIZATION_NVP_NAME("array", framework::container::make_array(t.array_));
        }
        
    }

    template<typename Archive, 
        typename Elem, typename Dim, typename Alloc>
    inline void serialize2(
        Archive & ar,
        MultiArray<Elem, Dim, Alloc> & t)
    {
        ar & t.array();
    }

} }

#endif // _UTIL_SERIALIZATION_MULTI_ARRAY_H_
