// StreamBuffers.h

#ifndef _UTIL_STREAM_STREAM_BUFFERS_H_
#define _UTIL_STREAM_STREAM_BUFFERS_H_

#include <boost/asio/buffer.hpp>

#include <utility>

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
            typedef Buffer const * const_iterator;

        public:
            StreamBuffers()
                : buf_(NULL)
            {
            }

            template <
                typename BufferSequence
            >
            StreamBuffers(
                BufferSequence const & buffers)
                : buf_(NULL)
            {
                typename BufferSequence::const_iterator beg = buffers.begin();
                typename BufferSequence::const_iterator end = buffers.end();
                size_t count = std::distance(beg, end);
                if (count > 0) {
                    reserve(count);
                    buf_->count = count;
                    std::copy(beg, end, buf_->buffers);
                }
            }

            StreamBuffers(
                StreamBuffers const & r)
                : buf_(r.buf_)
            {
                if (buf_) {
                    ++buf_->nref;
                }
            }

            ~StreamBuffers()
            {
                if (buf_ && --buf_->nref == 0) {
                    delete [] buf_->buffers;
                    delete buf_;
                }
            }

        public:
            const_iterator begin() const
            {
                return buf_ ? buf_->buffers : NULL;
            }

            const_iterator end() const
            {
                return buf_ ? buf_->buffers + buf_->count : NULL;
            }

            size_t size() const
            {
                return buf_ ? buf_->count : 0;
            }

            size_t capacity() const
            {
                return buf_ ? buf_->capacity : 0;
            }

        public:
            void reset()
            {
                StreamBuffers tmp;
                std::swap(tmp.buf_, buf_);
            }

            void reset(
                StreamBuffers const & r)
            {
                StreamBuffers tmp(r);
                std::swap(tmp.buf_, buf_);
            }

            StreamBuffers & operator=(
                StreamBuffers const & r)
            {
                reset(r);
                return *this;
            }

        public:
            void reserve(
                size_t count)
            {
                if (buf_) {
                    if (buf_->nref > 1) {
                        StreamBuffers tmp;
                        std::swap(tmp.buf_, buf_);
                        buf_ = new Buf;
                        buf_->nref = 1;
                        buf_->count = tmp.buf_->count;
                        buf_->capacity = count;
                        if (buf_->capacity < buf_->count)
                            buf_->capacity = buf_->count;
                        buf_->buffers = new Buffer[buf_->capacity];
                        if (buf_->count)
                            std::copy(tmp.begin(), tmp.end(), buf_->buffers);
                    } else if (count > buf_->capacity) {
                        Buffer * buffers = NULL;
                        std::swap(buffers, buf_->buffers);
                        buf_->capacity = count;
                        buf_->buffers = new Buffer[buf_->capacity];
                        if (buf_->count)
                            std::copy(buffers, buffers + buf_->count, buf_->buffers);
                        delete [] buffers;
                    }
                } else if (count > 0) {
                    buf_ = new Buf;
                    buf_->nref = 1;
                    buf_->count = 0;
                    buf_->capacity = count;
                    buf_->buffers = new Buffer[buf_->capacity];
                }
            }

            void push_back(
                Buffer const & buffer)
            {
                reserve(size() + 1);
                buf_->buffers[buf_->count] = buffer;
                ++buf_->count;
            }

            void clear()
            {
                if (buf_) {
                    if (buf_->nref > 1) {
                        reset();
                    } else {
                        buf_->count = 0;
                    }
                }
            }

        private:
            struct Buf
            {
                size_t nref;
                size_t count;
                size_t capacity;
                Buffer * buffers;
            };

        private:
            Buf * buf_;
        };

        typedef StreamBuffers<boost::asio::const_buffer> StreamConstBuffers;
        typedef StreamBuffers<boost::asio::mutable_buffer> StreamMutableBuffers;

    } // namespace stream
} // namespace util

#endif // _UTIL_STREAM_SOURCE_H_
