// FileSink.h

#ifndef _UTIL_STREAM_URL_FILE_SINK_H_
#define _UTIL_STREAM_URL_FILE_SINK_H_

#include "util/stream/UrlSink.h"

#include <framework/filesystem/File.h>

namespace util
{
    namespace stream
    {

        class FileSink
            : public UrlSink
        {
        public:
            FileSink(
                boost::asio::io_service & io_svc);

            virtual ~FileSink();

        public:
            virtual bool open(
                framework::string::Url const & url,
                boost::uint64_t beg, 
                boost::uint64_t end, 
                boost::system::error_code & ec);

            using UrlSink::open;

            virtual bool is_open(
                boost::system::error_code & ec);

            virtual bool close(
                boost::system::error_code & ec);

            virtual boost::uint64_t total(
                boost::system::error_code & ec);

        private:
            // implement util::stream::Sink
            virtual std::size_t private_write_some(
                boost::asio::const_buffer const & buffer,
                boost::system::error_code & ec);

        private:
            framework::filesystem::File file_;
            bool is_open_;
        };

        UTIL_REGISTER_URL_SINK("file", FileSink);

    } // namespace stream
} // namespace util

#endif // _UTIL_STREAM_URL_FILE_SINK_H_
