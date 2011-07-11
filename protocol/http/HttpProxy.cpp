// HttpProxy.cpp

#include "util/Util.h"
#include "util/stream/StreamTransfer.h"
#include "util/protocol/http/HttpProxy.h"
#include "util/protocol/http/HttpError.h"
using namespace util::stream;

#include <framework/logger/Logger.h>
#include <framework/logger/LoggerFormatRecord.h>
#include <framework/logger/LoggerSection.h>
#include <framework/system/LogicError.h>
#include <framework/string/Url.h>
#include <framework/string/Format.h>
using namespace framework::logger;
using namespace framework::network;
using namespace framework::string;
using namespace framework::system::logic_error;

#include <boost/bind.hpp>
#include <boost/asio/read.hpp>
#include <boost/asio/write.hpp>
#include <boost/asio/completion_condition.hpp>
using namespace boost::system;

namespace util
{
    namespace protocol
    {

        FRAMEWORK_LOGGER_DECLARE_MODULE("HttpProxy");

        static size_t const DATA_BUFFER_SIZE = 10240;

        HttpProxy::HttpProxy(
            boost::asio::io_service & io_svc)
            : http_to_client_(io_svc)
            , http_to_server_(NULL)
        {
        }

        HttpProxy::~HttpProxy()
        {
            close();
            if (http_to_server_) {
                delete http_to_server_;
                http_to_server_ = NULL;
            }
        }

        void HttpProxy::close()
        {
            boost::system::error_code ec;
            http_to_client_.close(ec);
            if (http_to_server_) {
                http_to_server_->close(ec);
            }
        }

        /*
        Local:
            receive_request_head
            prepare
            transfer_request_data
            local_process
            receive_response_head
            send_response_head
            transfer_response_data

        Proxy:
            receive_request_head
            prepare
            connect
            send_request_head
            transfer_request_data
            local_process
            receive_response_head
            send_response_head
            transfer_response_data
        */

        void HttpProxy::start()
        {
            LOG_F(Logger::kLevelDebug, "[start]");

            http_to_client_.async_read(request_.head(), 
                boost::bind(&HttpProxy::handle_receive_request_head, this, _1, _2));
        }

        void HttpProxy::cancel()
        {
            if (http_to_server_)
                http_to_server_->cancel();
            http_to_client_.cancel();
        }

        error_code HttpProxy::cancel(
            error_code & ec)
        {
            if (http_to_server_)
                http_to_server_->cancel(ec);
            http_to_client_.cancel(ec);
            return ec;
        }

        HttpRequest & HttpProxy::get_request()
        {
            return request_;
        }

        HttpResponse & HttpProxy::get_response()
        {
            return response_;
        }

        HttpRequestHead & HttpProxy::get_request_head()
        {
            return request_.head();
        }

        HttpResponseHead & HttpProxy::get_response_head()
        {
            return response_.head();
        }

        HttpSocket & HttpProxy::get_client_data_stream()
        {
            return http_to_client_;
        }

        HttpSocket & HttpProxy::get_server_data_stream()
        {
            assert(http_to_server_);
            if (http_to_server_)
                return *http_to_server_;
            else
                return *(HttpSocket *)NULL;
        }

        bool HttpProxy::is_local()
        {
            return http_to_server_ == NULL;
        }

        void HttpProxy::handle_receive_request_head(
            error_code const & ec, 
            size_t bytes_transferred)
        {
            LOG_SECTION();

            LOG_F(Logger::kLevelDebug, "[handle_receive_request_head] ec = %s, bytes_transferred = %u" 
                % ec.message().c_str() % bytes_transferred);

            if (ec) {
                request_.clear_data();
                error_code ec1;
                bool block = !http_to_client_.get_non_block(ec1);
                if (block)
                    http_to_client_.set_non_block(true, ec1);
                boost::asio::read(http_to_client_, request_.data(), boost::asio::transfer_at_least(4096), ec1);
                if (block)
                    http_to_client_.set_non_block(false, ec1);
                if (request_.data().size() > 4096) {
                    LOG_HEX(Logger::kLevelDebug, 
                        boost::asio::buffer_cast<unsigned char const *>(request_.data().data()), 4096);
                    LOG_STR(Logger::kLevelDebug, (format(request_.data().size() - 4096) + " bytes remain").c_str());
                } else {
                    LOG_HEX(Logger::kLevelDebug, 
                        boost::asio::buffer_cast<unsigned char const *>(request_.data().data()), request_.data().size());
                }
                handle_error(ec);
                return;
            }

            on_receive_request_head(
                request_.head(), 
                boost::bind(&HttpProxy::handle_prepare, this, _1, _2));
        }

        void HttpProxy::handle_prepare(
            boost::system::error_code const & ec, 
            bool proxy)
        {
            LOG_SECTION();

            LOG_F(Logger::kLevelDebug, "[handle_prepare] ec = %s" 
                % ec.message().c_str());

            if (ec) {
                response_error(ec);
                return;
            }
            if (proxy) {
                if (!http_to_server_)
                    http_to_server_ = new HttpSocket(http_to_client_.get_io_service());
                NetName addr;
                addr.svc("http");
                if (request_.head().host.is_initialized())
                    addr.from_string(request_.head().host.get());
                http_to_server_->async_connect(addr, 
                    boost::bind(&HttpProxy::handle_connect_server, this, _1));
            } else {
                response_.head().connection = request_.head().connection;
                transfer_request_data(
                    boost::bind(&HttpProxy::handle_transfer_request_data, this, _1, _2));
            }
        }

        void HttpProxy::handle_connect_server(
            error_code const & ec)
        {
            LOG_SECTION();

            LOG_F(Logger::kLevelDebug, "[handle_connect_server] ec = %s" 
                % ec.message().c_str());

            if (ec) {
                response_error(ec);
                return;
            }

            http_to_server_->async_write(request_.head(), 
                boost::bind(&HttpProxy::handle_send_request_head, this, _1, _2));
        }

        void HttpProxy::handle_send_request_head(
            error_code const & ec, 
            size_t bytes_transferred)
        {
            LOG_SECTION();

            LOG_F(Logger::kLevelDebug, "[handle_send_request_head] ec = %s, bytes_transferred = %u" 
                % ec.message().c_str() % bytes_transferred);

            if (ec) {
                response_error(ec);
                return;
            }
            
            transfer_request_data(
                boost::bind(&HttpProxy::handle_transfer_request_data, this, _1, _2));
        }

        void HttpProxy::transfer_request_data(
            transfer_response_type const & resp)
        {
            size_t content_length = request_.head().content_length.get_value_or(0);
            transfer_buf_.reset();
            if (is_local()) {
                if (content_length) {
                    boost::asio::async_read(
                        http_to_client_, 
                        transfer_buf_, 
                        boost::asio::transfer_at_least(content_length), 
                        boost::bind(&HttpProxy::handle_receive_request_data, this, _1, _2));
                } else {
                    handle_transfer_request_data(boost::system::error_code(), transfer_size(0, 0));
                }
            } else {
                if (content_length) {
                    async_transfer(
                        http_to_client_, 
                        *http_to_server_, 
                        transfer_buf_.prepare(DATA_BUFFER_SIZE), 
                        transfer_at_least(content_length), 
                        resp);
                } else {
                    handle_transfer_request_data(boost::system::error_code(), transfer_size(0, 0));
                }
            }
        }

        void HttpProxy::handle_receive_request_data(
            boost::system::error_code const & ec, 
            size_t bytes_transferred)
        {
            LOG_SECTION();

            LOG_F(Logger::kLevelDebug, "[handle_receive_request_head] ec = %s, bytes_transferred = %u" 
                % ec.message().c_str() % bytes_transferred);

            if (ec) {
                handle_error(ec);
                return;
            }

            on_receive_request_data(transfer_buf_);
            transfer_buf_.consume(transfer_buf_.size());
            handle_transfer_request_data(ec, transfer_size(bytes_transferred, bytes_transferred));
        }

        void HttpProxy::handle_transfer_request_data(
            boost::system::error_code const & ec, 
            transfer_size const & bytes_transferred)
        {
            LOG_SECTION();

            LOG_F(Logger::kLevelDebug, "[handle_transfer_request_data] ec = %s, bytes_transferred = {%u, %u}" 
                % ec.message().c_str() % bytes_transferred.first % bytes_transferred.second);

            if (ec) {
                handle_error(ec);
                return;
            }

            local_process(
                boost::bind(&HttpProxy::handle_local_process, this, _1));
        }

        void HttpProxy::handle_local_process(
            error_code const & ec)
        {
            LOG_SECTION();

            LOG_F(Logger::kLevelDebug, "[handle_local_process] ec = %s" 
                % ec.message().c_str());

            if (ec) {
                response_error(ec);
                return;
            }

            if (is_local()) {
                handle_receive_response_head(ec, 0);
            } else {
                http_to_server_->async_read(response_.head(), 
                    boost::bind(&HttpProxy::handle_receive_response_head, this, _1, _2));
            }
        }

        void HttpProxy::handle_receive_response_head(
            error_code const & ec, 
            size_t bytes_transferred)
        {
            LOG_SECTION();

            LOG_F(Logger::kLevelDebug, "[handle_receive_response_head] ec = %s, bytes_transferred = %u" 
                % ec.message().c_str() % bytes_transferred);

            if (ec) {
                response_.clear_data();
                error_code ec1;
                bool block = !http_to_server_->get_non_block(ec1);
                if (block)
                    http_to_server_->set_non_block(true, ec1);
                boost::asio::read(*http_to_server_, response_.data(), boost::asio::transfer_at_least(4096), ec1);
                if (block)
                    http_to_server_->set_non_block(false, ec1);
                if (response_.data().size() > 4096) {
                    LOG_HEX(Logger::kLevelDebug, 
                        boost::asio::buffer_cast<unsigned char const *>(response_.data().data()), 4096);
                    LOG_STR(Logger::kLevelDebug, (format(response_.data().size() - 4096) + " bytes remain").c_str());
                } else {
                    LOG_HEX(Logger::kLevelDebug, 
                        boost::asio::buffer_cast<unsigned char const *>(response_.data().data()), response_.data().size());
                }
                response_error(ec);
                return;
            }

            on_receive_response_head(response_.head());

            if (!response_.head().content_length.is_initialized()) {
                response_.head().connection.reset(http_filed::Connection::close);
            }

            http_to_client_.async_write(response_.head(), 
                boost::bind(&HttpProxy::handle_send_response_head, this, _1, _2));
        }

        void HttpProxy::handle_send_response_head(
            error_code const & ec, 
            size_t bytes_transferred)
        {
            LOG_SECTION();

            LOG_F(Logger::kLevelDebug, "[handle_send_response_head] ec = %s, bytes_transferred = %u" 
                % ec.message().c_str() % bytes_transferred);

            if (ec) {
                handle_error(ec);
                return;
            }

            transfer_response_data(
                boost::bind(&HttpProxy::handle_transfer_response_data, this, _1, _2));
        }

        void HttpProxy::transfer_response_data(
            transfer_response_type const & resp)
        {
            LOG_SECTION();

            transfer_buf_.reset();
            if (is_local()) {
                on_receive_response_data(transfer_buf_);
                if (transfer_buf_.size()) {
                    boost::asio::async_write(
                        http_to_client_, 
                        transfer_buf_, 
                        boost::bind(&HttpProxy::handle_send_response_data, this, _1, _2));
                } else {
                    handle_transfer_response_data(boost::system::error_code(), transfer_size(0, 0));
                }
            } else {
                size_t content_length = response_.head().content_length.get_value_or(0);
                if (content_length) {
                    async_transfer(
                        *http_to_server_, 
                        http_to_client_, 
                        transfer_buf_.prepare(DATA_BUFFER_SIZE), 
                        transfer_at_least(content_length), 
                        resp);
                } else {
                    async_transfer(
                        *http_to_server_, 
                        http_to_client_, 
                        transfer_buf_.prepare(DATA_BUFFER_SIZE), 
                        resp);
                }
            }
        }

        void HttpProxy::handle_send_response_data(
            boost::system::error_code const & ec, 
            size_t bytes_transferred)
        {
            handle_transfer_response_data(ec, transfer_size(bytes_transferred, bytes_transferred));
        }

        void HttpProxy::handle_transfer_response_data(
            boost::system::error_code const & ec, 
            transfer_size const & bytes_transferred)
        {
            LOG_SECTION();

            LOG_F(Logger::kLevelDebug, "[handle_transfer_response_data] ec = %s, bytes_transferred = {%u, %u}" 
                % ec.message().c_str() % bytes_transferred.first % bytes_transferred.second);

            if (ec) {
                handle_error(ec);
                return;
            }

            on_finish();

            if (!response_.head().connection
                || response_.head().connection.get() == http_filed::Connection::close) {
                    delete this;
            } else {
                start();
            }
        }

        void HttpProxy::handle_response_error(
            boost::system::error_code const & ec, 
            size_t bytes_transferred)
        {
            LOG_SECTION();

            LOG_F(Logger::kLevelDebug, "[handle_response_error] ec = %s" 
                % ec.message().c_str());

            on_finish();
            delete this;
        }

       void HttpProxy::handle_error(
            error_code const & ec)
        {
            LOG_F(Logger::kLevelDebug, "[handle_error] ec = %s" 
                % ec.message().c_str());

            on_error(ec);
            on_finish();
            delete this;
        }

        void HttpProxy::response_error(
            error_code const & ec)
        {
            HttpResponseHead & head = response_.head();
            head = HttpResponseHead(); // clear
            if (ec.category() == http_error::get_category()) {
                head.err_code = ec.value();
            } else if (ec.category() == boost::asio::error::get_system_category()
                || ec.category() == boost::asio::error::get_netdb_category()
                || ec.category() == boost::asio::error::get_addrinfo_category()
                || ec.category() == boost::asio::error::get_misc_category()) {
                response_.head().err_code = http_error::service_unavailable;
            } else {
                head.err_code = http_error::internal_server_error;
            }
            head.err_msg = ec.message();
            head.content_length.reset(0);
            response_.head().connection = http_filed::Connection::close;
            on_error(ec);
            http_to_client_.async_write(response_.head(), 
                boost::bind(&HttpProxy::handle_response_error, this, _1, _2));
        }

    } // namespace protocol
} // namespace util
