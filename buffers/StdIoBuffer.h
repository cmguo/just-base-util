// StdIoBuffer.h

#ifndef _UTIL_BUFFERS_STD_IO_BUFFER_H_
#define _UTIL_BUFFERS_STD_IO_BUFFER_H_

#include <boost/asio/buffer.hpp>

#include <streambuf>

namespace util
{
    namespace buffers
    {

        template <
            class _Mode
        >
        struct StdIoMode;

        namespace detail
        {
            struct _write{};
            struct _read{};
        }

        template <
        >
        struct StdIoMode<detail::_read>
            : detail::_read
        {
            typedef detail::_read mode_t;
            typedef void const * void_t;
            typedef boost::asio::const_buffer buffer_t;
            typedef boost::asio::const_buffers_1 buffers_t;
            typedef detail::_write switch_mode_t;
            typedef StdIoMode<detail::_write> switch_t;
        };

        template <
        >
        struct StdIoMode<detail::_write>
            : detail::_write
        {
            typedef detail::_write mode_t;
            typedef void * void_t;
            typedef boost::asio::mutable_buffer buffer_t;
            typedef boost::asio::mutable_buffers_1 type;
            typedef detail::_read switch_mode_t;
            typedef StdIoMode<detail::_read> switch_t;
        };

        template <
            class _Mode, 
            typename _Elem = char, 
            typename _Traits = std::char_traits<_Elem>
        >
        class StdIoBuffer;

        template <
            typename _Elem = char, 
            typename _Traits = std::char_traits<_Elem>
        >
        class StdIoStream
            : public std::basic_streambuf<_Elem, _Traits>
        {
        public:
            typedef typename std::basic_streambuf<_Elem, _Traits>::char_type char_type;

        private:
            friend class StdIoBuffer<StdIoMode<detail::_read>, _Elem, _Traits>;
            friend class StdIoBuffer<StdIoMode<detail::_write>, _Elem, _Traits>;

            void set_buffer(
                char_type * beg, 
                char_type * cur, 
                char_type * end, 
                detail::_write const &)
            {
                this->setp(beg, end);
                this->pbump(static_cast<int>(cur - beg));
            }

            void set_buffer(
                char_type * beg, 
                char_type * cur, 
                char_type * end, 
                detail::_read const &)
            {
                this->setg(beg, cur, end);
            }

            void get_buffer(
                char_type *& beg, 
                char_type *& cur, 
                char_type *& end, 
                detail::_write const &)
            {
                beg = this->pbase();
                cur = this->pptr();
                end = this->epptr();
            }

            void get_buffer(
                char_type *& beg, 
                char_type *& cur, 
                char_type *& end, 
                detail::_read const &)
            {
                beg = this->eback();
                cur = this->gptr();
                end = this->egptr();
            }

            char_type * get_ptr(
                detail::_write const &)
            {
                return this->pptr();
            }

            char_type * get_ptr(
                detail::_read const &)
            {
                return this->gptr();
            }
        };

        template <
            class _Mode, 
            typename _Elem, 
            typename _Traits
        >
        class StdIoBuffer
            : _Mode
        {
        public:
            typedef typename StdIoStream<_Elem, _Traits>::char_type char_type;

        public:
            StdIoBuffer(
                StdIoStream<_Elem, _Traits> & io_stream)
                : io_stream_(io_stream)
            {
                char_type * p = NULL;
                io_stream_.set_buffer(p, p, p, *this);
            }

            char_type * get_ptr() const
            {
                return io_stream_.get_ptr(*this);
            }

        protected:
            void set_buffer(
                char_type * beg,
                char_type * cur,
                char_type * end)
            {
                io_stream_.set_buffer(beg, cur, end, *this);
            }

        private:
            StdIoStream<_Elem, _Traits> & io_stream_;
        };

    }
}

#endif // _UTIL_BUFFERS_STD_IO_BUFFER_H_
