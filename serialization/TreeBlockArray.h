// TreeBlockArray.h

#ifndef _UTIL_SERIALIZATION_TREE_BLOCK_ARRAY_H_
#define _UTIL_SERIALIZATION_TREE_BLOCK_ARRAY_H_

#include "util/serialization/Collection.h"
#include "util/serialization/SplitMember.h"

#include <framework/container/MultiArray.h>

namespace util
{
    namespace serialization
    {

        template<typename Archive, 
            typename Value, size_t Block, typename Alloc
        >
        struct is_collection<Archive, framework::container::TreeBlockArray<Value, Block, Alloc> >
            : boost::true_type
        {
        };

    }  // namespace serialization
} // namespace util

namespace framework { namespace container {

    template<typename Archive, 
        typename Value, size_t Block, typename Alloc>
    inline void serialize(
        Archive & ar,
        TreeBlockArray<Value, Block, Alloc> & t)
    {
        util::serialization::serialize_collection(ar, t);
    }

    namespace detail {
        
        template <typename Archive, 
            typename Value, size_t Block>
        inline void serialize(
            Archive & ar,
            TreeBlockArrayItem<Value, Block> & t)
        {
            ar & SERIALIZATION_NVP_1(t, index)
                & SERIALIZATION_NVP_NAME("values", framework::container::make_array(t.values));
        }
        
    }

    template<typename Archive, 
        typename Value, size_t Block, typename Alloc>
    inline void serialize2(
        Archive & ar,
        TreeBlockArray<Value, Block, Alloc> & t)
    {
        ar & t.items();
    }
        

} }

#endif // _UTIL_SERIALIZATION_TREE_BLOCK_ARRAY_H_
