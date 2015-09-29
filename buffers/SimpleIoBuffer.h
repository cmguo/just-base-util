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
            typedef ByteIterator<BufferIterator> byte_iterator;
            typedef SimpleIoBuffer<typename _Mode::switch_t, BufferIterator, _Elem, _Traits> switch_t;
            typedef typename std_io_buf_t::char_type char_type;

        public:
            SimpleIoBuffer(
                StdIoStream<_Elem, _Traits> & io_stream, 
                switch_t & switch_buf,
                byte_iterator iter)
                : std_io_buf_t(io_stream)
                , byte_buf_t(iter, byte_buf_t::delay_init())
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
                // make another be in the left side
                r.set_offset(get_offset());
                return *this;
            }

        public:
            void reset(
                byte_iterator iter)
            {
                pos_ = 0;
                byte_buf_t::reset(iter);
                pos_ -= get_offset();
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
                pos_ += get_offset();
                byte_buf_t::advance(switch_buf_, 0);
                pos_ -= get_offset();
            }

            void next_buffer(
                BufferIterator end)
            {
                pos_ += get_offset();
                byte_buf_t::advance(ByteIterator<BufferIterator>(end), 0);
                pos_ -= get_offset();
            }

            size_t position() const
            {
                return pos_ + get_offset();
            }

            void inc_pos(
                size_t d)
            {
                pos_ += d;
            }

        private:
            friend class ByteIteratorT<SimpleIoBuffer, BufferIterator>;
            friend class SimpleIoBuffer<typename _Mode::switch_t, BufferIterator, _Elem, _Traits>;

            void take_full()
            {
                char_type * p = (char_type *)byte_buf_t::buffer_ptr();
                char_type * c = this->get_ptr();
                char_type * e = p + byte_buf_t::buffer_size();
                std_io_buf_t::set_buffer(p, c, e);
            }

            void set_offset(
                size_t o)
            {
                char_type * p = (char_type *)byte_buf_t::buffer_ptr();
                char_type * c = p + o;
                char_type * e = p + byte_buf_t::buffer_size();
                if (byte_buf_t::buffer_iter() == switch_buf_.buffer_iter()) {
                    char_type * c1 = switch_buf_.get_ptr();
                    if (c1 < c) {
                        // if in front of another, then leave my left to it
                        std_io_buf_t::set_buffer(c, c, e);
                        switch_buf_.set_buffer(p, c1, c);
                    } else if (c < c1) {
                        // if in back of another, then take it's left
                        std_io_buf_t::set_buffer(p, c, c1);
                        switch_buf_.set_buffer(c1, c1, e);
                    } else {
                        // if come up with another, then we are in left halt
                        std_io_buf_t::set_buffer(p, c, c);
                        switch_buf_.set_buffer(c, c, e);
                    }
                } else {
                    std_io_buf_t::set_buffer(p, c, e);
                    switch_buf_.take_full();
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
