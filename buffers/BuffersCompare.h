// BuffersCompare.h

#ifndef _UTIL_BUFFERS_BUFFERS_COMPARE_H_
#define _UTIL_BUFFERS_BUFFERS_COMPARE_H_

#include "util/buffers/BuffersPosition.h"

namespace util
{
    namespace buffers
    {

        namespace detail
        {

            template <
                typename Buffer1, 
                typename BufferIterator1, 
                typename Buffer2, 
                typename BufferIterator2
            >
            int buffers_compare_impl(
                BuffersPosition<Buffer1, BufferIterator1> pos1, 
                BuffersPosition<Buffer1, BufferIterator1> const & end1, 
                BuffersPosition<Buffer2, BufferIterator2> pos2, 
                BuffersPosition<Buffer2, BufferIterator2> const & end2, 
                size_t size = (size_t)-1)
            {
                char const * buf1 = "";
                size_t size1 = 0;
                char const * buf2 = "";
                size_t size2 = 0;
                int result = 0;
                while (size > 0 && !pos1.at_end() && !pos2.at_end()) {
                    buf1 = boost::asio::buffer_cast<char const *>(pos1.dereference_buffer());
                    size1 = boost::asio::buffer_size(pos1.dereference_buffer());
                    buf2 = boost::asio::buffer_cast<char const *>(pos2.dereference_buffer());
                    size2 = boost::asio::buffer_size(pos2.dereference_buffer());
                    if (size1 <= size2) {
                        if (size1 > size)
                            size1 = size;
                        result = ::memcmp(buf1, buf2, size1);
                        size -= size1;
                        if (result || size == 0)
                            break;
                        pos2.increment_bytes(end2, size1);
                        pos1.increment_buffer(end1);
                    } else {
                        if (size2 > size)
                            size2 = size;
                        result = ::memcmp(buf1, buf2, size2);
                        size -= size2;
                        if (result || size == 0)
                            break;
                        pos1.increment_bytes(end1, size2);
                        pos2.increment_buffer(end2);
                    }
                }
                if (result == 0 && size != 0) {
                    if (pos1.at_end() && pos2.at_end()) {
                    } else if (pos1.at_end()) {
                        result = -(int)pos2.dereference_byte();
                    } else {
                        result = (int)pos1.dereference_byte();
                    }
                    result += 256;
                }
                return result;
            }

            template <
                typename Buffer1, 
                typename BufferIterator1, 
                typename Buffer2, 
                typename BufferIterator2
            >
            int buffers_compare_impl(
                BufferIterator1 const & pos1, 
                BufferIterator1 const & end1, 
                BufferIterator2 const & pos2, 
                BufferIterator2 const & end2, 
                size_t size = (size_t)-1)
            {
                BuffersPosition<Buffer1, BufferIterator1> bpos1(pos1, end1);
                BuffersPosition<Buffer1, BufferIterator1> bend1(end1);
                BuffersPosition<Buffer2, BufferIterator2> bpos2(pos2, end2);
                BuffersPosition<Buffer2, BufferIterator2> bend2(end2);
                return buffers_compare_impl(bpos1, bend1, bpos2, bend2);
            }

        } // namespace detail

        template <typename ConstBufferSequence1, typename ConstBufferSequence2>
        size_t buffers_compare(
            ConstBufferSequence1 const & buffers1, 
            ConstBufferSequence2 const & buffers2, 
            size_t size = size_t(-1))
        {
            return detail::buffers_compare_impl<
                typename ConstBufferSequence1::value_type, 
                typename ConstBufferSequence1::const_iterator, 
                typename ConstBufferSequence2::value_type, 
                typename ConstBufferSequence2::const_iterator
            >(buffers1.begin(), buffers1.end(), 
                buffers2.begin(), buffers2.end(), size);
        }

    } // namespace buffers
} // namespace util

#endif // _UTIL_BUFFERS_BUFFERS_COMPARE_H_
