// FindIterator.h

#ifndef _UTIL_BUFFERS_FIND_ITERATOR_H_
#define _UTIL_BUFFERS_FIND_ITERATOR_H_

#include "util/buffers/BuffersSize.h"
#include "util/buffers/ByteIterator.h"
#include "util/buffers/BuffersCompare.h"

#include <boost/iterator/iterator_facade.hpp>

namespace util
{
    namespace buffers
    {

        namespace detail
        {

            template <
                typename BufferIterator
            >
            ByteIterator<BufferIterator> buffers_char(
                ByteIterator<BufferIterator> const & beg, 
                ByteIterator<BufferIterator> const & end, 
                char chr, 
                size_t & skipped_bytes)
            {
                ByteBufferIterator<BufferIterator> beg2(beg, end);
                ByteBufferIterator<BufferIterator> end2(end);
                while (beg2 != end2) {
                    char const * buf = boost::asio::buffer_cast<char const *>(*beg2);
                    size_t size = boost::asio::buffer_size(*beg2);
                    char const * buf1 = (char const *)::memchr(buf, chr, size);
                    if (buf1) {
                        skipped_bytes += (buf1 - buf);
                        return beg2.byte_iter() + (buf1 - buf);
                    }
                    skipped_bytes += size;
                    ++beg2;
                }
                return end;
            }

        } // namespace detail

        template <
            typename BufferIterator
        >
        class FindCharIterator
            : public boost::iterator_facade<
                FindCharIterator<BufferIterator>,
                ByteIterator<BufferIterator> const, 
                boost::forward_traversal_tag>
        {
        public:
            typedef ByteIterator<BufferIterator> byte_iterator;

        public:
            FindCharIterator(
                BufferIterator const & beg, 
                BufferIterator const & end, 
                int chr)
                : beg_(beg)
                , end_(end)
                , chr_(chr)
                , reset_postion_(false)
                , skipped_bytes_(0)
            {
                beg_ = detail::buffers_char(beg_, end_, chr_, skipped_bytes_);
            }

            FindCharIterator(
                byte_iterator const & beg, 
                byte_iterator const & end, 
                int chr)
                : beg_(beg)
                , end_(end)
                , chr_(chr)
                , reset_postion_(false)
                , skipped_bytes_(0)
            {
                beg_ = detail::buffers_char(beg_, end_, chr_, skipped_bytes_);
            }

            template <typename ConstBufferSequence>
            FindCharIterator(
                ConstBufferSequence const & buffers, 
                int chr)
                : beg_(buffers.begin())
                , end_(buffers.end())
                , chr_(chr)
                , reset_postion_(false)
                , skipped_bytes_(0)
            {
                beg_ = detail::buffers_char(beg_, end_, chr_, skipped_bytes_);
            }

            FindCharIterator(
                BufferIterator const & end)
                : beg_(end)
                , end_(end)
                , chr_(0)
                , reset_postion_(false)
                , skipped_bytes_(0)
            {
            }
            
        public:
            size_t skipped_bytes() const
            {
                return skipped_bytes_;
            }

            void skip_bytes(
                size_t size)
            {
                beg_ += size;
                skipped_bytes_ += size;
                reset_postion_ = true;
            }

            template <
                typename BufferIterator2
            >
            int compare_continue(
                BufferIterator2 const & pos2, 
                BufferIterator2 const & end2, 
                size_t size)
            {
                ByteBufferIterator<BufferIterator> pos1(beg_, end_);
                ByteBufferIterator<BufferIterator> end1(end_);
                return buffers_compare(pos1, end1, pos2, end2, size);
            }

            bool at_end() const
            {
                return beg_ == end_;
            }

        protected:
            friend class boost::iterator_core_access;

            void increment()
            {
                if (!reset_postion_) {
                    ++beg_;
                    ++skipped_bytes_;
                }
                beg_ = detail::buffers_char(beg_, end_, chr_, skipped_bytes_);
                reset_postion_ = false;
            }

            bool equal(
                const FindCharIterator & other) const
            {
                return beg_ == other.beg_;
            }

            byte_iterator const & dereference() const
            {
                return beg_;
            }

        protected:
            byte_iterator beg_;
            byte_iterator end_;
            char chr_;
            bool reset_postion_;
            size_t skipped_bytes_;
        };

        template <
            typename BufferIterator, 
            typename ConstBufferSequence2
        >
        class FindIterator
            : public boost::iterator_facade<
                FindIterator<BufferIterator, ConstBufferSequence2>, 
                ByteIterator<BufferIterator> const, 
                boost::forward_traversal_tag>
        {
        public:
            typedef ByteIterator<BufferIterator> byte_iterator;

            typedef typename ConstBufferSequence2::value_type Buffer2;

            typedef typename ConstBufferSequence2::const_iterator BufferIterator2;

            typedef FindCharIterator<BufferIterator> CharIterator;

        public:
            FindIterator(
                BufferIterator const & beg, 
                BufferIterator const & end, 
                ConstBufferSequence2 const & find)
                : char_iter_(beg, end, first_char(find))
                , reset_postion_(false)
                , beg2_(find.begin())
                , end2_(find.end())
                , size_find_(buffers_size(find))
            {
                while (char_iter_.at_end() && char_iter_.compare_continue(beg2_, end2_, size_find_) != 0)
                    ++char_iter_;
            }

            template <typename ConstBufferSequence1>
            FindIterator(
                ConstBufferSequence1 const & buffers, 
                ConstBufferSequence2 const & find)
                : char_iter_(buffers, first_char(find))
                , reset_postion_(false)
                , beg2_(find.begin())
                , end2_(find.end())
                , size_find_(buffers_size(find))
            {
                while (char_iter_.at_end() && char_iter_.compare_continue(beg2_, end2_, size_find_) != 0)
                    ++char_iter_;
            }

            FindIterator(
                BufferIterator const & end)
                : char_iter_(end)
                , reset_postion_(false)
                , size_find_(0)
            {
            }

        public:
            size_t skipped_bytes() const
            {
                return char_iter_.skipped_bytes();
            }

            void skip_bytes(
                size_t size)
            {
                char_iter_.skip_bytes(size);
                reset_postion_ = true;
            }

        private:
            static int first_char(
                ConstBufferSequence2 const & find)
            {
                typename ConstBufferSequence2::const_iterator iter = find.begin();
                typename ConstBufferSequence2::const_iterator end = find.end();
                while (iter != end) {
                    size_t n = boost::asio::buffer_size(buffer(*iter));
                    if (n > 0)
                        break;
                    ++iter;
                }
                int chr = *boost::asio::buffer_cast<char const *>(buffer(*iter));
                return chr;
            }
            
        private:
            friend class boost::iterator_core_access;

            void increment()
            {
                if (!reset_postion_)
                    char_iter_.skip_bytes(size_find_);
                ++char_iter_;
                while (!char_iter_.at_end() && char_iter_.compare_continue(beg2_, end2_, size_find_) != 0) {
                    ++char_iter_;
                }
                reset_postion_ = false;
            }

            bool equal(
                const FindIterator & other) const
            {
                return char_iter_ == other.char_iter_;
            }

            byte_iterator const & dereference() const
            {
                return *char_iter_;
            }

        private:
            CharIterator char_iter_;
            bool reset_postion_;
            BufferIterator2 beg2_;
            BufferIterator2 end2_;
            size_t size_find_;
        };

        template <typename ConstBufferSequence>
        ByteIterator<
            typename ConstBufferSequence::const_iterator
        > buffers_char(
            ConstBufferSequence const & buffers, 
            int chr)
        {
            typedef FindCharIterator<
                typename ConstBufferSequence::const_iterator
            > Iterator;
            return Iterator(buffers.begin(), buffers.end(), chr).position();
        }

        template <typename ConstBufferSequence1, typename ConstBufferSequence2>
        ByteIterator<
            typename ConstBufferSequence1::const_iterator
        > buffers_find(
            ConstBufferSequence1 const & buffers, 
            ConstBufferSequence2 const & find)
        {
            typedef FindIterator<
                typename ConstBufferSequence1::const_iterator, 
				ConstBufferSequence2
            > Iterator;
            return Iterator(buffers.begin(), buffers.end(), find).position();
        }

    } // namespace buffers
} // namespace util

#endif // _UTIL_BUFFERS_FIND_ITERATOR_H_
