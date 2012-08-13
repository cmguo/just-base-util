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
                : count_(0)
                , buffers_(NULL)
            {
            }

            template <
                typename BufferSequence
            >
            StreamBuffers(
                BufferSequence const & buffers)
                : count_(0)
                , buffers_(NULL)
            {
                typename BufferSequence::const_iterator beg = buffers.begin();
                typename BufferSequence::const_iterator end = buffers.end();
                count_ = std::distance(beg, end);
                if (count_ > 0) {
                    buffers_ = new Buffer[count_];
                    std::copy(beg, end, buffers_);
                }
            }

            StreamBuffers(
                StreamBuffers & r)
                : count_(0)
                , buffers_(NULL)
            {
                swap(*this, r);
            }

            ~StreamBuffers()
            {
                if (buffers_)
                    delete [] buffers_;
            }

        public:
            const_iterator begin() const
            {
                return buffers_;
            }

            const_iterator end() const
            {
                return buffers_ + count_;
            }

        public:
            void reset()
            {
                swap(StreamBuffers(), *this);
            }

        private:
            friend void swap(
                StreamBuffers & l, 
                StreamBuffers & r)
            {
                std::swap(l.count_, r.count_);
                std::swap(l.buffers_, r.buffers_);
            }

        private:
            mutable size_t count_;
            mutable Buffer * buffers_;
        };

        typedef StreamBuffers<boost::asio::const_buffer> StreamConstBuffers;
        typedef StreamBuffers<boost::asio::mutable_buffer> StreamMutableBuffers;

    } // namespace stream
} // namespace util

#endif // _UTIL_STREAM_SOURCE_H_
