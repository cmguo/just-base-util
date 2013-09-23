// UrlSource.h

#ifndef _UTIL_STREAM_URL_SOURCE_H_
#define _UTIL_STREAM_URL_SOURCE_H_

#include "util/stream/Source.h"
#include "util/stream/UrlBase.h"
#include "util/tools/ClassFactory.h"

namespace util
{
    namespace stream
    {

        class UrlSource
            : public Source
            , public UrlBase
            , public util::tools::ClassFactory<
                UrlSource, 
                std::string, 
                UrlSource * (boost::asio::io_service &)
            >
        {
        public:
            typedef boost::function<void (
                boost::system::error_code const &)
            > response_type;

        public:
            static boost::system::error_code error_not_found();

            static UrlSource * create(
                boost::asio::io_service & io_svc,
                std::string const & proto, 
                boost::system::error_code & ec);

        public:
            UrlSource(
                boost::asio::io_service & io_svc);
        };

    } // namespace stream
} // namespace util

#define UTIL_REGISTER_URL_SOURCE(k, c) UTIL_REGISTER_CLASS(k, c)

#endif // _UTIL_STREAM_URL_SOURCE_H_
