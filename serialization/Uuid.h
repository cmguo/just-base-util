// Uuid.h

#ifndef _UTIL_SERIALIZATION_STRING_GUID_H_
#define _UTIL_SERIALIZATION_STRING_GUID_H_

#include "util/serialization/NVPair.h"
#include "util/serialization/SplitFree.h"
#include "util/serialization/Array.h"

#include <framework/string/Uuid.h>

namespace framework
{
    namespace string
    {

        template <typename Archive>
        void serialize(Archive & ar, Uuid & t)
        {
            util::serialization::split_free(ar, t);
        }

        template <typename Archive>
        void load(Archive & ar, Uuid & t)
        {
            framework::string::Uuid::bytes_type uuid;
            ar >> framework::container::make_array(uuid.elems);
            if (ar)
                t.from_bytes(uuid);
        };

        template <typename Archive>
        void save(Archive & ar, Uuid const & t)
        {
            framework::string::Uuid::bytes_type uuid = t.to_bytes();
            ar << framework::container::make_array(uuid.elems);
        }

    } // namespace string
} // namespace framework

#endif // _UTIL_SERIALIZATION_STRING_GUID_H_
