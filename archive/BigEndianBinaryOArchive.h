// BigEndianBinaryOArchive.h

#ifndef _UTIL_ARCHIVE_BIG_ENDIAN_BINARY_O_ARCHIVE_H_
#define _UTIL_ARCHIVE_BIG_ENDIAN_BINARY_O_ARCHIVE_H_

#include "util/archive/StreamOArchive.h"
#include "util/serialization/Array.h"

#include <framework/system/BytesOrder.h>

#include <boost/type_traits/is_same.hpp>
#include <boost/utility/enable_if.hpp>

#include <ostream>

#ifdef BOOST_BIG_ENDIAN

#  include "util/archive/BinaryOArchive.h"

namespace util
{
    namespace archive
    {

        template <
            typename _Elem = char, 
            typename _Traits = std::char_traits<_Elem>
        >
        class BigEndianBinaryOArchive
            : public BinaryOArchive<_Elem, _Traits>
        {
        public:
            BigEndianBinaryOArchive(
                std::basic_ostream<_Elem, _Traits> & os)
                : BinaryOArchive<_Elem, _Traits>(*os.rdbuf())
            {
            }

            BigEndianBinaryOArchive(
                std::basic_streambuf<_Elem, _Traits> & buf)
                : BinaryOArchive<_Elem, _Traits>(buf)
            {
            }
        };

    } // namespace archive
} // namespace util

#else // BOOST_BIG_ENDIAN

namespace util
{
    namespace archive
    {

        /**
        转换字节顺序的模板，主机顺序->网络顺序

        为了让编译器自动绑定相应的转换函数
        注意：没有实现8个字节数据的字节顺序转换
        */ 
        template <typename T, int size = sizeof(T)>
        struct htob
        {
            static void apply(
                T &)
            {
            }
        };

        /// 2个字节数据的字节顺序转换
        template <typename T>
        struct htob<T, 2>
        {
            static void apply(
                T & t)
            {
                t = (T)framework::system::BytesOrder::host_to_big_endian_short(t);
            }
        };

        /// 4个字节数据的字节顺序转换
        template <typename T>
        struct htob<T, 4>
        {
            static void apply(
                T & t)
            {
                t = (T)framework::system::BytesOrder::host_to_big_endian_long(t);
            }
        };

        template <typename T>
        struct htob<T, 8>
        {
            static void apply(
                T & t)
            {
                t = (T)framework::system::BytesOrder::host_to_big_endian_longlong(t);
            }
        };

        /// 网络字节顺序序列化类
        template <
            typename _Elem = char, 
            typename _Traits = std::char_traits<_Elem>
        >
        class BigEndianBinaryOArchive
            : public StreamOArchive<BigEndianBinaryOArchive<_Elem, _Traits>, _Elem, _Traits>
        {
            friend class StreamOArchive<BigEndianBinaryOArchive<_Elem, _Traits>, _Elem, _Traits>;

        public:
            /// 从ostream构造
            BigEndianBinaryOArchive(
                std::basic_ostream<_Elem, _Traits> & os)
                : StreamOArchive<BigEndianBinaryOArchive<_Elem, _Traits>, _Elem, _Traits>(*os.rdbuf())
            {
            }

            BigEndianBinaryOArchive(
                std::basic_streambuf<_Elem, _Traits> & buf)
                : StreamOArchive<BigEndianBinaryOArchive<_Elem, _Traits>, _Elem, _Traits>(buf)
            {
            }

            /// 序列化通用类型实现
            template <typename T>
            void save(
                T const & t)
            {
                // 先转换字节顺序
                T t1 = t;
                htob<T>::apply(t1);
                this->save_binary((char const *)&t1, sizeof(T));
            }

            using StreamOArchive<BigEndianBinaryOArchive>::save;

            /// 判断某个类型是否可以优化数组的序列化
            /// 只有char类型能够直接序列化数组，不需要转换字节顺序
            template<class T>
            struct use_array_optimization
                : boost::integral_constant<bool, sizeof(T) == 1>
            {
            };

            // 序列化数组，直接二进制批量写入，是针对char数组的优化实现
            template<class T>
            void save_array(
                framework::container::Array<char> const & a, 
                typename boost::enable_if<use_array_optimization<T> >::type * = NULL)
            {
                this->save_binary((char *)a.address(), a.count());
            }
        };

    } // namespace archive
} // namespace util

#endif // BOOST_BIG_ENDIAN

namespace util
{
    namespace serialization
    {
        template <
            typename _Elem, 
            typename _Traits, 
            typename T
        >
        struct use_array_optimization<util::archive::BigEndianBinaryOArchive<_Elem, _Traits>, T>
            : util::archive::BigEndianBinaryOArchive<_Elem, _Traits>::template use_array_optimization<T>
        {
        };
    }
}

#endif // _UTIL_ARCHIVE_BIG_ENDIAN_BINARY_O_ARCHIVE_H_
