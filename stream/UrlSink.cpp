// UrlSink.cpp

#include "util/Util.h"
#include "util/stream/UrlSink.h"
#include "util/stream/StreamErrors.h"
#include "util/stream/url/SinkTypes.h"

#include <boost/bind.hpp>
#include <boost/asio/io_service.hpp>

namespace util
{
    namespace stream
    {

        boost::system::error_code UrlSink::error_not_found()
        {
            return error::unknown_url_proto;
        }

        UrlSink * UrlSink::create(
            boost::asio::io_service & io_svc,
            std::string const & proto, 
            boost::system::error_code & ec)
        {
            return factory_type::create(proto, io_svc, ec);
        }

        UrlSink::UrlSink(
            boost::asio::io_service & io_svc)
            : Sink(io_svc)
            , UrlBase(io_svc)
        {
        }

    } // namespace stream
} // namespace util
