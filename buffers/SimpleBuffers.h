// SimpleBuffers.h

#ifndef _UTIL_BUFFERS_CYCLE_BUFFERS_H_
#define _UTIL_BUFFERS_CYCLE_BUFFERS_H_

#include "util/buffers/SimpleIoBuffer.h"
#include "util/buffers/ByteBuffers.h"

namespace util
{
    namespace buffers
    {

        template <
            typename BufferIterator, 
            typename Elem = char, 
            typename Traits = std::char_traits<Elem>
        >
        class SimpleBuffers
            : public StdIoStream<Elem, Traits>
        {
        public:
            typedef typename StdIoStream<Elem, Traits>::char_type char_type;

            typedef typename StdIoStream<Elem, Traits>::int_type int_type;

            typedef typename StdIoStream<Elem, Traits>::traits_type traits_type;

            typedef typename StdIoStream<Elem, Traits>::pos_type pos_type;
 
            typedef typename StdIoStream<Elem, Traits>::off_type off_type;
 

            typedef StdIoStream<Elem, Traits> stream_type;

            typedef BufferIterator buffer_iterator;

            typedef ByteIterator<buffer_iterator> byte_iterator;

            typedef SimpleIoBuffer<
                StdIoMode<detail::_read>, buffer_iterator, char_type, traits_type> read_buffer;

            typedef SimpleIoBuffer<
                StdIoMode<detail::_write>, buffer_iterator, char_type, traits_type> write_buffer;

            typedef ByteBuffers<buffer_iterator> byte_buffers;

            typedef ByteBuffers<buffer_iterator> const_byte_buffers;

            typedef ByteBuffers<buffer_iterator> mutable_byte_buffers;

            //typedef ConstBuffers const_buffers_type;
            //typedef MutableBuffers mutable_buffers_type;

        public:
            SimpleBuffers()
                : write_(*this, read_)
                , read_(*this, write_)
            {
            }

            template <typename Buffers>
            SimpleBuffers(
                Buffers const & buffers, 
                size_t avail = 0)
                : beg_(buffers.begin())
                , end_(buffers.end())
                , write_(*this, read_, beg_)
                , read_(*this, write_, end_)
            {
                reset(avail);
            }

            SimpleBuffers(
                byte_buffers const & buffers)
                : beg_(buffers.begin().byte_iter())
                , end_(buffers.end().byte_iter())
                , write_(*this, read_, beg_)
                , read_(*this, write_, end_)
            {
                reset();
            }

            SimpleBuffers(
                buffer_iterator beg, 
                size_t avail)
                : beg_(beg)
                , end_(beg_ + avail)
                , write_(*this, read_, beg_)
                , read_(*this, write_, beg_)
            {
                reset(avail);
            }

            SimpleBuffers(
                buffer_iterator beg, 
                buffer_iterator end)
                : beg_(beg)
                , end_(end)
                , write_(*this, read_, beg_)
                , read_(*this, write_, end_)
            {
                reset();
            }

            SimpleBuffers(
                byte_iterator beg, 
                size_t avail)
                : beg_(beg)
                , end_(beg_ + avail)
                , write_(*this, read_, beg_)
                , read_(*this, write_, end_)
            {
                reset();
            }

            SimpleBuffers(
                byte_iterator beg, 
                byte_iterator end)
                : beg_(beg)
                , end_(end)
                , write_(*this, read_, beg_)
                , read_(*this, write_, end_)
            {
                reset();
            }

            // Copy constructor.
            SimpleBuffers(
                SimpleBuffers const & other)
                : beg_(other.beg_)
                , end_(other.end_)
                , write_(*this, read_, beg_)
                , read_(*this, write_, end_)
            {
            }

        public:
            mutable_byte_buffers prepare() const
            {
                return mutable_byte_buffers(write_, end_);
            }

            mutable_byte_buffers prepare(
                size_t size) const
            {
                //if (size > this->out_avail()) 
                //    throw std::length_error("util::buffer::SimpleBuffers too long");
                return mutable_byte_buffers(write_, byte_iterator(write_) + size);
            }

            const_byte_buffers data() const
            {
                return const_byte_buffers(read_, write_);
            }

            const_byte_buffers data(
                size_t size) const
            {
                if (size > this->in_avail()) 
                    throw std::length_error("util::buffer::SimpleBuffers too long");
                return const_byte_buffers(read_, byte_iterator(read_) + size);
            }

        public:
            size_t in_avail() const
            {
                return write_.position() - read_.position();
            }

            //size_t out_avail() const
            //{
                //return capacity_ - in_avail();
            //}

            size_t in_position() const
            {
                return read_.position();
            }

            size_t out_position() const
            {
                return write_.position();
            }

            byte_iterator read_pos() const
            {
                return read_;
            }

            byte_iterator write_pos() const
            {
                return write_;
            }

        public:
            // Consume the specified number of bytes from the buffers.
            void commit(
                std::size_t size)
            {
                size_t rpos = in_position();
                size_t wpos = out_position();
                write_.seek_off(end_, size);
                check();
                assert(rpos == in_position());
                assert(wpos + size == out_position());
                (void)rpos;
                (void)wpos;
            }

            void consume(
                std::size_t size)
            {
                size_t rpos = in_position();
                size_t wpos = out_position();
                read_.seek_off(size);
                check();
                assert(rpos + size == in_position());
                assert(wpos == out_position());
                (void)rpos;
                (void)wpos;
            }

        public:
            void reset(
                byte_iterator beg)
            {
                beg_ = beg;
                reset();
            }

            void reset(
                byte_iterator beg, 
                byte_iterator end)
            {
                beg_ = beg;
                end_ = end;
                reset();
            }

            template <typename Buffers>
            void reset(
                Buffers const & buffers, 
                size_t avail = 0)
            {
                reset(buffers.begin(), buffers.end());
                reset(avail);
            }

            void reset(
                size_t avail = 0)
            {
                if (avail) {
                    write_.reset(beg_ + avail);
                } else if (is_const_buffer_iterator<BufferIterator>::value) {
                    write_.reset(end_);
                } else {
                    write_.reset(beg_);
                }
                read_.reset(beg_);
            }

        private:
            virtual int_type underflow()
            {
                if (read_ != write_) {
                    read_.next_buffer();
                    if (this->gptr() != this->pptr()) {
                        check();
                        return traits_type::to_int_type(*this->gptr());
                    } else {
                        //assert(0);
                        return traits_type::eof();
                    }
                } else {
                    // assert(0);
                    return traits_type::eof();
                }
            }

            virtual int_type overflow(
                int_type c)
            {
                if (!traits_type::eq_int_type(c, traits_type::eof()))
                {
                    //assert(this->gptr() == this->pptr());
                    write_.next_buffer(end_);
                    assert(this->gptr() != this->pptr());
                    *this->pptr() = traits_type::to_char_type(c);
                    this->pbump(1);
                    check();
                    return c;
                }
                return traits_type::not_eof(c);
            }

            virtual pos_type seekoff(
                off_type off, 
                std::ios_base::seekdir dir,
                std::ios_base::openmode mode)
            {
                pos_type pos = 0;
                if (dir == std::ios_base::beg) {
                    pos = off;
                } else if (dir == std::ios_base::cur) {
                    if (mode == std::ios_base::in) {
                        pos = read_.position() + off;
                    } else if (mode == std::ios_base::out) {
                        pos = write_.position() + off;
                    } else {
                        return pos_type(-1);
                    }
                } else {
                    return pos_type(-1);
                }
                check();
                return seekpos(pos, mode);
            }

            virtual pos_type seekpos(
                pos_type pos, 
                std::ios_base::openmode mode)
            {
                if (mode == std::ios_base::in) {
                    if (pos > write_.position() || pos < 0) {
                        return pos_type(-1);
                    }
                    if (pos > read_.position()) {
                        read_.seek_off((size_t)pos - read_.position());
                    } else if (pos < read_.position()) {
                        read_.reset(beg_);
                        read_.seek_off(pos);
                    }
                    assert(pos == read_.position());
                } else if (mode == std::ios_base::out) {
                    if (is_const_buffer_iterator<BufferIterator>::value)
                        return pos_type(-1);
                    if (pos < read_.position()) {
                        return pos_type(-1);
                    }
                    if (pos > write_.position()) {
                        write_.seek_off(end_, (size_t)pos - write_.position());
                    } else if (pos < write_.position()) {
                        write_ = read_;
                        if (pos > write_.position())
                            write_.seek_off(end_, (size_t)pos - write_.position());
                    }
                    assert(pos == write_.position());
                } else { // mode == std::ios_base::in | std::ios_base::out
                    if (is_const_buffer_iterator<BufferIterator>::value)
                        return pos_type(-1);
                    reset();
                    write_.seek_off(end_, pos);
                    read_ = write_;
                }
                return pos;
            }

            void check()
            {
                assert(write_.position() >= read_.position());
                if (write_.buffer_iter() == read_.buffer_iter()) {
                    assert(this->egptr() == this->pbase() || this->epptr() == this->eback());
                }
            }

        private:
            byte_iterator beg_;
            byte_iterator end_;
            write_buffer write_;
            read_buffer read_;
        };

    } // namespace buffer
} // namespace util

#endif // _UTIL_BUFFERS_CYCLE_BUFFERS_H_
