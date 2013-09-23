// PipeSource.h

#ifndef _UTIL_STREAM_URL_PIPE_SOURCE_H_
#define _UTIL_STREAM_URL_PIPE_SOURCE_H_

#include <util/stream/UrlSource.h>

#ifndef BOOST_WINDOWS_API
#  include <boost/asio/posix/stream_descriptor.hpp>
typedef boost::asio::posix::stream_descriptor source_descriptor;
#else
#  include <boost/asio/windows/stream_handle.hpp>
#  if (defined BOOST_ASIO_HAS_WINDOWS_STREAM_HANDLE)
typedef boost::asio::windows::stream_handle source_descriptor;
#  else
#    define UTIL_NO_PIPE_SOURCE
struct source_descriptor
{
	typedef HANDLE native_type;
};
#  endif
#endif

namespace util
{
    namespace stream
    {

        class PipeSource
            : public UrlSource
        {
        public:
            typedef source_descriptor::native_type native_descriptor;

        public:
            PipeSource(
                boost::asio::io_service & io_svc);

            virtual ~PipeSource();

        public:
            virtual bool open(
                framework::string::Url const & url,
                boost::uint64_t beg, 
                boost::uint64_t end, 
                boost::system::error_code & ec);

            virtual bool is_open(
                boost::system::error_code & ec);

            virtual bool close(
                boost::system::error_code & ec);

        public:
            virtual bool set_non_block(
                bool non_block, 
                boost::system::error_code & ec);

        private:
            // implement util::stream::Source
            virtual std::size_t private_read_some(
                buffers_t const & buffers,
                boost::system::error_code & ec);

        private:
            source_descriptor descriptor_;
            bool is_open_;
        };

#ifndef UTIL_NO_PIPE_SOURCE
        UTIL_REGISTER_URL_SOURCE("pipe", PipeSource);
#endif

    } // namespace stream
} // namespace util

#endif // _UTIL_STREAM_URL_PIPE_SOURCE_H_
