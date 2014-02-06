// NullBuffer.h

#ifndef _UTIL_BUFFERS_NULL_BUFFER_H_
#define _UTIL_BUFFERS_NULL_BUFFER_H_

#include <streambuf>
#include <stdexcept>
#include <limits>

#include <boost/asio/buffer.hpp>

namespace util
{
    namespace buffers
    {

        template <
            typename _Elem = char, 
            typename _Traits = std::char_traits<_Elem>
        >
        class NullBuffer
            : public std::basic_streambuf<_Elem, _Traits>
        {
        public:
            typedef typename std::basic_streambuf<_Elem, _Traits>::char_type char_type;
            typedef typename std::basic_streambuf<_Elem, _Traits>::int_type int_type;
            typedef typename std::basic_streambuf<_Elem, _Traits>::traits_type traits_type;
            typedef typename std::basic_streambuf<_Elem, _Traits>::pos_type pos_type;
            typedef typename std::basic_streambuf<_Elem, _Traits>::off_type off_type;

        public:
            NullBuffer()
                : rpos_(0)
                , wpos_(0)
            {
                this->setg(NULL, NULL, NULL);
                this->setp(NULL, NULL);
            }

        public:
            virtual int_type underflow()
            {
                return traits_type::to_int_type(0);
            }

            virtual int_type overflow(int_type c)
            {
                wpos_ += 1;
                return c;
            }

            virtual std::streamsize xsgetn(
                _Elem * _Ptr,
                std::streamsize _Count)
            {
                rpos_ += _Count;
                memset(_Ptr, 0, _Count);
                return _Count;
            }

            virtual std::streamsize xsputn(
                _Elem const * _Ptr,
                std::streamsize _Count)
            {
                wpos_ += _Count;
                return _Count;
            }

            virtual pos_type seekoff(
                off_type off, 
                std::ios_base::seekdir dir,
                std::ios_base::openmode mode)
            {
                if (dir == std::ios_base::beg) {
                    return seekpos(off, mode);
                } else if (dir == std::ios_base::cur) {
                    pos_type pos = (mode == std::ios_base::in) ? rpos_ : wpos_;
                    if (off == 0) {
                        return pos;
                    }
                    pos += off;
                    return seekpos(pos, mode);
                } else if (dir == std::ios_base::end) {
                    return pos_type(-1);
                }
                return pos_type(-1);
            }

            virtual pos_type seekpos(
                pos_type position, 
                std::ios_base::openmode mode)
            {
                if (mode == std::ios_base::in) {
                    rpos_ = position;
                } else {
                    wpos_ = position;
                }
                return position;
            }

        private:
            pos_type rpos_;
            pos_type wpos_;
        };

    } // namespace buffers
} // namespace util

#endif // _UTIL_BUFFERS_NULL_BUFFER_H_
