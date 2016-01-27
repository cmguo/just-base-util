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

        UrlSource::UrlSource(
            boost::asio::io_service & io_svc)
            : Source(io_svc)
            , UrlBase(io_svc)
        {
        }

        boost::system::error_code UrlSourceTraits::error_not_found()
        {
            return error::unknown_url_proto;
        }

        UrlSource * UrlSourceFactory::create(
            boost::asio::io_service & io_svc,
            std::string const & proto, 
            boost::system::error_code & ec)
        {
            return factory_type::create(proto, io_svc, ec);
        }

    } // namespace stream
} // namespace util
