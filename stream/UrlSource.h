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
        {
        public:
            UrlSource(
                boost::asio::io_service & io_svc);
        };

        struct UrlSourceTraits
            : util::tools::ClassFactoryTraits
        {
            typedef std::string key_type;
            typedef UrlSource * (create_proto)(
                boost::asio::io_service &);

            static boost::system::error_code error_not_found();
        };

        class UrlSourceFactory
            : public util::tools::ClassFactory<UrlSourceTraits>
        {
        public:
            static UrlSource * create(
                boost::asio::io_service & io_svc,
                std::string const & proto, 
                boost::system::error_code & ec);
        };

    } // namespace stream
} // namespace util

#define UTIL_REGISTER_URL_SOURCE(k, c) UTIL_REGISTER_CLASS(util::stream::UrlSourceFactory, k, c)

#endif // _UTIL_STREAM_URL_SOURCE_H_
