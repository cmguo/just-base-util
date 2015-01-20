// BuffersSize.h

#ifndef _UTIL_BUFFERS_BUFFERS_SIZE_H_
#define _UTIL_BUFFERS_BUFFERS_SIZE_H_

#include <boost/asio/buffer.hpp>

namespace util
{
    namespace buffers
    {

        template <typename ConstBufferIterator>
        size_t buffers_size(
            ConstBufferIterator const & beg, 
            ConstBufferIterator const & end)
        {
            ConstBufferIterator iter = beg;
            size_t total_size = 0;
            for (; iter != end; ++iter) {
                total_size += boost::asio::buffer_size(buffer(*iter));
            }
            return total_size;
        }

        template <typename ConstBufferSequence>
        size_t buffers_size(
            ConstBufferSequence const & buf)
        {
            return buffers_size(buf.begin(), buf.end());
        }

    } // namespace buffers
} // namespace util

#endif // _UTIL_BUFFERS_BUFFERS_SIZE_H_
