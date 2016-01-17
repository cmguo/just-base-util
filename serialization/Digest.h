// Digest<N>.h

#ifndef _UTIL_SERIALIZATION_STRING_DIGEST_H_
#define _UTIL_SERIALIZATION_STRING_DIGEST_H_

#include "util/serialization/NVPair.h"
#include "util/serialization/SplitFree.h"
#include "util/serialization/Array.h"

#include <framework/string/Digest.h>

namespace framework
{
    namespace string
    {

        template <typename Archive, size_t N>
        void serialize(Archive & ar, Digest<N> & t)
        {
            util::serialization::split_free(ar, t);
        }

        template <typename Archive, size_t N>
        void load(Archive & ar, Digest<N> & t)
        {
            typename framework::string::Digest<N>::bytes_type digest;
            ar >> framework::container::make_array(digest.elems);
            if (ar)
                t.from_bytes(digest);
        };

        template <typename Archive, size_t N>
        void save(Archive & ar, Digest<N> const & t)
        {
            typename framework::string::Digest<N>::bytes_type digest = t.to_bytes();
            ar << framework::container::make_array(digest.elems);
        }

    } // namespace string
} // namespace framework

#endif // _UTIL_SERIALIZATION_STRING_DIGEST_H_
