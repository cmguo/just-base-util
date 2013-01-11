// BuffersPosition.h

#ifndef _UTIL_BUFFERS_BUFFERS_POSITION_H_
#define _UTIL_BUFFERS_BUFFERS_POSITION_H_

#include <boost/asio/buffer.hpp>

namespace util
{
    namespace buffers
    {

        template<
            typename Buffer
        >
        struct BufferByteType
        {
            typedef char type;
        };

        template<
        >
        struct BufferByteType<boost::asio::const_buffer>
        {
            typedef char const type;
        };

        template <
            typename Buffer, 
            typename BufferIterator
        >
        class BuffersPosition
        {
        public:
            typedef typename BufferByteType<Buffer>::type Byte;

        public:
            BuffersPosition(
                BufferIterator const & iter, 
                BufferIterator const & end, 
                size_t off = 0)
                : iter_(iter)
                , skipped_bytes_(0)
                , at_end_(false)
            {
                if (iter == end) {
                    at_end_ = true;
                } else {
                    buf_ = *iter_;
                    buf_ = buf_ + off;
                    if (boost::asio::buffer_size(buf_) == 0)
                        normalize(BuffersPosition(end));
                }
            }

            BuffersPosition(
                BufferIterator const & end)
                : iter_(end)
                , skipped_bytes_(0)
                , at_end_(true)
            {
            }

            BuffersPosition()
                : skipped_bytes_(0)
                , at_end_(true)
            {
            }

        public:
            void set_end(
                BuffersPosition const & end)
            {
                if (!at_end_ && iter_ == end.iter_) {
                    std::ptrdiff_t size = boost::asio::buffer_cast<char const *>(end.buf_) 
                        - boost::asio::buffer_cast<char const *>(buf_);
                    if (size <= 0) {
                        size = 0;
                        at_end_ = true;
                    }
                    // when at end, we should also clear buf_, because we will use (buf_.size() == 0) as end mark in increment_bytes
                    buf_ = boost::asio::buffer(buf_, size);
                }
            }

        public:
            bool at_end() const
            {
                return at_end_;
            }

            bool equal(
                BuffersPosition const & r) const
            {
                if (at_end_ && r.at_end_)
                    return true;
                return at_end_ == r.at_end_ 
                    && iter_ == r.iter_ 
                    && boost::asio::buffer_cast<void const *>(buf_) == boost::asio::buffer_cast<void const *>(r.buf_);
            }

            Byte & dereference_byte() const
            {
                return *boost::asio::buffer_cast<Byte *>(buf_);
            }

            Buffer const & dereference_buffer() const
            {
                return buf_;
            }

            void increment_byte(
                BuffersPosition const & end)
            {
                assert(!at_end_);
                if (at_end_) return;
                buf_ = buf_ + 1;
                ++skipped_bytes_;
                if (boost::asio::buffer_size(buf_) == 0) {
                    normalize(end);
                }
            }

            void increment_bytes(
                BuffersPosition const & end, 
                size_t size)
            {
                assert(!at_end_);
                size_t this_size = boost::asio::buffer_size(buf_);
                assert(this_size > 0);
                while (size > 0 && this_size > 0) {
                    if (this_size > size) {
                        buf_ = buf_ + size;
                        skipped_bytes_ += size;
                        return;
                    }
                    size -= this_size;
                    skipped_bytes_ += this_size;
                    this_size = normalize(end);
                }
                assert(size == 0);
            }

            void increment_buffer(
                BuffersPosition const & end)
            {
                assert(!at_end_);
                if (at_end_) return;
                skipped_bytes_ += boost::asio::buffer_size(buf_);
                normalize(end);
            }

            size_t skipped_bytes() const
            {
                return skipped_bytes_;
            }

        private:
            size_t normalize(
                BuffersPosition const & end)
            {
                assert(!at_end_);
                if (iter_ == end.iter_) {
                    buf_ = buf_ + boost::asio::buffer_size(buf_);
                    at_end_ = true;
                    return 0;
                }
                while (!at_end_) {
                    if (++iter_ == end.iter_) {
                        char const * end_ptr = boost::asio::buffer_cast<char const *>(end.buf_);
                        if (end_ptr == NULL) {
                            buf_ = end.buf_;
                            at_end_ = true;
                            return 0;
                        } else {
                            buf_ = *iter_;
                            std::ptrdiff_t size = end_ptr - boost::asio::buffer_cast<char const *>(buf_);
                            if (size <= 0) {
                                size = 0;
                                at_end_ = true;
                            }
                            // when at end, we should also clear buf_, because we will use (buf_.size() == 0) as end mark in increment_bytes
                            buf_ = boost::asio::buffer(buf_, size);
                            return size;
                        }
                    } else {
                        buf_ = *iter_;
                        if (boost::asio::buffer_size(buf_) > 0) {
                            return boost::asio::buffer_size(buf_);
                        }
                    }
                }
                // when at end, we should also clear buf_, because we will use (buf_.size() == 0) as end mark in increment_bytes
                buf_ = buf_ + boost::asio::buffer_size(buf_);
                at_end_ = true;
                return 0;
            }

        private:
            BufferIterator iter_;
            Buffer buf_;
            size_t skipped_bytes_;
            bool at_end_;
        };

        template <
            typename Value, 
            typename Iterator
        >
        class Container
        {
        public:
            typedef Value value_type;
            typedef Iterator const_iterator;

        public:
            Container(
                Iterator const & beg, 
                Iterator const & end = Iterator())
                : beg_(beg)
                , end_(end)
            {
            }

            const_iterator begin() const
            {
                return beg_;
            }

            const_iterator end() const
            {
                return end_;
            }

        private:
            Iterator const beg_;
            Iterator const end_;
        };

    } // namespace mux
} // namespace ppbox

#endif // _UTIL_BUFFERS_BUFFERS_POSITION_H_
