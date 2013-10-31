// map.h

#ifndef _UTIL_SERIALIZATION_STL_MAP_H_
#define _UTIL_SERIALIZATION_STL_MAP_H_

#include "util/serialization/Collection.h"
#include "util/serialization/SplitMember.h"
#include "util/serialization/stl/utility.h"

#include <map>

namespace util
{
    namespace serialization
    {

        template<
            class Archive, 
            class _Kty, 
            class _Ty, 
            class _Pr, 
            class _Alloc
        >
        inline void serialize(
            Archive & ar,
            std::map<_Kty, _Ty, _Pr, _Alloc> & t)
        {
            serialize_collection(ar, t);
        }

        template<
            class _Size, 
            class _Kty, 
            class _Ty, 
            class _Pr, 
            class _Alloc
        >
        class SizedMap
        {
        public:
            SizedMap(
                std::map<_Kty, _Ty, _Pr, _Alloc> & map)
                : map_(map)
            {
            }

            SERIALIZATION_SPLIT_MEMBER();

            template <typename Archive>
            void save(Archive & ar) const
            {
                _Size count((_Size)map_.size());
                ar & SERIALIZATION_NVP(count);
                save_collection(ar, map_, count);
            }

            template <typename Archive>
            void load(Archive & ar)
            {
                _Size count(0);
                ar & SERIALIZATION_NVP(count);
                load_collection(ar, map_, count);
            }

        private:
            std::map<_Kty, _Ty, _Pr, _Alloc> & map_;
        };

        template<
            class _Size, 
            class _Kty, 
            class _Ty, 
            class _Pr, 
            class _Alloc
        >
        SizedMap<_Size, _Kty, _Ty, _Pr, _Alloc> const make_sized(
            std::map<_Kty, _Ty, _Pr, _Alloc> & map)
        {
            return SizedMap<_Size, _Kty, _Ty, _Pr, _Alloc>(map);
        }

    } // namespace serialization
} // namespace util

#endif // _UTIL_SERIALIZATION_STL_MAP_H_
