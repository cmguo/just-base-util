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
        inline void serialize(
            Archive & ar,
            std::deque<_Ty, _Alloc> & t)
        {
            serialize_collection(ar, t);
        }

        template<
            class _Size, 
            class _Ty, 
            class _Alloc
        >
        class SizedDeque
        {
        public:
            SizedDeque(
                std::deque<_Ty, _Alloc> & deque)
                : deque_(deque)
            {
            }

            SERIALIZATION_SPLIT_MEMBER();

            template <typename Archive>
            void save(Archive & ar) const
            {
                _Size count((_Size)deque_.size());
                ar & SERIALIZATION_NVP(count);
                save_collection(ar, deque_, count);
            }

            template <typename Archive>
            void load(Archive & ar)
            {
                _Size count(0);
                ar & SERIALIZATION_NVP(count);
                deque_.resize(count);
                load_collection(ar, deque_, count);
            }

        private:
            std::deque<_Ty, _Alloc> & deque_;
        };

        template<
            class _Size, 
            class _Ty, 
            class _Alloc
        >
        SizedDeque<_Size, _Ty, _Alloc> const make_sized(
            std::deque<_Ty, _Alloc> & deque)
        {
            return SizedDeque<_Size, _Ty, _Alloc>(deque);
        }

    } // namespace serialization
} // namespace util

#endif // _UTIL_SERIALIZATION_STL_DEQUE_H_
