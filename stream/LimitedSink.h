// LimitedSink.h

#ifndef _UTIL_STREAM_LIMITED_SINK_H_
#define _UTIL_STREAM_LIMITED_SINK_H_

#include "util/stream/Sink.h"

namespace util
{
    namespace stream
    {

        class LimitedSink
            : public Sink
        {
        public:
            LimitedSink(
                Sink & sink, 
                boost::uint64_t limit);

        public:
            Sink & sink()
            {
                return sink_;
            }

        protected:
            virtual std::size_t private_write_some(
                StreamConstBuffers const & buffers,
                boost::system::error_code & ec);

            virtual void private_async_write_some(
                StreamConstBuffers const & buffers, 
                StreamHandler const & handler);

        private:
            Sink & sink_;
            boost::uint64_t limit_;
        };

    } // namespace stream
} // namespace util

#endif // _UTIL_STREAM_LIMITED_SINK_H_
