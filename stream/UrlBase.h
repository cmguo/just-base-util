// UrlBase.h

#ifndef _UTIL_STREAM_URL_BASE_H_
#define _UTIL_STREAM_URL_BASE_H_

#include <framework/string/Url.h>

#include <boost/function.hpp>

namespace util
{
    namespace stream
    {

        class UrlBase
        {
        public:
            typedef boost::function<void (
                boost::system::error_code const &)
            > response_type;

        public:
            UrlBase(
                boost::asio::io_service & io_svc);

            virtual ~UrlBase();

        public:
            virtual bool open(
                framework::string::Url const & url,
                boost::uint64_t beg, 
                boost::uint64_t end, 
                boost::system::error_code & ec) = 0;

            virtual bool open(
                framework::string::Url const & url,
                boost::system::error_code & ec);

            virtual void async_open(
                framework::string::Url const & url,
                boost::uint64_t beg, 
                boost::uint64_t end, 
                response_type const & resp);

            virtual void async_open(
                framework::string::Url const & url,
                response_type const & resp);

            virtual bool is_open(
                boost::system::error_code & ec) = 0;

            virtual bool close(
                boost::system::error_code & ec) = 0;

            virtual boost::uint64_t total(
                boost::system::error_code & ec);

        public:
            virtual bool recoverable(
                boost::system::error_code const & ec);

        private:
            boost::asio::io_service & io_svc_;
        };

    } // namespace stream
} // namespace util

#endif // _UTIL_STREAM_URL_BASE_H_
