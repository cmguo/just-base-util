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
                    buf_ = new Buf;
                    buf_->nref = 1;
                    buf_->count = count;
                    buf_->buffers = new Buffer[count];
                    std::copy(beg, end, buf_->buffers);
                }
            }

            StreamBuffers(
                StreamBuffers const & r)
                : buf_(r.buf_)
            {
                ++buf_->nref;
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
                return buf_->buffers;
            }

            const_iterator end() const
            {
                return buf_->buffers + buf_->count;
            }

        public:
            void reset()
            {
                swap(StreamBuffers(), *this);
            }

        private:
            struct Buf
            {
                size_t nref;
                size_t count;
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
