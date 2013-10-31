// BuffersSize.h

#ifndef _UTIL_BUFFERS_BUFFERS_FILL_H_
#define _UTIL_BUFFERS_BUFFERS_FILL_H_

#include <boost/asio/buffer.hpp>

namespace util
{
    namespace buffers
    {

        template <typename ConstBufferSequence>
        size_t buffers_fill(
            ConstBufferSequence const & buf, 
            boost::uint8_t chr, 
            size_t len = (size_t)-1)
        {
            using namespace boost::asio;
            typename ConstBufferSequence::const_iterator iter = buf.begin();
            typename ConstBufferSequence::const_iterator end = buf.end();
            size_t left = len;
            for (; iter != end; ++iter) {
                size_t size = boost::asio::buffer_size(buffer(*iter));
                if (size < left) {
                    memset(boost::asio::buffer_cast<void *>(buffer(*iter)), chr, size);
                    left -= size;
                } else {
                    memset(boost::asio::buffer_cast<void *>(buffer(*iter)), chr, left);
                    left = 0;
                    break;
                }
            }
            return len - left;
        }

        template <typename ConstBufferSequence>
        size_t buffers_zero(
            ConstBufferSequence const & buf, 
            size_t len = (size_t)-1)
        {
            return buffers_fill(buf, 0, len);
        }

    } // namespace buffers
} // namespace util

#endif // _UTIL_BUFFERS_BUFFERS_FILL_H_
