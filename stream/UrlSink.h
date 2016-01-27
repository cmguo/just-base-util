// UrlSink.h

#ifndef _UTIL_STREAM_URL_SINK_H_
#define _UTIL_STREAM_URL_SINK_H_

#include "util/stream/Sink.h"
#include "util/stream/UrlBase.h"
#include "util/tools/ClassFactory.h"

namespace util
{
    namespace stream
    {

        class UrlSink
            : public Sink
            , public UrlBase
        {
        public:
            UrlSink(
                boost::asio::io_service & io_svc);
        };

        struct UrlSinkTraits
            : util::tools::ClassFactoryTraits
        {
            typedef std::string key_type;
            typedef UrlSink * (create_proto)(
                boost::asio::io_service &);

            static boost::system::error_code error_not_found();
        };

        class UrlSinkFactory
            : public util::tools::ClassFactory<UrlSinkTraits>
        {
        public:
            static UrlSink * create(
                boost::asio::io_service & io_svc,
                std::string const & proto, 
                boost::system::error_code & ec);
        };

    } // namespace stream
} // namespace util

#define UTIL_REGISTER_URL_SINK(k, c) UTIL_REGISTER_CLASS(util::stream::UrlSinkFactory, k, c)

#endif // _UTIL_STREAM_URL_SINK_H_
