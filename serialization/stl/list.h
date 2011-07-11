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

        template<class Archive, class _Ty, class _Alloc>
        inline void serialize(
            Archive & ar,
            std::list<_Ty, _Alloc> & t)
        {
            serialize_collection(ar, t);
        }

        template<
            class _Size, 
            class _Ty, 
            class _Alloc
        >
        class SizedList
        {
        public:
            SizedList(
                std::list<_Ty, _Alloc> & list)
                : list_(list)
            {
            }

            SERIALIZATION_SPLIT_MEMBER();

            template <typename Archive>
            void save(Archive & ar) const
            {
                _Size size = list_.size();
                ar & size;
                save_collection(ar, list_, size);
            }

            template <typename Archive>
            void load(Archive & ar)
            {
                _Size size = 0;
                ar & len;
                list_.resize(size);
                load_collection(ar, list_, size);
            }

        private:
            std::list<_Ty, _Alloc> & list_;
        };

        template<
            class _Size, 
            class _Ty, 
            class _Alloc
        >
        SizedList<_Size, _Ty, _Alloc> const make_sized(
            std::list<_Ty, _Alloc> & list)
        {
            return SizedList<_Size, _Ty, _Alloc>(list);
        }

    } // namespace serialization
} // namespace util

#endif // _UTIL_SERIALIZATION_STL_LIST_H_
