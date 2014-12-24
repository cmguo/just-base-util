// BuffersFind.h

#ifndef _UTIL_BUFFERS_BUFFERS_BYTE_ITERATOR_H_
#define _UTIL_BUFFERS_BUFFERS_BYTE_ITERATOR_H_

#include "util/buffers/ByteIterator.h"

#include <boost/iterator/iterator_facade.hpp>

namespace util
{
    namespace buffers
    {

        template <
            typename BufferIterator
        >
        class ByteBuffersIterator
            : public boost::iterator_facade<
                ByteBuffersIterator<BufferIterator>,
                typename BufferIteratorTraits<BufferIterator>::buffer_type const, 
                boost::forward_traversal_tag
            >
        {
        public:
            typedef ByteIterator<BufferIterator> byte_iterator;
            typedef typename byte_iterator::byte_t byte_t;
            typedef typename byte_iterator::buffer_t buffer_t;

        public:
            ByteBuffersIterator(
                BufferIterator const & beg, 
                BufferIterator const & end, 
                size_t off = 0)
                : beg_(beg, off)
                , end_(end)
            {
                update();
            }

            ByteBuffersIterator(
                byte_iterator const & beg, 
                byte_iterator const & end)
                : beg_(beg)
                , end_(end)
            {
                update();
            }

            ByteBuffersIterator()
            {
            }

        private:
            void update()
            {
                if (beg_.buffer_iter() == end_.buffer_iter()) {
                    byte_t * b = beg_.buffer_ptr();
                    size_t o1 = beg_.buffer_offset();
                    size_t o2 = end_.buffer_offset();
                    if (o1 < o2)
                        buf_ = boost::asio::buffer(b + o1, o2 - o1); 
                } else {
                    buf_ = beg_.buffer() + beg_.buffer_offset(); 
                }
            }

        private:
            friend class boost::iterator_core_access;

            void increment()
            {
                assert (beg_ != end_);
                if (beg_.buffer_iter() == end_.buffer_iter()) {
                    beg_ = end_;
                } else {
                    beg_.increment_buffer();
                }
                update();
            }

            bool equal(
                ByteBuffersIterator const & r) const
            {
                assert(r.beg_ == r.end_);
                return beg_ == end_;
            }

            buffer_t const & dereference() const
            {
                return buf_;
            }

        private:
            byte_iterator beg_;
            byte_iterator end_;
            buffer_t buf_;
        };

        template <
            typename BufferIterator
        >
        class ByteBuffers
        {
        public:
            typedef ByteIterator<BufferIterator> byte_iterator;
            typedef ByteBuffersIterator<BufferIterator> const_iterator;
            typedef typename const_iterator::value_type value_type;

        public:
            ByteBuffers(
                const_iterator beg, 
                const_iterator end = const_iterator())
                : beg_(beg)
                , end_(end, end)
            {
            }

            ByteBuffers(
                byte_iterator beg, 
                byte_iterator end = byte_iterator())
                : beg_(beg, end)
                , end_(end, end)
            {
            }

        public:
            const_iterator begin() const
            {
                return beg_;
            }

            const_iterator end() const
            {
                return end_;
            }
        private:
            const_iterator beg_;
            const_iterator end_;
        };

    } // namespace mux
} // namespace ppbox

#endif // _UTIL_BUFFERS_BUFFERS_BYTE_ITERATOR_H_
