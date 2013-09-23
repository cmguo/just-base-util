// HttpSource.h

#ifndef _UTIL_STREAM_URL_HTTP_SOURCE_H_
#define _UTIL_STREAM_URL_HTTP_SOURCE_H_

#include "util/stream/UrlSource.h"

#include <util/protocol/http/HttpClient.h>

namespace util
{
    namespace stream
    {

        class HttpSource
            : public UrlSource
        {
        public:
            HttpSource(
                boost::asio::io_service & io_svc);

            virtual ~HttpSource();

        public:
            virtual bool open(
                framework::string::Url const & url,
                boost::uint64_t beg, 
                boost::uint64_t end, 
                boost::system::error_code & ec);

            virtual void async_open(
                framework::string::Url const & url,
                boost::uint64_t beg, 
                boost::uint64_t end, 
                response_type const & resp);

            virtual bool is_open(
                boost::system::error_code & ec);

            virtual bool close(
                boost::system::error_code & ec);

            virtual boost::uint64_t total(
                boost::system::error_code & ec);

        public:
            virtual bool recoverable(
                boost::system::error_code const & ec);

        public:
            virtual bool cancel(
                boost::system::error_code & ec);

        public:
            virtual bool set_non_block(
                bool non_block, 
                boost::system::error_code & ec);

            virtual bool set_time_out(
                boost::uint32_t time_out, 
                boost::system::error_code & ec);

        private:
            // implement util::stream::Source
            virtual std::size_t private_read_some(
                buffers_t const & buffers,
                boost::system::error_code & ec);

            virtual void private_async_read_some(
                buffers_t const & buffers, 
                handler_t const & handler);

        protected:
            bool flag_;
            util::protocol::HttpClient http_;
        };

        UTIL_REGISTER_URL_SOURCE("http", HttpSource);

    } // namespace stream
} // namespace util

#endif // _UTIL_STREAM_URL_HTTP_SOURCE_H_
