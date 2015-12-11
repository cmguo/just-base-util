// LimitedSource.cpp

#include "util/Util.h"
#include "util/stream/LimitedSource.h"
#include "util/buffers/SubBuffers.h"
#include "util/buffers/BuffersSize.h"

#include <boost/bind.hpp>
#include <boost/asio/io_service.hpp>

namespace util
{
    namespace stream
    {

        LimitedSource::LimitedSource(
            Source & source, 
            boost::uint64_t limit)
            : Source(source.get_io_service())
              , source_(source)
              , limit_(limit)
        {
        }

        std::size_t LimitedSource::private_read_some(
            StreamMutableBuffers const & buffers,
            boost::system::error_code & ec)
        {
            std::size_t bytes_read = util::buffers::buffers_size(buffers);
            if (bytes_read <= limit_)
                bytes_read = source_.read_some(buffers, ec);
            else if (limit_)
                bytes_read = source_.read_some(util::buffers::sub_buffers(buffers, 0, limit_), ec);
            else
                ec = boost::asio::error::eof, bytes_read = 0;
            limit_ -= bytes_read;
            return bytes_read;
        }

        struct limited_read_handler
        {
            typedef void result_type;

            limited_read_handler(
                boost::uint64_t & limit, 
                StreamHandler handler)
                : limit_(limit)
                  , handler_(handler)
            {
            }

            void operator()(
                boost::system::error_code const & ec, 
                size_t bytes_transferred)
            {
                limit_ -= bytes_transferred;
                handler_(ec, bytes_transferred);
            }

        private:
            boost::uint64_t & limit_;
            StreamHandler handler_;
        };

        void LimitedSource::private_async_read_some(
            StreamMutableBuffers const & buffers, 
            StreamHandler const & handler)
        {
            std::size_t bytes_read = util::buffers::buffers_size(buffers);
            if (bytes_read <= limit_)
                source_.async_read_some(buffers, limited_read_handler(limit_, handler));
            else if (limit_)
                source_.async_read_some(util::buffers::sub_buffers(buffers, 0, limit_), limited_read_handler(limit_, handler));
            else
                get_io_service().post(boost::bind(handler, boost::asio::error::eof, 0));
        }

    } // namespace stream
} // namespace util
