// UrlSource.cpp

#include "util/Util.h"
#include "util/stream/UrlSource.h"
#include "util/stream/StreamErrors.h"
#include "util/stream/url/SourceTypes.h"

#include <boost/bind.hpp>

namespace util
{
    namespace stream
    {

        boost::system::error_code UrlSource::error_not_found()
        {
            return error::unknown_url_proto;
        }

        UrlSource * UrlSource::create(
            boost::asio::io_service & io_svc,
            std::string const & proto, 
            boost::system::error_code & ec)
        {
            return factory_type::create(proto, io_svc, ec);
        }

        UrlSource::UrlSource(
            boost::asio::io_service & io_svc)
            : Source(io_svc)
            , UrlBase(io_svc)
        {
        }

    } // namespace stream
} // namespace util
