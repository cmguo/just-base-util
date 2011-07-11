// HttpClient.h

#ifndef _UTIL_PROTOCOL_HTTP_CLIENT_H_
#define _UTIL_PROTOCOL_HTTP_CLIENT_H_

#include "util/protocol/http/HttpSocket.h"
#include "util/protocol/http/HttpRequest.h"
#include "util/protocol/http/HttpResponse.h"
#include "util/protocol/http/HttpError.h"

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
                    request_head_time 
                        = request_data_time 
                        = response_head_time 
                        = response_data_time
                        = (boost::uint32_t)-1;
                }

                boost::uint32_t request_head_time;
                boost::uint32_t request_data_time;
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
            // ∑¢ÀÕGET«Î«Û
            boost::system::error_code open(
                framework::string::Url const & url, 
                HttpRequestHead::MethodEnum method, 
                boost::system::error_code & ec);

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
                boost::system::error_code & ec);

            boost::system::error_code open(
                HttpRequest const & request, 
                boost::system::error_code & ec);

            boost::system::error_code reopen(
                framework::network::NetName const & addr, 
                HttpRequest const & request, 
                boost::system::error_code & ec);

            boost::system::error_code reopen(
                boost::system::error_code & ec);

            boost::system::error_code reopen(
                boost::uint64_t offset, 
                boost::system::error_code & ec);

            boost::system::error_code poll(
                boost::system::error_code & ec);

            bool is_open(
                boost::system::error_code & ec);

            void async_open(
                framework::string::Url const & url, 
                HttpRequestHead::MethodEnum method, 
                response_type const & resp);

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
                response_type const & resp);

            void async_open(
                HttpRequest const & request, 
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
                boost::system::error_code & ec);

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
                boost::system::error_code & ec);

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
                response_type const & resp);

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
                response_type const & resp);

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

        public:
            HttpRequest & get_request();

            HttpResponse & get_response();

            HttpRequestHead & get_request_head();

            HttpResponseHead & get_response_head();

            HttpRequest & request()
            {
                return request_;
            }

            HttpResponse & response()
            {
                return response_;
            }

            HttpRequestHead & request_head()
            {
                return request_.head();
            }

            HttpResponseHead & response_head()
            {
                return response_.head();
            }

			Statistics const & stat() const
			{
				return stat_;
			}

        private:
            enum StatusEnum
            {
                closed, 
                connectting, 
                established, 
                sending_req_head, 
                sending_req_data, 
                recving_resp_head, 
                opened, 
                recving_resp_data, 
                closing
            };

        protected:
            boost::system::error_code resume(
                boost::system::error_code & ec);

            void async_start(
                response_type const & resp);

            void handle_async(
                boost::system::error_code const & ec);

            void response(
                boost::system::error_code const & ec);

            bool handle_redirect(
                boost::system::error_code & ec);

            void post_handle(
                boost::system::error_code & ec);

        private:
            static std::string const status_str[];

        private:
            static size_t next_id_;

        private:
            framework::network::NetName addr_;
            StatusEnum status_;
            bool is_async_;
            bool is_fetch_;
            HttpRequest request_;
            HttpResponse response_;
            response_type resp_;
			Statistics stat_;

        private:
            size_t id_;
        };

    } // namespace protocol
} // namespace util

#endif // _UTIL_PROTOCOL_HTTP_CLIENT_H_
