// LimitedSink.cpp

#include "util/Util.h"
#include "util/stream/LimitedSink.h"
#include "util/buffers/SubBuffers.h"
#include "util/buffers/BuffersSize.h"

#include <boost/bind.hpp>
#include <boost/asio/io_service.hpp>

namespace util
{
    namespace stream
    {

        LimitedSink::LimitedSink(
            Sink & sink, 
            boost::uint64_t limit)
            : Sink(sink.get_io_service())
              , sink_(sink)
              , limit_(limit)
        {
        }

        std::size_t LimitedSink::private_write_some(
            StreamConstBuffers const & buffers,
            boost::system::error_code & ec)
        {
            std::size_t bytes_write = util::buffers::buffers_size(buffers);
            if (bytes_write <= limit_)
                bytes_write = sink_.write_some(buffers, ec);
            else if (limit_)
                bytes_write = sink_.write_some(util::buffers::sub_buffers(buffers, 0, limit_));
            else
                ec = boost::asio::error::eof, bytes_write = 0;
            limit_ -= bytes_write;
            return bytes_write;
        }

        struct limited_write_handler
        {
            typedef void result_type;

            limited_write_handler(
                boost::uint64_t & limit, 
                StreamHandler handler)
                : limit_(limit)
                  , handler_(handler)
            {
            }

            void operator()(
                boost::system::error_code const & ec, 
                size_t bytes_write)
            {
                limit_ -= bytes_write;
                handler_(ec, bytes_write);
            }

        private:
            boost::uint64_t & limit_;
            StreamHandler handler_;
        };

        void LimitedSink::private_async_write_some(
            StreamConstBuffers const & buffers, 
            StreamHandler const & handler)
        {
            std::size_t bytes_write = util::buffers::buffers_size(buffers);
            if (bytes_write <= limit_)
                sink_.async_write_some(buffers, limited_write_handler(limit_, handler));
            else if (limit_)
                sink_.async_write_some(util::buffers::sub_buffers(buffers, 0, limit_), limited_write_handler(limit_, handler));
            else
                get_io_service().post(boost::bind(handler, boost::asio::error::eof, 0));
        }

    } // namespace stream
} // namespace util
