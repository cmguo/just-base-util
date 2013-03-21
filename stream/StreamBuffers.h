// StreamBuffers.h

#ifndef _UTIL_STREAM_STREAM_BUFFERS_H_
#define _UTIL_STREAM_STREAM_BUFFERS_H_

#include <boost/asio/buffer.hpp>
#include <boost/intrusive_ptr.hpp>

#include <utility>
#include <deque>

namespace util
{
    namespace stream
    {

        template <
            typename Buffer
        >
        class StreamBuffers
        {
        public:
            typedef Buffer value_type;
            typedef typename std::deque<Buffer>::const_iterator const_iterator;

        public:
            StreamBuffers()
            {
            }

            template <
                typename BufferSequence
            >
            StreamBuffers(
                BufferSequence const & buffers)
            {
                typename BufferSequence::const_iterator beg = buffers.begin();
                typename BufferSequence::const_iterator end = buffers.end();
                size_t count = std::distance(beg, end);
                if (count > 0) {
                    buf_.reset(new Buf);
                    std::copy(beg, end, std::back_inserter(*buf_));
                }
            }

        public:
            const_iterator begin() const
            {
                return buf_ ? buf_->begin() : const_iterator();
            }

            const_iterator end() const
            {
                return buf_ ? buf_->end(): const_iterator();
            }

            size_t size() const
            {
                return buf_ ? buf_->size() : 0;
            }

        public:
            void reset()
            {
                buf_.reset();
            }

            void reset(
                StreamBuffers const & r)
            {
                buf_.reset(r.buf_);
            }

            StreamBuffers & operator=(
                StreamBuffers const & r)
            {
                reset(r);
                return *this;
            }

        public:
            void push_front(
                Buffer const & buffer)
            {
                before_modify();
                buf_->push_front(buffer);
            }

            void push_back(
                Buffer const & buffer)
            {
                before_modify();
                buf_->push_back(buffer);
            }

            void pop_front()
            {
                before_modify();
                buf_->pop_front();
            }

            void pop_back(
                size_t n)
            {
                before_modify();
                buf_->pop_back();
            }

            void clear()
            {
                if (buf_) {
                    if (buf_->nref > 1) {
                        reset();
                    } else {
                        buf_->clear();
                    }
                }
            }

        private:
            void before_modify()
            {
                if (!buf_) {
                    buf_.reset(new Buf);
                } else if (buf_->nref > 1) {
                    buf_.reset(new Buf(*buf_));
                }
            }

        private:
            struct Buf
                : std::deque<Buffer>
            {
                Buf()
                    : nref(0)
                {
                }

                size_t nref;

                friend void intrusive_ptr_add_ref(
                    Buf * p)
                {
                    ++p->nref;
                }

                friend void intrusive_ptr_release(
                    Buf * p)
                {
                    if (--p->nref == 0) {
                        delete p;
                    }
                }
            };

        private:
            boost::intrusive_ptr<Buf> buf_;
        };

        typedef StreamBuffers<boost::asio::const_buffer> StreamConstBuffers;
        typedef StreamBuffers<boost::asio::mutable_buffer> StreamMutableBuffers;

    } // namespace stream
} // namespace util

#endif // _UTIL_STREAM_SOURCE_H_
