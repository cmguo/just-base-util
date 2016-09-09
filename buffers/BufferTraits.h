// BufferTraits.h

#ifndef _UTIL_BUFFERS_BUFFER_TRAITS_H_
#define _UTIL_BUFFERS_BUFFER_TRAITS_H_

#include <boost/asio/buffer.hpp>
#include <boost/type_traits/integral_constant.hpp>
#include <boost/type_traits/detail/yes_no_type.hpp>

namespace boost { namespace asio {
    template <typename T>
    void * buffer(T);
//    template <typename P, typename T>
//    P buffer_cast(T t) { return buffer_cast<P>(buffer(t)); }
//    template <typename T>
//    size_t buffer_size(T t) { return buffer_size(buffer(t)); }
}}

namespace util
{
    namespace buffers
    {

        template <typename Buffer>
        struct is_mutable_buffer_impl
        {
            static boost::type_traits::no_type check(...);

            static boost::type_traits::yes_type check(
                boost::asio::const_buffer);

            BOOST_STATIC_CONSTANT(unsigned, s = 
                sizeof(is_mutable_buffer_impl<Buffer>::check(boost::asio::buffer(*(Buffer const *)0))));

            BOOST_STATIC_CONSTANT(bool, value = (s == sizeof(boost::type_traits::yes_type)));
        };

        template <typename Buffer>
        struct is_mutable_buffer
            : boost::integral_constant<bool, is_mutable_buffer_impl<Buffer>::value>
        {
            using boost::integral_constant<bool, is_mutable_buffer_impl<Buffer>::value>::value;
        };

        template <typename Buffer>
        struct is_const_buffer_impl
        {
            static boost::type_traits::no_type check(...);

            static boost::type_traits::yes_type check(
                boost::asio::const_buffer);

            BOOST_STATIC_CONSTANT(unsigned, s = 
                sizeof(is_const_buffer_impl<Buffer>::check(boost::asio::buffer(*(Buffer const *)0))));

            BOOST_STATIC_CONSTANT(bool, value = (s == sizeof(boost::type_traits::yes_type)));
        };

        template <typename Buffer>
        struct is_const_buffer
            : boost::integral_constant<bool, is_const_buffer_impl<Buffer>::value>
        {
            using boost::integral_constant<bool, is_const_buffer_impl<Buffer>::value>::value;
        };

        template <typename BufferIterator>
        struct is_const_buffer_iterator_impl
        {
            static boost::type_traits::no_type check(...);

            static boost::type_traits::yes_type check(
                boost::asio::const_buffers_1);

            BOOST_STATIC_CONSTANT(unsigned, s = 
                sizeof(is_const_buffer_iterator_impl<BufferIterator>::check(boost::asio::buffer(**(BufferIterator const *)0))));

            BOOST_STATIC_CONSTANT(bool, value = (s == sizeof(boost::type_traits::yes_type)));
        };

        template <typename BufferIterator>
        struct is_const_buffer_iterator
            : boost::integral_constant<bool, is_const_buffer_iterator_impl<BufferIterator>::value>
        {
            using boost::integral_constant<bool, is_const_buffer_iterator_impl<BufferIterator>::value>::value;
        };

    } // namespace buffers
} // namespace util

#endif // _UTIL_BUFFERS_BUFFER_TRAITS_H_
