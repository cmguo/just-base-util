// NVPair.h

#ifndef _UTIL_SERIALIZATION_NV_PAIR_H_
#define _UTIL_SERIALIZATION_NV_PAIR_H_

#include "util/serialization/Serialization.h"

namespace util
{
    namespace serialization
    {

        template <typename T> // T是非const类型
        struct NVPair
            : wrapper
        {
            char const * name_;
            T & t_;

            NVPair(
                char const * name, 
                T & t) 
                : name_(name)
                ,t_(t)
            {
            }

            T const & const_data() const
            {
                return t_;
            }

            T & data() const
            {
                return t_;
            }

            char const * name() const
            {
                return name_;
            }

            /// 不支持nvp的流，直接序列化值
            template <typename Archive>
            void serialize(Archive & ar)
            {
                ar & t_;
            }
        };

        /// 从定常数组构造array
        template<class T>
        NVPair<T> const make_nvp(
            char const * name, 
            T const & t)
        {
            return NVPair<T>(name, const_cast<T &>(t));
        }

        template<class T>
        char const * wrapper_name(
            NVPair<T> const & t)
        {
            return t.name();
        }

        template<class T>
        T & wrapper_data(
            NVPair<T> const & t)
        {
            return t.data();
        }

    } // namespace serialize
} // namespace util

#define STRINGLIZE(s) #s

#define SERIALIZATION_NVP(v) \
    util::serialization::make_nvp(STRINGLIZE(v), v)

#define SERIALIZATION_NVP_NAME(n, v) \
    util::serialization::make_nvp(n, v)

#define SERIALIZATION_NVP_1(t, v) \
    util::serialization::make_nvp(STRINGLIZE(v), t.v)

#define SERIALIZATION_NVP_2(t, v) \
    util::serialization::make_nvp(STRINGLIZE(v), t.v())

#define SERIALIZATION_NVP_3(t, v) \
    util::serialization::make_nvp(STRINGLIZE(v), t.v ## _)

#endif // _UTIL_SERIALIZATION_NV_PAIR_H_
