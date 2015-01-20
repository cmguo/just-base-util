// BuffersCompare.h

#ifndef _UTIL_BUFFERS_BUFFERS_COMPARE_H_
#define _UTIL_BUFFERS_BUFFERS_COMPARE_H_

#include <boost/asio/buffer.hpp>

namespace util
{
    namespace buffers
    {

        template <typename ConstBufferIterator1, typename ConstBufferIterator2>
        size_t buffers_compare(
            ConstBufferIterator1 iter1, 
            ConstBufferIterator1 const & end1, 
            ConstBufferIterator2 iter2, 
            ConstBufferIterator2 const & end2, 
            size_t size = size_t(-1), 
            size_t off1 = 0, 
            size_t off2 = 0)
        {
            using namespace boost::asio;

            const_buffer buf1;
            if (iter1 == end1) {
                off1 = 1;
            } else {
                buf1 = buffer(*iter1);
                while (off1) {
                    if (off1 >= buffer_size(buf1)) {
                        if (++iter1 == end1)
                            break;
                        off1 -= buffer_size(buf1);
                        buf1 = buffer(*iter1);
                    } else {
                        buf1 = buf1 + off1;
                        off1 = 0;
                    }
                }
            }
            const_buffer buf2;
            if (iter2 == end2) {
                off2 = 1;
            } else {
                buf2 = buffer(*iter2);
                while (off2) {
                    if (off2 >= buffer_size(buf2)) {
                        if (++iter2 == end2)
                            break;
                        off2 -= buffer_size(buf2);
                        buf2 = buffer(*iter2);
                    } else {
                        buf2 = buf2 + off2;
                        off2 = 0;
                    }
                }
            }
            size_t result = 0;
            if (off1 || off2) {
                if (off1 & off2) {
                } else if (off1) {
                    result = -(int)*buffer_cast<char const *>(buf2);
                } else {
                    result = (int)*buffer_cast<char const *>(buf1);
                }
                result += 256;
                return result;
            }
            while (1) {
                size_t this_size = size;
                if (this_size > buffer_size(buf1)) {
                    this_size = buffer_size(buf1);
                }
                if (this_size > buffer_size(buf2)) {
                    this_size = buffer_size(buf2);
                }
                int result = memcmp(buffer_cast<void const *>(buf1), buffer_cast<void const *>(buf2), this_size);
                size -= this_size;
                if (result || size == 0)
                    break;
                if (this_size == buffer_size(buf1)) {
                    buf2 = buf2 + this_size;
                    if (++iter1 == end1)
                        break;
                    buf1 = buffer(*iter1);
                    this_size = 0;
                }
                if (this_size == buffer_size(buf2)) {
                    buf1 = buf1 + this_size;
                    if (++iter2 == end2)
                        break;
                    buf2 = buffer(*iter2);
                }
            }
            if (result == 0 && size != 0) {
                if (iter1 == end1 && iter2 == end2) {
                } else if (iter1 == end1) {
                    result = -(int)*buffer_cast<char const *>(buf2);
                } else {
                    result = (int)*buffer_cast<char const *>(buf1);
                }
                result += 256;
            }
            return result;
        }

        template <typename ConstBufferSequence1, typename ConstBufferSequence2>
        size_t buffers_compare(
            ConstBufferSequence1 const & buffers1, 
            ConstBufferSequence2 const & buffers2, 
            size_t size = size_t(-1), 
            size_t off1 = 0, 
            size_t off2 = 0)
        {
            return buffers_compare(buffers1.begin(), buffers1.end(), buffers2.begin(), buffers2.end(), size, off1, off2);
        }

    } // namespace buffers
} // namespace util

#endif // _UTIL_BUFFERS_BUFFERS_COMPARE_H_
