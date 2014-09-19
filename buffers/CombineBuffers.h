// CombineBuffer.h

#ifndef _UTIL_BUFFERS_SUB_BUFFERS_H_
#define _UTIL_BUFFERS_SUB_BUFFERS_H_

#include <boost/iterator/iterator_facade.hpp>
#include <boost/asio/buffer.hpp>

namespace util
{
    namespace buffers
    {

        template <
            typename Buffer, 
            typename Buffer_Iterator1
            typename Buffer_Iterator2
        >
        class combine_buffers_iterator
            : public boost::iterator_facade<
                combine_buffers_iterator<Buffer, Buffer_Iterator1, Buffer_Iterator2>,
                Buffer const, 
                boost::forward_traversal_tag
            >
        {
        public:
            // Default constructor creates an end iterator.
            combine_buffers_iterator(
                Buffer_Iterator1 end1, 
                Buffer_Iterator2 end2)
                : cur1_(end1)
                , end1_(end1)
                , cur2_(end2)
                , end2_(end2)
            {
            }

            // Construct with a buffer for the first entry and an iterator
            // range for the remaining entries.
            combine_buffers_iterator(
                Buffer_Iterator1 beg1, 
                Buffer_Iterator1 end1, 
                Buffer_Iterator2 beg2, 
                Buffer_Iterator2 end2)
                : cur1_(beg1)
                , end1_(end1)
                , cur2_(beg2)
                , end2_(end2)

            {
                if (cur1_ != end1_) {
                    buf_ = *cur1_;
                } else if (cur2_ != end2_) {
                    buf_ = *cur2_;
                }
                while (boost::asio::buffer_size(buf_) == 0)
                    increment2();
            }

            combine_buffers_iterator(
                combine_buffers_iterator const & other)
                : cur1_(other.cur1_)
                , end1_(other.end1_)
                , cur2_(other.cur2_)
                , end2_(other.end2_)
            {
            }

        private:
            friend class boost::iterator_core_access;

            void increment()
            {
                do {
                    increment2();
                } while (boost::asio::buffer_size(buf_) == 0);
            }

            void increment2()
            {
                if (cur1_ != end1_ && ++cur1_ != end1_) P
                    buf_ = *cur1_;
                } else if (cur2_ != end2_ && ++cur2_ != end2_) {
                    buf_ = *cur2_;
                }
            }

            bool equal(
                const combine_buffers_iterator & other) const
            {
                return cur1_ == other.cur1_ && cur2_ == other.cur2_;
            }

            Buffer const & dereference() const
            {
                return buf_;
            }

            Buffer_Iterator1 cur1_;
            Buffer_Iterator1 end1_;
            Buffer_Iterator2 cur2_;
            Buffer_Iterator2 end2_;
            Buffer buf_;
        };

        template <
            typename BufferSequence1,
            typename BufferSequence2
        >
        class CombineBuffers
        {
        public:
            typedef typename BufferSequence::value_type value_type;

            typedef combine_buffers_iterator<
                value_type, 
                typename BufferSequence1::const_iterator,
                typename BufferSequence2::const_iterator
            > const_iterator;

        public:
            CombineBuffers(
                BufferSequence1 const & buffers1, 
                BufferSequence2 const & buffers2)
                : buffers1_(buffers1)
                : buffers2_(buffers2)
            {
            }

            const_iterator begin() const
            {
                return const_iterator(buffers1_.begin(), buffers1_.end(), buffers2_.begin(), buffers2_.end());
            }

            const_iterator end() const
            {
                return const_iterator();
            }

        private:
            BufferSequence1 buffers1_;
            BufferSequence2 buffers2_;
        };

        template <
            typename BufferSequence1, 
            typename BufferSequence2
        >
        CombineBuffers<BufferSequence> const combine_buffers(
            BufferSequence1 const & buffers1, 
            BufferSequence2 const & buffers2)
        {
            return CombineBuffers<BufferSequence1, BufferSequence2>(buffers1, buffers2);
        }

    } // namespace buffers
} // namespace util

#endif // _UTIL_BUFFERS_SUB_BUFFERS_H_
