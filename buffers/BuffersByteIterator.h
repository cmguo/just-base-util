// BuffersFind.h

#ifndef _UTIL_BUFFERS_BUFFERS_BYTE_ITERATOR_H_
#define _UTIL_BUFFERS_BUFFERS_BYTE_ITERATOR_H_

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
        class BuffersByteIterator
            : public boost::iterator_facade<
                BuffersByteIterator<Buffer, BufferIterator>,
                typename BufferByteType<Buffer>::type, 
                boost::forward_traversal_tag
            >
        {
        public:
            typedef BuffersPosition<Buffer, BufferIterator> Position;

        public:
            BuffersByteIterator(
                BufferIterator const & beg, 
                BufferIterator const & end, 
                size_t off = 0)
                : beg_(beg, end, off)
                , end_(end)
            {
            }

        public:
            BuffersByteIterator(
                Position const & beg, 
                Position const & end)
                : beg_(beg)
                , end_(end)
            {
                beg_.set_end(end);
            }

            BuffersByteIterator()
            {
            }

        private:
            friend class boost::iterator_core_access;

            void increment()
            {
                beg_.increment_byte(end_);
            }

            bool equal(
                BuffersByteIterator const & r) const
            {
                return beg_.equal(r.beg_);
            }

            typename Position::Byte & dereference() const
            {
                return beg_.dereference_byte();
            }

        private:
            Position beg_;
            Position end_;
        };

    } // namespace mux
} // namespace ppbox

#endif // _UTIL_BUFFERS_BUFFERS_BYTE_ITERATOR_H_
