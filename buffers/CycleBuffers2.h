// CycleBuffers.h

#ifndef _UTIL_BUFFERS_CYCLE_BUFFERS_H_
#define _UTIL_BUFFERS_CYCLE_BUFFERS_H_

#include "util/buffers/SimpleIoBuffer.h"
#include "util/buffers/ByteBuffers.h"
#include "util/buffers/BuffersSize.h"

#include <iterator>

namespace util
{
    namespace buffers
    {

        template <
            typename BufferIterator
        >
        class CycleBufferIterator
            : public boost::iterator_facade<
                CycleBufferIterator<BufferIterator>,
                typename std::iterator_traits<BufferIterator>::value_type const, 
                boost::forward_traversal_tag
            >
        {
        public:
            typedef typename std::iterator_traits<BufferIterator>::value_type const buffer_t;

        public:
            CycleBufferIterator(
                BufferIterator const & beg = BufferIterator(), 
                BufferIterator const & end = BufferIterator(),
                bool is_read = false)
                : beg_(beg)
                , cur_(beg)
                , end_(end)
                , is_read_(is_read)
                , at_end_(beg == end || is_read)
            {
            }

        public:
            CycleBufferIterator & operator=(
                CycleBufferIterator const & r)
            {
                cur_ = r.cur_;
                if (is_read_ != r.is_read_) {
                    at_end_ = (beg_ == end_ || is_read_);
                }
                return *this;
            }

        private:
            friend class boost::iterator_core_access;

            void increment()
            {
                if (++cur_ == end_) {
                    cur_ = beg_;
                    at_end_ = true;
                }
            }

            bool equal(
                CycleBufferIterator const & r) const
            {
                if (cur_ == r.cur_) {
                    return is_read_ == r.is_read_ || at_end_;
                } else {
                    return false;
                }
            }

            buffer_t & dereference() const
            {
                return *cur_;
            }

        private:
            BufferIterator beg_;
            BufferIterator cur_;
            BufferIterator end_;
            bool is_read_;
            bool at_end_;
        };
        
        template <
            typename BufferIterator, 
            typename Elem = char, 
            typename Traits = std::char_traits<Elem>
        >
        class CycleBuffers
            : public StdIoStream<Elem, Traits>
        {
        public:
            typedef typename StdIoStream<Elem, Traits>::char_type char_type;

            typedef typename StdIoStream<Elem, Traits>::int_type int_type;

            typedef typename StdIoStream<Elem, Traits>::traits_type traits_type;

            typedef typename StdIoStream<Elem, Traits>::pos_type pos_type;
 
            typedef typename StdIoStream<Elem, Traits>::off_type off_type;
 
            typedef CycleBuffers simple_buffers_type;

            typedef StdIoStream<Elem, Traits> stream_type;

            typedef BufferIterator buffer_iterator;

            typedef CycleBufferIterator<BufferIterator> cycle_buffer_iterator;

            typedef ByteIterator<buffer_iterator> byte_iterator;

            typedef SimpleIoBuffer<
                StdIoMode<detail::_read>, cycle_buffer_iterator, char_type, traits_type> read_buffer;

            typedef SimpleIoBuffer<
                StdIoMode<detail::_write>, cycle_buffer_iterator, char_type, traits_type> write_buffer;

            typedef ByteBuffers<cycle_buffer_iterator> const_byte_buffers;

            typedef ByteBuffers<cycle_buffer_iterator> mutable_byte_buffers;

            //typedef ConstBuffers const_buffers_type;
            //typedef MutableBuffers mutable_buffers_type;

        public:
            CycleBuffers()
                : write_(*this, read_)
                , read_(*this, write_)
                , capacity_(0)
            {
            }

            template <typename Buffers>
            CycleBuffers(
                Buffers const & buffers)
                : beg_(buffers.begin())
                , end_(buffers.end())
                , write_(*this, read_, cycle_buffer_iterator(beg_, end_, false))
                , read_(*this, write_, cycle_buffer_iterator(beg_, end_, true))
            {
                capacity_ = buffers_size(beg_, end_);
                reset();
            }

            // Copy constructor.
            CycleBuffers(
                CycleBuffers const & other)
                : beg_(other.beg_)
                , end_(other.end_)
                , write_(*this, read_, other.write_)
                , read_(*this, write_, other.read_)
                , capacity_(other.capacity_)
            {
            }

        public:
            mutable_byte_buffers prepare() const
            {
                return mutable_byte_buffers(write_, read_);
            }

            mutable_byte_buffers prepare(
                size_t size) const
            {
                if (size > this->out_avail()) 
                    throw std::length_error("util::buffer::CycleStreamBuffers too long");
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
                    throw std::length_error("util::buffer::CycleStreamBuffers too long");
                return const_byte_buffers(read_, byte_iterator(read_) + size);
            }

        public:
            size_t in_avail() const
            {
                return write_.position() - read_.position();
            }

            size_t out_avail() const
            {
                return capacity_ - in_avail();
            }

            size_t in_position() const
            {
                return read_.position();
            }

            size_t out_position() const
            {
                return write_.position();
            }

        public:
            // Consume the specified number of bytes from the buffers.
            void commit(
                std::size_t size)
            {
                size_t rpos = in_position();
                size_t wpos = out_position();
                write_.seek_off(size);
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

            void reset()
            {
                read_.reset(cycle_buffer_iterator(beg_, end_, true));
                write_.reset(cycle_buffer_iterator(beg_, end_, false));
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
                    write_.next_buffer();
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
                    if (pos > write_.position() || (size_t)pos + capacity_ < write_.position()) {
                        return pos_type(-1);
                    }
                    if (pos > read_.position()) {
                        read_.seek_off((size_t)pos - read_.position());
                    } else if (pos < read_.position()) {
                        write_.inc_pos(capacity_);
                        read_ = write_;
                        read_.seek_off((size_t)pos + capacity_ - read_.position());
                        read_.inc_pos(-capacity_);
                        write_.inc_pos(-capacity_);
                    }
                    assert(pos == read_.position());
                } else if (mode == std::ios_base::out) {
                    if (pos < read_.position() || pos > capacity_ + read_.position()) {
                        return pos_type(-1);
                    }
                    if (pos > write_.position()) {
                        write_.seek_off((size_t)pos - write_.position());
                    } else if (pos < write_.position()) {
                        write_ = read_;
                        write_.seek_off((size_t)pos - write_.position());
                    }
                    assert(pos == write_.position());
                } else { // mode == std::ios_base::in | std::ios_base::out
                    reset();
                    pos_type pos1 = pos % capacity_;
                    write_.seek_off(pos1);
                    write_.inc_pos(pos - pos1);
                    read_ = write_;
                }
                return pos;
            }

            void check()
            {
                assert(write_.position() >= read_.position());
            }

        private:
            buffer_iterator beg_;
            buffer_iterator end_;
            write_buffer write_;
            read_buffer read_;
            size_t capacity_;
        };

    } // namespace buffer
} // namespace util

#endif // _UTIL_BUFFERS_CYCLE_BUFFERS_H_
