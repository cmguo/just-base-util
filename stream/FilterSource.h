// FilterSource.h

#ifndef _UTIL_STREAM_FILTER_SOURCE_H_
#define _UTIL_STREAM_FILTER_SOURCE_H_

#include "util/stream/Source.h"

#include <boost/iostreams/filtering_stream.hpp>
#include <boost/asio/streambuf.hpp>

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
                Source & source);

            virtual ~FilterSource();        public:
            void complete();            Source & source()            {                return source_;            }
        private:
            friend class read_handler;

            size_t filter_read(
                buffers_t const & buffers, 
                boost::system::error_code & ec);

        private:
            // 内部filter设置类型为buffered_call
            virtual std::size_t private_read_some(
                buffers_t const & buffers,
                boost::system::error_code & ec);

            // 内部filter设置类型为buffered_call
            virtual void private_async_read_some(
                buffers_t const & buffers, 
                handler_t const & handler);

        private:
            Source & source_;
            boost::asio::streambuf buf_;
        };

    } // stream
} // util

#endif // _UTIL_STREAM_FILTER_SOURCE_H_
