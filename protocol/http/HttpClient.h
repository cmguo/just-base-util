// HttpClient.h

#ifndef _UTIL_PROTOCOL_HTTP_CLIENT_H_
#define _UTIL_PROTOCOL_HTTP_CLIENT_H_

#include "util/protocol/http/HttpSocket.h"
#include "util/protocol/http/HttpRequest.h"
#include "util/protocol/http/HttpResponse.h"
#include "util/protocol/http/HttpError.h"
#include "util/stream/Socket.h"
#include "util/stream/ChunkedSource.h"
#include "util/stream/ChunkedSink.h"
#include "util/stream/FilterSource.h"
#include "util/stream/FilterSink.h"

#include <framework/string/Url.h>

#include <boost/function/function1.hpp>

namespace util
{
    namespace protocol
    {

        class HttpClient
            : public HttpSocket
        {
        public:
            typedef boost::function1<
                void, 
                boost::system::error_code const &> response_type;

            struct Statistics
                : public HttpSocket::Statistics
            {
                void reset()
                {
                    HttpSocket::Statistics::reset();
                    send_pend_time 
                        = request_head_time 
                        = request_data_time 
                        = recv_pend_time 
                        = response_head_time 
                        = response_data_time
                        = (boost::uint32_t)-1;
                }

                boost::uint32_t send_pend_time;
                boost::uint32_t request_head_time;
                boost::uint32_t request_data_time;
                boost::uint32_t recv_pend_time;
                boost::uint32_t response_head_time;
                boost::uint32_t response_data_time;
            };

        public:
            static bool recoverable(
                boost::system::error_code const & ec)
            {
                return ec.category() == util::protocol::http_error::get_category() ? 
                    (ec.value() < util::protocol::http_error::ok || 
                    ec.value() >= util::protocol::http_error::internal_server_error) :
                (ec == boost::asio::error::network_reset || 
                    ec == boost::asio::error::timed_out || 
                    ec == boost::asio::error::network_down || 
                    ec == boost::asio::error::connection_reset || 
                    ec == boost::asio::error::connection_refused || 
                    ec == boost::asio::error::host_not_found || 
                    ec == boost::asio::error::eof || 
                    ec == boost::asio::error::host_unreachable);
            }

        public:
            HttpClient(
                boost::asio::io_service & io_svc);

            ~HttpClient();

        public:
            boost::system::error_code bind_host(
                std::string const & host, 
                boost::system::error_code & ec);

            boost::system::error_code bind_host(
                std::string const & host, 
                std::string const & port, 
                boost::system::error_code & ec);

            boost::system::error_code bind_host(
                framework::network::NetName const & addr, 
                boost::system::error_code & ec);

        public:
            // 发送GET请求
            boost::system::error_code open(
                framework::string::Url const & url, 
                HttpRequestHead::MethodEnum method, 
                boost::system::error_code & ec)
            {
                HttpRequestHead head;
                return open(url_to_head(head, url, method), ec);
            }

            boost::system::error_code open(
                framework::string::Url const & url, 
                boost::system::error_code & ec)
            {
                return open(url, HttpRequestHead::get, ec);
            }

            boost::system::error_code open(
                std::string const & url, 
                boost::system::error_code & ec)
            {
                return open(framework::string::Url(url), ec);
            }

            boost::system::error_code open(
                HttpRequestHead const & head, 
                boost::system::error_code & ec)
            {
                return open(HttpRequest(head), ec);
            }

            boost::system::error_code open(
                HttpRequest const & request, 
                boost::system::error_code & ec);

            boost::system::error_code reopen(
                boost::system::error_code & ec);

            bool is_open(
                boost::system::error_code & ec);

            void async_open(
                framework::string::Url const & url, 
                HttpRequestHead::MethodEnum method, 
                response_type const & resp)
            {
                HttpRequestHead head;
                async_open(url_to_head(head, url, method), resp);
            }

            void async_open(
                framework::string::Url const & url, 
                response_type const & resp)
            {
                async_open(url, HttpRequestHead::get, resp);
            }

            void async_open(
                std::string const & url, 
                response_type const & resp)
            {
                async_open(framework::string::Url(url), resp);
            }

            void async_open(
                HttpRequestHead const & head, 
                response_type const & resp)
            {
                async_open(HttpRequest(head), resp);
            }

            void async_open(
                HttpRequest const & request, 
                response_type const & resp);

            void async_reopen(
                response_type const & resp);

            boost::system::error_code read_finish(
                boost::system::error_code & ec, 
                boost::uint64_t bytes_transferred);

        public:
            void close();

            boost::system::error_code close(
                boost::system::error_code & ec);

        public:
            boost::system::error_code fetch(
                framework::string::Url const & url, 
                HttpRequestHead::MethodEnum method, 
                boost::system::error_code & ec)
            {
                HttpRequestHead head;
                return fetch(url_to_head(head, url, method), ec);
            }

            boost::system::error_code fetch(
                framework::string::Url const & url, 
                boost::system::error_code & ec)
            {
                return fetch(url, HttpRequestHead::get, ec);
            }

            boost::system::error_code fetch(
                std::string const & url, 
                boost::system::error_code & ec)
            {
                return fetch(framework::string::Url(url), ec);
            }

            boost::system::error_code fetch(
                HttpRequestHead const & head, 
                boost::system::error_code & ec)
            {
                return fetch(HttpRequest(head), ec);
            }

            boost::system::error_code fetch(
                HttpRequest const & request, 
                boost::system::error_code & ec);

            boost::system::error_code refetch(
                boost::system::error_code & ec);

            bool is_fetch(
                boost::system::error_code & ec);

            boost::system::error_code fetch_get(
                std::string const & url, 
                boost::system::error_code & ec)
            {
                return fetch(framework::string::Url(url), HttpRequestHead::get, ec);
            }

            boost::system::error_code fetch_get(
                framework::string::Url const & url, 
                boost::system::error_code & ec)
            {
                return fetch(url, HttpRequestHead::get, ec);
            }

            boost::system::error_code fetch_post(
                std::string const & url, 
                boost::system::error_code & ec)
            {
                return fetch(framework::string::Url(url), HttpRequestHead::post, ec);
            }

            boost::system::error_code fetch_post(
                framework::string::Url const & url, 
                boost::system::error_code & ec)
            {
                return fetch(url, HttpRequestHead::post, ec);
            }

            void async_fetch(
                framework::string::Url const & url, 
                HttpRequestHead::MethodEnum method, 
                response_type const & resp)
            {
                HttpRequestHead head;
                return async_fetch(url_to_head(head, url, method), resp);
            }

            void async_fetch(
                framework::string::Url const & url, 
                response_type const & resp)
            {
                async_fetch(url, HttpRequestHead::get, resp);
            }

            void async_fetch(
                std::string const & url, 
                response_type const & resp)
            {
                async_fetch(framework::string::Url(url), resp);
            }

            void async_fetch(
                HttpRequestHead const & head, 
                response_type const & resp)
            {
                return async_fetch(HttpRequest(head), resp);
            }

            void async_fetch(
                HttpRequest const & request, 
                response_type const & resp);

            void async_refetch(
                response_type const & resp);

            void async_fetch_get(
                std::string const & url, 
                response_type const & resp)
            {
                async_fetch(framework::string::Url(url), HttpRequestHead::get, resp);
            }

            void async_fetch_get(
                framework::string::Url const & url, 
                response_type const & resp)
            {
                async_fetch(url, HttpRequestHead::get, resp);
            }

            void async_fetch_post(
                std::string const & url, 
                response_type const & resp)
            {
                async_fetch(framework::string::Url(url), HttpRequestHead::post, resp);
            }

            void async_fetch_post(
                framework::string::Url const & url, 
                response_type const & resp)
            {
                async_fetch(url, HttpRequestHead::post, resp);
            }

        private:
            static HttpRequestHead const & url_to_head(
                HttpRequestHead & head, 
                framework::string::Url const & url, 
                HttpRequestHead::MethodEnum method)
            {
                head.method = method;
                head.path = url.path_all();
                if (!url.host().empty())
                    head.host.reset(url.host_svc());
                return head;
            }

        private:
            enum ConnectionStatusEnum
            {
                closed, 
                connectting, 
                established, 
                ready, // 第一个请求已经打开，并且所有请求已经发出去
                broken, 
            };

            enum RequestStatusEnum
            {
                send_pending, 
                sending_req_head, 
                sending_req_data, 
                recv_pending, 
                recving_resp_head, 
                opened, 
                recving_resp_data, 
                finished, 
            };

        private:
            struct Request
                : HttpRequest
            {
                Request(
                    size_t id, 
                    HttpRequest const & request, 
                    bool is_fetch)
                    : HttpRequest(request)
                    , id(id)
                    , is_fetch(is_fetch)
                    , is_async(false)
                    , status(send_pending)
                {
                }

                Request(
                    size_t id, 
                    HttpRequest const & request, 
                    bool is_fetch, 
                    response_type const & resp)
                    : HttpRequest(request)
                    , id(id)
                    , is_fetch(is_fetch)
                    , is_async(true)
                    , status(send_pending)
                    , resp(resp)
                {
                }

                size_t id;
                bool is_fetch;
                bool is_async;
                RequestStatusEnum status;
                Statistics stat;
                response_type resp;
            };

        public:
            HttpRequest & request()
            {
                return requests_[0];
            }

            HttpResponse & response()
            {
                return response_;
            }

            HttpRequestHead & request_head()
            {
                return request().head();
            }

            HttpResponseHead & response_head()
            {
                return response().head();
            }

            util::stream::Sink & request_stream()
            {
                return filter_sink_;
            }

            util::stream::Source & response_stream()
            {
                return filter_source_;
            }

            boost::asio::streambuf & request_data()
            {
                return request().data();
            }

            boost::asio::streambuf & response_data()
            {
                return response().data();
            }

			Statistics const & stat() const
            {
                return requests_[0].stat;
            }

        private:
            boost::system::error_code post_reqeust(
                HttpRequest const & request, 
                bool is_fetch, 
                response_type const & resp, 
                boost::system::error_code & ec);

            boost::system::error_code post_reqeust(
                HttpRequest const & request, 
                bool is_fetch, 
                boost::system::error_code & ec)
            {
                return post_reqeust(request, is_fetch, response_type(), ec);
            }

            boost::system::error_code resume(
                bool pending, 
                boost::system::error_code & ec);

            boost::system::error_code resume_connect(
                boost::system::error_code & ec);

            boost::system::error_code resume_request(
                bool pending, 
                boost::system::error_code & ec);

            void async_resume();

            void handle_async_connect(
                boost::system::error_code const & ec);

            void handle_async_reqeust(
                bool pending, 
                boost::system::error_code const & ec);

            void response_request(
                Request & request, 
                boost::system::error_code const & ec);

            bool handle_redirect(
                Request & request, 
                boost::system::error_code & ec);

            bool handle_next(
                boost::system::error_code & ec);

            void post_handle_request(
                Request & request, 
                boost::system::error_code & ec);

            void set_request_stream(
                HttpHead const & head);

            void set_response_stream(
                HttpHead const & head);

            void dump(
                char const * function, 
                boost::system::error_code const & ec);

            void dump_request(
                Request const & request, 
                char const * function, 
                boost::system::error_code const & ec);

            void close_socket(
                boost::system::error_code & ec);

        private:
            static std::string const con_status_str[];
            static std::string const req_status_str[];

        private:
            static size_t next_id_;

        private:
            framework::network::NetName addr_;
            framework::network::NetName req_addr_;
            ConnectionStatusEnum status_;
            boost::system::error_code broken_error_;
            bool is_keep_alive_;
            std::deque<Request> requests_;
            HttpResponse response_;
            size_t req_id_;
            size_t num_sent_;   // 已经发出去的请求数

        private:
            size_t id_;

        private:
            util::stream::Socket<HttpSocket> stream_;
            util::stream::ChunkedSource chunked_source_;
            util::stream::ChunkedSink chunked_sink_;
            util::stream::FilterSource filter_source_;
            util::stream::FilterSink filter_sink_;
        };

    } // namespace protocol
} // namespace util

#endif // _UTIL_PROTOCOL_HTTP_CLIENT_H_
