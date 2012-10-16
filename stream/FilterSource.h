// FilterSource.h

#ifndef _UTIL_STREAM_FILTER_SOURCE_H_
#define _UTIL_STREAM_FILTER_SOURCE_H_

#include "util/stream/Source.h"

#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iostreams/categories.hpp>

namespace util
{
    namespace stream
    {

        class FilterSource
            : public boost::iostreams::filtering_istream
            , public Source
        {
        public:
            FilterSource(
                boost::asio::io_service & ios);

            virtual ~FilterSource();            using boost::iostreams::filtering_istream::push;            void push(
                Source & t);
        private:
            // 内部filter设置类型为buffered_call
            virtual std::size_t private_read_some(
                StreamMutableBuffers const & buffers,
                boost::system::error_code & ec);

            // 内部filter设置类型为buffered_call
            virtual void private_async_read_some(
                StreamMutableBuffers const & buffers, 
                StreamHandler const & handler);
        };

    } // stream
} // util

#endif // _UTIL_STREAM_FILTER_SOURCE_H_
