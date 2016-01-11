// Url.h

#ifndef _UTIL_SERIALIZATION_STRING_URL_H_
#define _UTIL_SERIALIZATION_STRING_URL_H_

#include "util/serialization/SplitFree.h"

#include <framework/string/Url.h>

namespace framework
{
    namespace string
    {

        template <
            typename Archive
        >
        void load(
            Archive & ar, 
            framework::string::Url & url)
        {
            std::string str;
            if (ar >> str)
                if (url.from_string(str))
                    ar.failed();
        }

        template <
            typename Archive
        >
        void save(
            Archive & ar, 
            framework::string::Url const & url)
        {
            std::string str = url.to_string();
            ar << str;
        }

        template <class Archive>
        void serialize(
            Archive & ar, 
            framework::string::Url & url)
        {
            util::serialization::split_free(ar, url);
        }

    } // namespace string
} // namespace framework

#endif // _UTIL_SERIALIZATION_STRING_URL_H_
