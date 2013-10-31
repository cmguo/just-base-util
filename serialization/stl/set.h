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

        template<class Archive, class _Kty, class _Pr, class _Alloc>
        inline void serialize(
            Archive & ar,
            std::set<_Kty, _Pr, _Alloc> & t)
        {
            serialize_collection(ar, t);
        }

        template<
            class _Size, 
            class _Kty, 
            class _Alloc
        >
        class SizedSet
        {
        public:
            SizedSet(
                std::set<_Kty, _Alloc> & set)
                : set_(set)
            {
            }

            SERIALIZATION_SPLIT_MEMBER();

            template <typename Archive>
            void save(Archive & ar) const
            {
                _Size size = set_.size();
                ar & size;
                save_collection(ar, set_, size);
            }

            template <typename Archive>
            void load(Archive & ar)
            {
                _Size size = 0;
                ar & len;
                load_collection(ar, set_, size);
            }

        private:
            std::set<_Kty, _Alloc> & set_;
        };

        template<
            class _Size, 
            class _Kty, 
            class _Alloc
        >
        SizedSet<_Size, _Kty, _Alloc> const make_sized(
            std::set<_Kty, _Alloc> & set)
        {
            return SizedSet<_Size, _Kty, _Alloc>(set);
        }

    } // namespace serialization
} // namespace util

#endif // _UTIL_SERIALIZATION_STL_SET_H_
