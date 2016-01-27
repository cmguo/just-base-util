// FilterSink.h

#ifndef _UTIL_TREAM_FILTER_SINK_H_
#define _UTIL_TREAM_FILTER_SINK_H_

#include "util/stream/Sink.h"

#include <boost/iostreams/filtering_stream.hpp>
#include <boost/asio/streambuf.hpp>

namespace util
{
    namespace stream
    {

        class FilterSink
            : public boost::iostreams::filtering_ostream
            , public Sink
        {
        public:
            FilterSink(
                Sink & sink);

            virtual ~FilterSink();

        public:
            void complete();

            Sink & sink()
            {
                return sink_;
            }

        private:
            friend class write_handler;

            size_t filter_write(
                buffers_t const & buffers, 
                boost::system::error_code & ec);

        private:
            virtual size_t private_write_some(
                buffers_t const & buffers, 
                boost::system::error_code & ec);

            // 内部filter设置类型为buffered_call
            virtual void private_async_write_some(
                buffers_t const & buffers, 
                handler_t const & handler);

        private:
            Sink & sink_;
            boost::asio::streambuf buf_;
        };

    } // stream
} // util

#endif // _UTIL_TREAM_FILTER_SINK_H_
