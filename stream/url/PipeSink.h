// PipeSink.h

#ifndef _UTIL_STREAM_URL_PIPE_SINK_H_
#define _UTIL_STREAM_URL_PIPE_SINK_H_

#include "util/stream/UrlSink.h"

#ifndef BOOST_WINDOWS_API
#  include <boost/asio/posix/stream_descriptor.hpp>
typedef boost::asio::posix::stream_descriptor sink_descriptor;
#else
#  include <boost/asio/windows/stream_handle.hpp>
#  if (defined BOOST_ASIO_HAS_WINDOWS_STREAM_HANDLE)
typedef boost::asio::windows::stream_handle sink_descriptor;
#  else
#    define UTIL_NO_PIPE_SINK
struct sink_descriptor
{
    typedef HANDLE native_type;
};
#  endif
#endif

namespace util
{
    namespace stream
    {

        class PipeSink
            : public UrlSink
        {
        public:
            typedef sink_descriptor::native_type native_descriptor;

        public:
            PipeSink(
                boost::asio::io_service & io_svc);

            virtual ~PipeSink();

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

        public:
            virtual bool set_non_block(
                bool non_block, 
                boost::system::error_code & ec);

        private:
            // implement util::stream::Sink
            virtual std::size_t private_write_some(
                buffers_t const & buffers,
                boost::system::error_code & ec);

        private:
            sink_descriptor descriptor_;
            bool is_open_;
        };

#ifndef UTIL_NO_PIPE_SINK
        UTIL_REGISTER_URL_SINK("pipe", PipeSink);
#endif

    } // namespace stream
} // namespace util

#endif // _UTIL_STREAM_URL_PIPE_SINK_H_
