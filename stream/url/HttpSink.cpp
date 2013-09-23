#include "util/Util.h"
#include "util/stream/url/HttpSink.h"

#include <util/protocol/http/HttpSocket.hpp>

#include <framework/logger/Logger.h>
#include <framework/logger/StreamRecord.h>
#include <framework/logger/StringRecord.h>

namespace util
{
    namespace stream
    {

        FRAMEWORK_LOGGER_DECLARE_MODULE_LEVEL("util.stream.HttpSink", framework::logger::Debug);

        HttpSink::HttpSink(
            boost::asio::io_service & io_svc)
            : UrlSink(io_svc)
            , http_(io_svc)
        {
        }

        HttpSink::~HttpSink() {}

        bool HttpSink::open(
            framework::string::Url const & url,
            boost::uint64_t beg, 
            boost::uint64_t end, 
            boost::system::error_code & ec)
        {
            flag_ = true;

            util::protocol::HttpRequest request;
            util::protocol::HttpRequestHead & head = request.head();
            head.method = head.post;
            head.path = url.path_all();
            head["Accept"] = "{*.*}";
            head.host = url.host_svc();
            head.connection = util::protocol::http_field::Connection::keep_alive;
            if (beg != 0 || end != (boost::uint64_t)-1) {
                head.range.reset(util::protocol::http_field::Range((boost::int64_t)beg, (boost::int64_t)end));
            } else {
                head.range.reset();
            }
            std::ostringstream oss;
            head.get_content(oss);
            LOG_STR(framework::logger::Trace, ("http_request_head", oss.str()));

            http_.open(request, ec);

            return !ec;
        }

        void HttpSink::async_open(
            framework::string::Url const & url,
            boost::uint64_t beg, 
            boost::uint64_t end, 
            response_type const & resp)
        {
            flag_ = true;

            boost::system::error_code ec;
            util::protocol::HttpRequest request;
            util::protocol::HttpRequestHead & head = request.head();
            head.method = head.post;
            head.path = url.path_all();
            head["Accept"] = "{*.*}";
            head.host = url.host_svc();
            head.connection = util::protocol::http_field::Connection::keep_alive;
            if (beg != 0 || end != (boost::uint64_t)-1) {
                head.range.reset(util::protocol::http_field::Range((boost::int64_t)beg, (boost::int64_t)end));
            } else {
                head.range.reset();
            }
            std::ostringstream oss;
            head.get_content(oss);
            LOG_STR(framework::logger::Trace, ("http_request_head", oss.str()));

            http_.async_open(request, resp);
        }

        bool HttpSink::is_open(
            boost::system::error_code & ec)
        {
            bool result = http_.is_open(ec);
            if(flag_ && result){
                util::protocol::HttpResponseHead head = http_.response().head();
                std::ostringstream oss;
                head.get_content(oss);
                LOG_STR(framework::logger::Trace, ("http_response_head", oss.str()));
                flag_ = false;
            }
            return result;
        }

        bool HttpSink::cancel(
            boost::system::error_code & ec)
        {
            return !http_.cancel_forever(ec);
        }

        bool HttpSink::close(
            boost::system::error_code & ec)
        {
            return !http_.close(ec);
        }

        std::size_t HttpSink::private_read_some(
            util::stream::StreamMutableBuffers const & buffers, 
            boost::system::error_code & ec)
        {
            assert(http_.is_open(ec));
            return http_.read_some(buffers, ec);
        }

        void HttpSink::private_async_read_some(
            util::stream::StreamMutableBuffers const & buffers, 
            util::stream::StreamHandler const & handler)
        {
            boost::system::error_code ec;
            (void)ec;
            assert(http_.is_open(ec));
            http_.async_write_some(buffers, handler);
        }

        bool HttpSink::set_non_block(
            bool non_block, 
            boost::system::error_code & ec)
        {
            return !http_.set_non_block(non_block, ec);
        }

        bool HttpSink::set_time_out(
            boost::uint32_t time_out, 
            boost::system::error_code & ec)
        {
            return !http_.set_time_out(time_out, ec);
        }

        bool HttpSink::recoverable(
            boost::system::error_code const & ec)
        {
            return util::protocol::HttpClient::recoverable(ec);
        }

    } // namespace stream
} // namespace util
