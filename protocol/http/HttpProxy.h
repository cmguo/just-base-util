// HttpProxy.h

#ifndef _UTIL_PROTOCOL_HTTP_PROXY_H_
#define _UTIL_PROTOCOL_HTTP_PROXY_H_

#include "util/protocol/http/HttpSocket.h"
#include "util/protocol/http/HttpRequest.h"
#include "util/protocol/http/HttpResponse.h"
#include "util/stream/StreamTransfer.h"

#include <framework/network/NetName.h>

#include <boost/function/function1.hpp>
#include <boost/function/function2.hpp>

namespace util
{
    namespace protocol
    {

        class HttpProxy
        {
        public:
            HttpProxy(
                boost::asio::io_service & io_svc);

            virtual ~HttpProxy();

        public:
            void close();

        protected:
            typedef boost::function2<void, 
                boost::system::error_code const &, 
                bool> prepare_response_type;

            typedef boost::function2<void, 
                boost::system::error_code const &, 
                std::pair<std::size_t, std::size_t> const &
            > transfer_response_type;

            typedef boost::function1<void, 
                boost::system::error_code const &
            > local_process_response_type;

        protected:
            HttpRequest & get_request();

            HttpResponse & get_response();

            HttpRequestHead & get_request_head();

            HttpResponseHead & get_response_head();

            HttpSocket & get_client_data_stream();

            HttpSocket & get_server_data_stream();

            bool is_local();

        protected:
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

            boost::asio::streambuf & request_data()
            {
                return request_.data();
            }

            boost::asio::streambuf & response_data()
            {
                return response_.data();
            }

            HttpSocket & client_data_stream()
            {
                return http_to_client_;
            }

            HttpSocket & server_data_stream()
            {
                return *http_to_server_;
            }

        protected:
            virtual void on_receive_request_head(
                HttpRequestHead & request_head, 
                prepare_response_type const & resp)
            {
                // Server模式：返回false
                // Proxy模式：返回true，可以修改request_head
                resp(boost::system::error_code(), 
                    on_receive_request_head(request_head));
            }

            virtual bool on_receive_request_head(
                HttpRequestHead & request_head)
            {
                // Server模式：返回false
                // Proxy模式：返回true，可以修改request_head
                return true;
            }

            virtual void on_receive_request_data(
                boost::asio::streambuf & request_data)
            {
                // Server模式
            }

            virtual void local_process(
                local_process_response_type const & resp)
            {
                resp(boost::system::error_code());
            }

            virtual void on_receive_response_head(
                HttpResponseHead & response_head)
            {
                // Server模式：设置response_head
                // Proxy模式：可以修改response_head
            }

            virtual void on_receive_response_data(
                boost::asio::streambuf & response_data)
            {
                // Server模式
            }

            virtual void on_error(
                boost::system::error_code const & ec)
            {
            }

            virtual void on_finish()
            {
            }

            virtual void transfer_request_data(
                transfer_response_type const & resp);

            virtual void transfer_response_data(
                transfer_response_type const & resp);

        public:
            void cancel();

            boost::system::error_code cancel(
                boost::system::error_code & ec);

        private:
            void start();

        private:
            void handle_prepare(
                boost::system::error_code const & ec, 
                bool proxy);

            void handle_receive_request_head(
                boost::system::error_code const & ec, 
                size_t bytes_transferred);

            void handle_connect_server(
                boost::system::error_code const & ec);

            void handle_send_request_head(
                boost::system::error_code const & ec, 
                size_t bytes_transferred);

            void handle_receive_request_data(
                boost::system::error_code const & ec, 
                size_t bytes_transferred);

            void handle_transfer_request_data(
                boost::system::error_code const & ec, 
                util::stream::transfer_size const & bytes_transferred);

            void handle_local_process(
                boost::system::error_code const & ec);

            void handle_receive_response_head(
                boost::system::error_code const & ec, 
                size_t bytes_transferred);

            void handle_send_response_head(
                boost::system::error_code const & ec, 
                size_t bytes_transferred);

            void handle_send_response_data(
                boost::system::error_code const & ec, 
                size_t bytes_transferred);

            void handle_transfer_response_data(
                boost::system::error_code const & ec, 
                util::stream::transfer_size const & bytes_transferred);

            void handle_response_error(
                boost::system::error_code const & ec, 
                size_t bytes_transferred);

            void handle_error(
                boost::system::error_code const & ec);

            void response_error(
                boost::system::error_code const & ec);

        private:
            template <
                typename HttpProxy, 
                typename Manager
            >
            friend class HttpProxyManager;

            HttpSocket http_to_client_;
            HttpSocket * http_to_server_;
            HttpRequest request_;
            HttpResponse response_;
            boost::asio::streambuf transfer_buf_;
        };

    } // namespace protocol
} // namespace util

#endif // _UTIL_PROTOCOL_HTTP_PROXY_H_
