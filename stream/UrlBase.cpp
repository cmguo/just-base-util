// UrlBase.cpp

#include "util/Util.h"
#include "util/stream/UrlBase.h"

#include <framework/system/ErrorCode.h>

#include <boost/bind.hpp>
#include <boost/asio/io_service.hpp>

namespace util
{
    namespace stream
    {

        UrlBase::UrlBase(
            boost::asio::io_service & io_svc)
            : io_svc_(io_svc)
        {
        }

        UrlBase::~UrlBase()
        {
        }

        bool UrlBase::open(
            framework::string::Url const & url,
            boost::system::error_code & ec)
        {
            return open(url, 0, boost::uint64_t(-1), ec);
        }

        void UrlBase::async_open(
            framework::string::Url const & url,
            boost::uint64_t beg, 
            boost::uint64_t end, 
            response_type const & resp)
        {
            boost::system::error_code ec;
            open(url, beg, end, ec);
            io_svc_.post(boost::bind(resp, ec));
        }

        void UrlBase::async_open(
            framework::string::Url const & url,
            response_type const & resp)
        {
            async_open(url, 0, boost::uint64_t(-1), resp);
        }

        boost::uint64_t UrlBase::total(
            boost::system::error_code & ec)
        {
            ec = framework::system::logic_error::no_data;
            return 0;
        }

        bool UrlBase::recoverable(
            boost::system::error_code const & ec)
        {
            return false;
        }

    } // namespace stream
} // namespace util
