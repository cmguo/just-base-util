// LimitedSource.h

#ifndef _UTIL_STREAM_LIMITED_SOURCE_H_
#define _UTIL_STREAM_LIMITED_SOURCE_H_

#include "util/stream/Source.h"

namespace util
{
    namespace stream
    {

        class LimitedSource
            : public Source
        {
        public:
            LimitedSource(
                Source & source, 
                boost::uint64_t limit);

        public:
            Source & source()
            {
                return source_;
            }

        protected:
            virtual std::size_t private_read_some(
                StreamMutableBuffers const & buffers,
                boost::system::error_code & ec);

            virtual void private_async_read_some(
                StreamMutableBuffers const & buffers, 
                StreamHandler const & handler);

        private:
            Source & source_;
            boost::uint64_t limit_;
        };

    } // namespace stream
} // namespace util

#endif // _UTIL_STREAM_LIMITED_SOURCE_H_
