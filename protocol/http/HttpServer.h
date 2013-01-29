// HttpServer.h

#ifndef _UTIL_PROTOCOL_HTTP_HTTP_SERVER_H_
#define _UTIL_PROTOCOL_HTTP_HTTP_SERVER_H_

#include "util/protocol/http/HttpSocket.h"
#include "util/protocol/http/HttpRequest.h"
#include "util/protocol/http/HttpResponse.h"

#include <framework/network/ServerManager.h>

#include <boost/function.hpp>

namespace util
{
    namespace protocol
    {

        class HttpServer
            : public HttpSocket
        {
        public:
            HttpServer(
                boost::asio::io_service & io_svc);

            virtual ~HttpServer();

        protected:
            class Size
            {
            public:
                Size() : type_(0) {}
                Size(bool b) : type_(1), s1_(b ? 1 : 0) {}
                Size(int s): type_(2), s1_(s) {}
                Size(size_t s): type_(2), s1_(s) {}
                Size(size_t s1, size_t s2): type_(3), s1_(s1), s2_(s2) {}
                Size(std::pair<size_t, size_t> const & s): type_(3), s1_(s.first), s2_(s.second) {}
            public:
                bool is_bool() const { return type_ == 1; }
                bool is_size_t() const { return type_ == 2; }
                bool is_size_pair() const { return type_ == 3; }
            public:
                bool get_bool() const { assert(type_ == 1); return s1_ == 1; }
                size_t get_size_t() const { assert(type_ == 2); return s1_; }
                std::pair<size_t, size_t> get_size_pair() const 
                { assert(type_ == 3); return std::pair<size_t, size_t>(s1_, s2_); }
                std::string to_string() const;
            private:
                int type_;
                size_t s1_;
                size_t s2_;
            };

            typedef boost::function<void (
                boost::system::error_code const &, 
                Size const &)> response_type;

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

            util::stream::Source & request_stream()
            {
                return source();
            }

            util::stream::Sink & response_stream()
            {
                return sink();
            }

        protected:
            virtual void on_receive_request_head(
                HttpRequestHead & request_head, 
                response_type const & resp)
            {
                on_receive_request_head(request_head);
                resp(boost::system::error_code(), Size());
            }

            virtual void on_receive_request_head(
                HttpRequestHead & request_head)
            {
            }

            virtual void on_receive_request_data(
                boost::asio::streambuf & request_data)
            {
            }

            virtual void local_process(
                response_type const & resp);

            virtual void on_receive_response_head(
                HttpResponseHead & response_head)
            {
                // 设置response_head
            }

            virtual void on_receive_response_data(
                boost::asio::streambuf & response_data)
            {
            }

            virtual void on_error(
                boost::system::error_code const & ec)
            {
            }

            virtual void on_finish()
            {
            }

            // 提醒派生类放弃
            virtual void on_broken_pipe();

            virtual void transfer_request_data(
                response_type const & resp);

            virtual void transfer_response_data(
                response_type const & resp);

        protected:
            enum StateEnum
            {
                stopped, 
                receiving_request_head, 
                preparing, 
                transferring_request_data, 
                local_processing, 
                sending_response_head, 
                transferring_response_data, 
                exiting, 
            };

            StateEnum state() const
            {
                return state_;
            };

        private:
            static std::string const state_str[];

        private:
            void start();

            void handle_watch(
                boost::system::error_code const & ec);

            void handle_async(
                boost::system::error_code const & ec, 
                Size const & bytes_transferred);

            void response_error(
                boost::system::error_code const & ec, 
                response_type const & resp);

        private:
            template <
                typename HttpServer, 
                typename Manager
            >
            friend class framework::network::ServerManager;

            enum WatchStateEnum
            {
                watch_stopped, 
                watching, 
                broken, 
            };

            size_t id_;
            StateEnum state_;
            WatchStateEnum watch_state_;
            HttpRequest request_;
            HttpResponse response_;
            boost::asio::streambuf transfer_buf_;
        };

    } // namespace protocol
} // namespace util

#endif // _UTIL_PROTOCOL_HTTP_HTTP_SERVER_H_
