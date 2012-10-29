// HttpServer.cpp

#include "util/Util.h"
#include "util/protocol/http/HttpServer.h"
#include "util/protocol/http/HttpError.h"

#include <framework/logger/Logger.h>
#include <framework/logger/FormatRecord.h>
#include <framework/logger/DataRecord.h>
#include <framework/logger/Section.h>
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

        FRAMEWORK_LOGGER_DECLARE_MODULE_LEVEL("util.protocol.HttpServer", Warn);

        std::string const HttpServer::state_str[] = {
            "stopped", 
            "receiving_request_head", 
            "preparing", 
            "transferring_request_data", 
            "local_processing", 
            "sending_response_head", 
            "transferring_response_data", 
            "exiting", 
        };

        std::string HttpServer::Size::to_string() const
        {
            switch (type_) {
                case 0:
                    return "none";
                case 1:
                    return get_bool() ? "true" : "false";
                case 2:
                    return format(s1_);
                case 3:
                    return "{" + format(s1_) + ", " + format(s2_) + "}";
                default:
                    return "ERROR";
            }
        }

        HttpServer::HttpServer(
            boost::asio::io_service & io_svc)
            : HttpSocket(io_svc)
            , state_(stopped)
            , watch_state_(watch_stopped)
        {
            static size_t gid = 0;
            id_ = gid++;
        }

        HttpServer::~HttpServer()
        {
            close();
        }

        void HttpServer::start()
        {
            handle_async(error_code(), Size());
        }

        /*
            receive_request_head
            prepare
            transfer_request_data
            local_process
            receive_response_head
            send_response_head
            transfer_response_data
        */

        void HttpServer::handle_async(
            boost::system::error_code const & ec, 
            Size const & bytes_transferred)
        {
            LOG_SECTION();

            LOG_DEBUG("[handle_async] (id = %u, status = %s, ec = %s, bytes_transferred = %s)" 
                %id_ % state_str[state_] % ec.message() % bytes_transferred.to_string());

            if (watch_state_ == broken) {
                on_finish();
                delete this;
                return;
            }

            if (ec) {
                if (state_ == receiving_request_head) {
                    error_code ec1;
                    response_.clear_data();
                    bool block = !get_non_block(ec1);
                    if (block)
                        set_non_block(true, ec1);
                    boost::asio::read(*this, response_.data(), boost::asio::transfer_at_least(4096), ec1);
                    if (block)
                        set_non_block(false, ec1);
                    LOG_DATA(Debug, ("receiving_request_head", response_.data().data()));
                }
                on_error(ec);
                switch (state_) {
                    case receiving_request_head:
                    case transferring_request_data:
                    case sending_response_head:
                    case transferring_response_data:
                        if (watch_state_ == watching) {
                            error_code ec1;
                            cancel(ec1);
                        }
                        on_finish();
                        state_ = exiting;
                        break;
                    case local_processing:
                        if (!response_.head().content_length.is_initialized() && bytes_transferred.is_size_t()) {
                            response_.head().content_length.reset(bytes_transferred.get_size_t());
                        }
                    case preparing:
                        state_ = sending_response_head;
                        response_error(ec, boost::bind(&HttpServer::handle_async, this, _1, _2));
                        break;
                    default:
                        assert(0);
                        break;
                }
                return;
            }

            switch (state_) {
                case stopped:
                    state_ = receiving_request_head;
                    response_.head() = HttpResponseHead();
                    async_read(request_.head(), 
                        boost::bind(&HttpServer::handle_async, this, _1, _2));
                    break;
                case receiving_request_head:
                    state_ = preparing;
                    if (watch_state_ == watch_stopped 
                        && request_.head().content_length.get_value_or(0) == 0) {
                            watch_state_ = watching;
                            async_read_some(boost::asio::null_buffers(), 
                                boost::bind(&HttpServer::handle_watch, this, _1));
                    }
                    on_receive_request_head(
                        request_.head(), 
                        boost::bind(&HttpServer::handle_async, this, _1, _2));
                    break;
                case preparing:
                    state_ = transferring_request_data;
                    set_source(request_.head());
                    transfer_request_data(
                        boost::bind(&HttpServer::handle_async, this, _1, _2));
                    break;
                case transferring_request_data:
                    on_receive_request_data(transfer_buf_);
                    transfer_buf_.consume(transfer_buf_.size());
                    state_ = local_processing;
                    if (watch_state_ == watch_stopped) {
                        watch_state_ = watching;
                        async_read_some(boost::asio::null_buffers(), 
                            boost::bind(&HttpServer::handle_watch, this, _1));
                    }
                    response_.head().connection = request_.head().connection;
                    local_process(
                        boost::bind(&HttpServer::handle_async, this, _1, _2));
                    break;
                case local_processing:
                    on_receive_response_head(response_.head());
                    if (!response_.head().content_length.is_initialized()) {
                        if (bytes_transferred.is_size_t())
                            response_.head().content_length.reset(bytes_transferred.get_size_t());
                        else
                            response_.head().connection.reset(http_field::Connection());
                    }
                    if (!response_.head().connection.is_initialized()) {
                        response_.head().connection.reset(http_field::Connection());
                    }
                    state_ = sending_response_head;
                    async_write(response_.head(), 
                        boost::bind(&HttpServer::handle_async, this, _1, _2));
                    break;
                case sending_response_head:
                    state_ = transferring_response_data;
                    set_sink(response_.head());
                    transfer_response_data(
                        boost::bind(&HttpServer::handle_async, this, _1, _2));
                    break;
                case transferring_response_data:
                    on_finish();
                    if (!response_.head().connection
                        || response_.head().connection.get() == http_field::Connection::close) {
                            state_ = exiting;
                            if (watch_state_ != watching) {
                                delete this;
                            } else {
                                error_code ec;
                                shutdown(boost::asio::socket_base::shutdown_send, ec);
                                boost::asio::ip::tcp::socket::cancel(ec);
                            }
                    } else {
                        state_ = stopped;
                        if (watch_state_ != watching) {
                            // restart
                            watch_state_ = watch_stopped;
                            handle_async(ec, Size());
                        } else {
                            error_code ec;
                            boost::asio::ip::tcp::socket::cancel(ec);
                        }
                    }
                    break;
                default:
                    assert(0);
                    break;
            }
        }

        void HttpServer::handle_watch(
            boost::system::error_code const & ec)
        {
            LOG_SECTION();

            LOG_DEBUG("[handle_watch] (id = %u, status = %s, ec = %s)" 
                %id_ % state_str[state_] % ec.message());

            if (state_ == exiting) {
                delete this;
            } else if (state_ == stopped) {
                // restart
                watch_state_ = watch_stopped;
                start();
            } else if (ec 
                && ec != boost::asio::error::eof
                && ec != boost::asio::error::operation_aborted) {
                    watch_state_ = broken;
                    on_broken_pipe();
            } else {
                watch_state_ = watch_stopped;
            }
        }

        void HttpServer::local_process(
            response_type const & resp)
        {
            resp(boost::system::error_code(), Size());
        }

        void HttpServer::transfer_request_data(
            response_type const & resp)
        {
            transfer_buf_.reset();
            size_t content_length = request_.head().content_length.get_value_or(0);
            if (content_length) {
                boost::asio::async_read(
                    source(), 
                    transfer_buf_, 
                    boost::asio::transfer_at_least(content_length), 
                    resp);
            } else {
                resp(boost::system::error_code(), Size());
            }
        }

        void HttpServer::transfer_response_data(
            response_type const & resp)
        {
            transfer_buf_.reset();
            on_receive_response_data(transfer_buf_);
            if (transfer_buf_.size()) {
                boost::asio::async_write(sink(), transfer_buf_, resp);
            } else {
                resp(boost::system::error_code(), Size());
            }
        }

        void HttpServer::on_broken_pipe()
        {
            error_code ec;
            on_error(ec);
            cancel(ec);
        }

        void HttpServer::response_error(
            error_code const & ec, 
            response_type const & resp)
        {
            HttpResponseHead & head = response_.head();
            error_code ec1;
            if (ec.category() == http_error::get_category()) {
                ec1 = ec;
            } else if (ec.category() == boost::asio::error::get_system_category()
                || ec.category() == boost::asio::error::get_netdb_category()
                || ec.category() == boost::asio::error::get_addrinfo_category()
                || ec.category() == boost::asio::error::get_misc_category()) {
                ec1 = http_error::service_unavailable;
            } else {
                ec1 = http_error::internal_server_error;
            }
            head.err_code = ec1.value();
            head.err_msg = ec1.message();
            if (!head.content_length.is_initialized())
                head.content_length.reset(0);
            response_.head().connection = http_field::Connection::close;
            async_write(response_.head(), 
                boost::bind(resp, _1, _2));
        }

    } // namespace protocol
} // namespace util
