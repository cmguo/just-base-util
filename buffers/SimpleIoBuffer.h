// SimpleIoBuffer.h

#ifndef _UTIL_BUFFERS_SIMPLE_IO_BUFFER_H_
#define _UTIL_BUFFERS_SIMPLE_IO_BUFFER_H_

#include "util/buffers/StdIoBuffer.h"
#include "util/buffers/ByteIterator.h"

namespace util
{
    namespace buffers
    {

        template <
            class _Mode, 
            typename BufferIterator, 
            typename _Elem = char, 
            typename _Traits = std::char_traits<_Elem>
        >
        class SimpleIoBuffer
            : public StdIoBuffer<_Mode, _Elem, _Traits>
            , public ByteIteratorT<SimpleIoBuffer<_Mode, BufferIterator, _Elem, _Traits>, BufferIterator>
        {
        public:
            typedef StdIoBuffer<_Mode, _Elem, _Traits> std_io_buf_t;
            typedef ByteIteratorT<SimpleIoBuffer, BufferIterator> byte_buf_t;
            typedef SimpleIoBuffer<typename _Mode::switch_t, BufferIterator, _Elem, _Traits> switch_t;
            typedef typename std_io_buf_t::char_type char_type;

        public:
            SimpleIoBuffer(
                StdIoStream<_Elem, _Traits> & io_stream, 
                switch_t & switch_buf,
                BufferIterator iter)
                : std_io_buf_t(io_stream)
                , byte_buf_t(iter)
                , switch_buf_(switch_buf)
                , pos_(0)
            {
            }

            SimpleIoBuffer(
                StdIoStream<_Elem, _Traits> & io_stream, 
                switch_t & switch_buf)
                : std_io_buf_t(io_stream)
                , switch_buf_(switch_buf)
                , pos_(0)
            {
            }

        public:
            SimpleIoBuffer & operator=(
                switch_t & r)
            {
                byte_buf_t::operator=(r);
                pos_ = r.position() - get_offset();
                return *this;
            }

        public:
            void reset(
                BufferIterator iter)
            {
                byte_buf_t::reset(iter);
                pos_ = 0;
            }

            void seek_off(
                size_t n)
            {
                pos_ += get_offset();
                pos_ += byte_buf_t::advance(switch_buf_, n);
                pos_ -= get_offset();
            }

            void seek_off(
                BufferIterator end, 
                size_t n)
            {
                pos_ += get_offset();
                pos_ += byte_buf_t::advance(ByteIterator<BufferIterator>(end), n);
                pos_ -= get_offset();
            }

            void next_buffer()
            {
                byte_buf_t::advance(switch_buf_, 0);
            }

            void next_buffer(
                BufferIterator end)
            {
                byte_buf_t::advance(ByteIterator<BufferIterator>(end), 0);
            }

            size_t position() const
            {
                return pos_ + get_offset();
            }

        private:
            friend class ByteIteratorT<SimpleIoBuffer, BufferIterator>;
            friend class SimpleIoBuffer<typename _Mode::switch_t, BufferIterator, _Elem, _Traits>;

            void set_offset(
                size_t o)
            {
                char_type * p = (char_type *)byte_buf_t::buffer_ptr();
                size_t s = byte_buf_t::buffer_size();
                if (byte_buf_t::buffer_iter() == switch_buf_.buffer_iter()) {
                    size_t o1 = switch_buf_.get_offset();
                    if (o1 < o) {
                        switch_buf_.set_buffer(p, p + o1, p + o);
                        std_io_buf_t::set_buffer(p + o, p + o, p + s);
                    } else if (o < o1) {
                        std_io_buf_t::set_buffer(p, p + o, p + o1);
                        switch_buf_.set_buffer(p + o1, p + o1, p + s);
                    } else {
                        switch_buf_.set_buffer(p, p + o, p + o);
                        std_io_buf_t::set_buffer(p + o, p + o, p + s);
                    }
                } else {
                    std_io_buf_t::set_buffer(p, p + o, p + s);
                }
            }

            size_t get_offset() const
            {
                char_type * b = (char_type *)byte_buf_t::buffer_ptr();
                char_type * c = std_io_buf_t::get_ptr();
                return c - b;
            }

        private:
            switch_t & switch_buf_;
            size_t pos_;
        };

    }
}

#endif // _UTIL_BUFFERS_SIMPLE_IO_BUFFER_H_
