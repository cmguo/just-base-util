// BuffersBufferIterator.h

#ifndef _UTIL_BUFFERS_BUFFERS_BUFFER_ITERATOR_H_
#define _UTIL_BUFFERS_BUFFERS_BUFFER_ITERATOR_H_

#include "util/buffers/BuffersPosition.h"

#include <boost/iterator/iterator_facade.hpp>

namespace util
{
    namespace buffers
    {

        template <
            typename Buffer, 
            typename BufferIterator
        >
        class BuffersBufferIterator
            : public boost::iterator_facade<
                BuffersBufferIterator<Buffer, BufferIterator>,
                Buffer const, 
                boost::forward_traversal_tag
            >
        {
        public:
            typedef BuffersPosition<Buffer, BufferIterator> Position;

        public:
            BuffersBufferIterator(
                BufferIterator const & beg, 
                BufferIterator const & end, 
                size_t off = 0)
                : beg_(beg, end, off)
                , end_(end)
            {
            }

            BuffersBufferIterator(
                Position const & beg, 
                Position const & end)
                : beg_(beg)
                , end_(end)
            {
                beg_.set_end(end);
            }

            BuffersBufferIterator()
            {
            }

        private:
            friend class boost::iterator_core_access;

            void increment()
            {
                beg_.increment_buffer(end_);
            }

            bool equal(
                BuffersBufferIterator const & r) const
            {
                return beg_.equal(r.beg_);
            }

            Buffer const & dereference() const
            {
                return beg_.dereference_buffer();
            }

        private:
            Position beg_;
            Position end_;
        };

    } // namespace buffers
} // namespace util

#endif // _UTIL_BUFFERS_BUFFERS_BUFFER_ITERATOR_H_
