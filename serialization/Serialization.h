// Serialization.h

#ifndef _UTIL_SERIALIZATION_SERIALIZATION_H_
#define _UTIL_SERIALIZATION_SERIALIZATION_H_

#include <boost/mpl/if.hpp>
#include <boost/type_traits/is_fundamental.hpp>
#include <boost/type_traits/is_base_and_derived.hpp>

namespace util
{
    namespace serialization
    {

        struct wrapper {};

        template <typename T>
        struct is_wrapper
            : boost::is_base_and_derived<wrapper, T>
        {};

        /// ָʾһ�������Ƿ������л���������
        /// �������͵����л��������л���ֱ�ӵ������л��������л������save��load������
        template <
            typename Ar, 
            typename T
        >
        struct is_primitive
            : boost::is_fundamental<T>
        {
        };

        /// ָʾһ�������Ƿ������л���������
        template <
            typename Ar, 
            typename T
        >
        struct is_single_unit
            : boost::false_type
        {
        };

        template <
            typename Ar, 
            typename T
        >
        struct is_stringlized
            : boost::false_type
        {
        };

        template <
            typename Ar, 
            typename T
        >
        struct is_collection
            : boost::false_type
        {
        };

        template<class Archive, class T>
        inline void serialize(Archive & ar, T & t){
            t.serialize(ar);
        }

    }  // namespace serialization
} // namespace util

#endif // _UTIL_SERIALIZATION_SERIALIZATION_H_
