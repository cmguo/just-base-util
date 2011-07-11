// HttpClient.cpp

#include "util/Util.h"
#include "util/stream/StreamTransfer.h"
#include "util/stream/StlStream.h"
#include "util/protocol/http/HttpClient.h"
#include "util/protocol/http/HttpError.h"
#include "util/protocol/http/HttpRequest.h"
#include "util/protocol/http/HttpResponse.h"
using namespace util::stream;
using namespace util::protocol::http_error;

#include <framework/logger/Logger.h>
#include <framework/system/LogicError.h>
#include <framework/logger/LoggerFormatRecord.h>
#include <framework/logger/LoggerSection.h>
#include <framework/string/Url.h>
#include <framework/network/NetName.h>
using namespace framework::logger;
using namespace framework::string;
using namespace framework::network;
using namespace framework::system::logic_error;

#include <boost/bind.hpp>
#include <boost/asio/read.hpp>
#include <boost/asio/write.hpp>
using namespace boost::system;
using namespace boost::asio;

namespace util
{
    namespace protocol
    {

        FRAMEWORK_LOGGER_DECLARE_MODULE("HttpClient");

        static char const DEFAULT_PORT[] = "80";

        std::string const HttpClient::status_str[] = {
            "closed", 
            "connectting", 
            "established", 
            "sending_req_head", 
            "sending_req_data", 
            "recving_resp_head", 
            "opened", 
            "recving_resp_data", 
            "closing"
        };

        HttpClient::HttpClient(
            boost::asio::io_service & io_svc)
            : HttpSocket(io_svc)
            , status_(closed)
            , is_async_(false)
            , is_fetch_(false)
        {
            static size_t gid = 0;
            id_ = gid++;
            addr_.svc(DEFAULT_PORT);
        }

        HttpClient::~HttpClient()
        {
            error_code ec;
            close(ec);
            assert(status_ == closed);
        }

        error_code HttpClient::bind_host(
            std::string const & host, 
            error_code & ec)
        {
            NetName addr;
            addr.svc(DEFAULT_PORT);
            addr.from_string(host);
            return bind_host(addr, ec);
        }

        error_code HttpClient::bind_host(
            std::string const & host, 
            std::string const & svc, 
            error_code & ec)
        {
            return bind_host(NetName(host, svc), ec);
        }

        error_code HttpClient::bind_host(
            NetName const & addr, 
            error_code & ec)
        {
            if (status_ != closed) {
                return ec = already_open;
            }
            if (addr.host().empty()) {
                return ec = invalid_argument;
            }
            addr_ = addr;
            ec = succeed;
            return ec;
        }

        HttpRequest & HttpClient::get_request()
        {
            return request_;
        }

        HttpResponse & HttpClient::get_response()
        {
            return response_;
        }

        HttpRequestHead & HttpClient::get_request_head()
        {
            return request_.head();
        }

        HttpResponseHead & HttpClient::get_response_head()
        {
            return response_.head();
        }

        error_code HttpClient::open(
            Url const & url, 
            HttpRequestHead::MethodEnum method, 
            error_code & ec)
        {
            LOG_F(Logger::kLevelDebug, "[open] (id = %u, addr = %s, url = %s)" 
                % id_ % addr_.to_string() % url.path_all());

            if (status_ != closed && status_ != established) {
                ec = busy_work;
                return ec;
            }
            HttpRequestHead & head = request_.head();
            head.method = method;
            head.path = url.path_all();
            head.host.reset(url.host() + ":" + url.svc_or("80"));
            if (request_.data().size())
                request_.head().content_length.reset(request_.data().size());
            return resume(ec);
        }

        error_code HttpClient::open(
            HttpRequestHead const & head, 
            error_code & ec)
        {
            LOG_F(Logger::kLevelDebug, "[open] (id = %u, addr = %s, url = %s)" 
                % id_ % addr_.to_string() % head.path);

            if (status_ != closed && status_ != established) {
                ec = busy_work;
                return ec;
            }
            request_.head() = head;
            if (request_.data().size())
                request_.head().content_length.reset(request_.data().size());
            return resume(ec);
        }

        error_code HttpClient::open(
            HttpRequest const & request, 
            error_code & ec)
        {
            LOG_F(Logger::kLevelDebug, "[open] (id = %u, addr = %s, url = %s)" 
                % id_ % addr_.to_string() % request.head().path.c_str());

            if (status_ != closed && status_ != established) {
                ec = busy_work;
                return ec;
            }
            request_ = request;
            if (request_.data().size())
                request_.head().content_length.reset(request_.data().size());
            return resume(ec);
        }

        error_code HttpClient::reopen(
            NetName const & addr, 
            HttpRequest const & request, 
            error_code & ec)
        {
            LOG_F(Logger::kLevelDebug, "[reopen] (id = %u, addr = %s, url = %s)" 
                % id_ % addr.to_string() % request.head().path);

            status_ = closed;
            addr_ = addr;
            request_ = request;
            // 在connect的时候会自动关闭
            return resume(ec);
        }

        error_code HttpClient::reopen(
            error_code & ec)
        {
            LOG_F(Logger::kLevelDebug, "[reopen] (id = %u, addr = %s, url = %s)" 
                % id_ % addr_.to_string() % request_.head().path);

            if (status_ != established)
                status_ = closed;
            // 在connect的时候会自动关闭
            return resume(ec);
        }

        error_code HttpClient::reopen(
            boost::uint64_t offset, 
            error_code & ec)
        {
            LOG_F(Logger::kLevelDebug, "[reopen] (id = %u, addr = %s, url = %s, offset = %lu)" 
                % id_ % addr_.to_string() % request_.head().path % (long unsigned int)offset);

            if (status_ != established)
                status_ = closed;
            request_.head().range.reset(http_filed::Range(offset));
            // 在connect的时候会自动关闭
            return resume(ec);
        }

        error_code HttpClient::poll(
            error_code & ec)
        {
            if (status_ == closed) {
                ec = not_open;
            } else if (status_ == connectting) {
                send(null_buffers(), 0, ec);
            } else if (status_ == established 
                || status_ == sending_req_head 
                || status_ == sending_req_data) {
                    send(null_buffers(), 0, ec);
            } else if (status_ == recving_resp_head 
                || status_ == opened 
                || status_ == recving_resp_data) {
                    receive(null_buffers(), 0, ec);
            } else if (status_ == closing) {
                ec = not_open;
            }
            return ec;
        }

        bool HttpClient::is_open(
            error_code & ec)
        {
            if (status_ == opened) {
                ec = error_code();
                return true;
            } else if (status_ == closed) {
                ec = not_open;
                return false;
            }
            resume(ec);
            return !ec;
        }

        void HttpClient::async_open(
            Url const & url, 
            HttpRequestHead::MethodEnum method, 
            response_type const & resp)
        {
            LOG_F(Logger::kLevelDebug, "[async_open] (id = %u, addr = %s, url = %s)" 
                % id_ % addr_.to_string() % url.path_all());

            if (status_ != closed && status_ != established) {
                get_io_service().post(
                    boost::bind(resp, busy_work));
                return;
            }
            HttpRequestHead & head = request_.head();
            head.method = method;
            head.path = url.path_all();
            head.host.reset(url.host() + ":" + url.svc_or("80"));
            if (request_.data().size())
                request_.head().content_length.reset(request_.data().size());
            async_start(resp);
        }

        void HttpClient::async_open(
            HttpRequestHead const & head, 
            response_type const & resp)
        {
            LOG_F(Logger::kLevelDebug, "[async_open] (id = %u, addr = %s, url = %s)" 
                % id_ % addr_.to_string() % head.path);

            if (status_ != closed && status_ != established) {
                get_io_service().post(
                    boost::bind(resp, busy_work));
                return;
            }
            request_.head() = head;
            if (request_.data().size())
                request_.head().content_length.reset(request_.data().size());
            async_start(resp);
        }

        void HttpClient::async_open(
            HttpRequest const & request, 
            response_type const & resp)
        {
            LOG_F(Logger::kLevelDebug, "[async_open] (id = %u, addr = %s, url = %s)" 
                % id_ % addr_.to_string() % request.head().path);

            if (status_ != closed && status_ != established) {
                get_io_service().post(
                    boost::bind(resp, busy_work));
                return;
            }
            request_ = request;
            if (request_.data().size())
                request_.head().content_length.reset(request_.data().size());
            async_start(resp);
        }

        error_code HttpClient::read_finish(
            error_code & ec, 
            boost::uint64_t bytes_transferred)
        {
            LOG_F(Logger::kLevelDebug1, "[read_finish] close (id = %u, status = %s, ec = %s)" 
                % id_ % status_str[status_] % ec.message());

            assert(status_ == opened);
            if (!ec && status_ == opened) {
                stat_.response_data_time = stat_.elapse();
                status_ = established;
            }
            if (status_ == established 
                && response_.head().connection
                && response_.head().connection.get() == http_filed::Connection::close) {
                    LOG_F(Logger::kLevelDebug, "[read_finish] close (id = %u, status = %s)" 
                        % id_ % status_str[status_]);
                    close(ec);
            } else if (ec && ec != boost::asio::error::would_block) {
                error_code ec2;
                LOG_F(Logger::kLevelDebug, "[read_finish] close (id = %u, status = %s)" 
                    % id_ % status_str[status_]);
                close(ec2);
            }
            return ec;
        }

        void HttpClient::close()
        {
            if (is_async_) {
                HttpSocket::cancel();
            } else {
                HttpSocket::close();
                status_ = closed;
            }
        }

        error_code HttpClient::close(
            error_code & ec)
        {
            if (is_async_) {
                HttpSocket::cancel(ec);
            } else {
                HttpSocket::close(ec);
                status_ = closed;
            }
            return ec;
        }

        error_code HttpClient::fetch(
            framework::string::Url const & url, 
            HttpRequestHead::MethodEnum method, 
            boost::system::error_code & ec)
        {
            is_fetch_ = true;
            return open(url, method, ec);
        }

        error_code HttpClient::fetch(
            HttpRequestHead const & head, 
            boost::system::error_code & ec)
        {
            is_fetch_ = true;
            return open(head, ec);
        }

        error_code HttpClient::fetch(
            HttpRequest const & request, 
            boost::system::error_code & ec)
        {
            is_fetch_ = true;
            return open(request, ec);
        }

        error_code HttpClient::refetch(
            boost::system::error_code & ec)
        {
            is_fetch_ = true;
            if (status_ != established)
                status_ = closed;
            return resume(ec);
        }

        void HttpClient::async_fetch(
            framework::string::Url const & url, 
            HttpRequestHead::MethodEnum method, 
            response_type const & resp)
        {
            is_fetch_ = true;
            return async_open(url, method, resp);
        }

        void HttpClient::async_fetch(
            HttpRequestHead const & head, 
            response_type const & resp)
        {
            is_fetch_ = true;
            return async_open(head, resp);
        }

        void HttpClient::async_fetch(
            HttpRequest const & request, 
            response_type const & resp)
        {
            is_fetch_ = true;
            return async_open(request, resp);
        }

        void HttpClient::async_refetch(
            response_type const & resp)
        {
            is_fetch_ = true;
            if (status_ != established)
                status_ = closed;
            async_start(resp);
        }

        error_code HttpClient::resume(
            error_code & ec)
        {
            LOG_F(Logger::kLevelDebug1, "[resume] (id = %u, status = %s)" 
                % id_ % status_str[status_]);

            bool from_established = status_ == established;

            switch (status_) {
                case closed:
                    stat_.reset();
                    LOG_F(Logger::kLevelDebug1, "[resume] connect... (id = %u, status = %s)" 
                        % id_ % status_str[status_]);
                    if (!addr_.host().empty()) {
                        connect(addr_, ec);
                    } else if (request_.head().host.is_initialized()) {
                        connect(NetName(request_.head().host.get()), ec);
                    } else {
                        ec = not_bind;
                    }
                    if (!ec) {
                        (HttpSocket::Statistics &)stat_ = HttpSocket::stat();
                        status_ = established;
                    } else if (ec == boost::asio::error::in_progress) {
                        ec = boost::asio::error::would_block;
                        status_ = connectting;
                        break;
                    } else if (ec == boost::asio::error::would_block) {
                        status_ = connectting;
                        break;
                    } else {
                        break;
                    }
                case connectting:
                    if (status_ == connectting) {
                        LOG_F(Logger::kLevelDebug1, "[resume] connect continue... (id = %u, status = %s)" 
                            % id_ % status_str[status_]);
                        if (connect(addr_, ec)) { // 这里的addr_没有意义
                            if (ec == boost::asio::error::in_progress) {
                                ec = boost::asio::error::would_block;
                            }
                            break;
                        }
                        (HttpSocket::Statistics &)stat_ = HttpSocket::stat();
                        status_ = established;
                    }
                case established:
                    if (from_established) {
                        stat_.reset();
                        stat_.zero();
                    }
                    if (!request_.head().content_length.is_initialized()) {
                        request_.head().content_length.reset(request_.data().size());
                    }
                    if (!request_.head().connection.is_initialized()) {
                        request_.head().connection.reset(http_filed::Connection());
                    }
                    status_ = sending_req_head;
                case sending_req_head:
                    LOG_F(Logger::kLevelDebug1, "[resume] sending_req_head... (id = %u, status = %s)" 
                        % id_ % status_str[status_]);
                    write(request_.head(), ec);
                    if (ec) {
                        break;
                    }
                    stat_.request_head_time = stat_.elapse();
                    status_ = sending_req_data;
                case sending_req_data:
                    if (request_.data().size()) {
                        LOG_F(Logger::kLevelDebug1, "[resume] sending_req_data... (id = %u, status = %s)" 
                            % id_ % status_str[status_]);
                        boost::asio::write(*this, request_.data(), boost::asio::transfer_all(), ec);
                        if (ec) {
                            break;
                        }
                    }
                    stat_.request_data_time = stat_.elapse();
                    status_ = recving_resp_head;
                case recving_resp_head:
                    LOG_F(Logger::kLevelDebug1, "[resume] recving_resp_head... (id = %u, status = %s)" 
                        % id_ % status_str[status_]);
                    read(response_.head(), ec);
                    if (ec) {
                        if (ec != boost::asio::error::would_block) {
                            // 临时用上response_.data()，把已有的数据拿过来看看
                            error_code ec1;
                            response_.clear_data();
                            bool block = !get_non_block(ec1);
                            if (block)
                                set_non_block(true, ec1);
                            boost::asio::read(*this, response_.data(), boost::asio::transfer_at_least(4096), ec1);
                            if (block)
                                set_non_block(false, ec1);
                            if (response_.data().size() > 4096) {
                                LOG_HEX(Logger::kLevelDebug, 
                                    boost::asio::buffer_cast<unsigned char const *>(response_.data().data()), 4096);
                                LOG_STR(Logger::kLevelDebug, (format(response_.data().size() - 4096) + " bytes remain").c_str());
                            } else {
                                LOG_HEX(Logger::kLevelDebug, 
                                    boost::asio::buffer_cast<unsigned char const *>(response_.data().data()), response_.data().size());
                            }
                        }
                        break;
                    }
                    stat_.response_head_time = stat_.elapse();
                    if (handle_redirect(ec)) {
                        error_code ec2;
                        close(ec2);
                        return resume(ec);
                    }
                    if (is_fetch_) {
                        response_.clear_data();
                        status_ = recving_resp_data;
                    } else {
                        status_ = opened;
                        break;
                    }
                case opened:
                    if (is_fetch_) {
                    } else {
                        ec = already_open;
                        break;
                    }
                case recving_resp_data:
                    if (is_fetch_) {
                        if (response_.head().content_length.is_initialized()) {
                            if (response_.head().content_length.get() > 0) {
                                    LOG_F(Logger::kLevelDebug1, "[resume] recving_resp_head... (id = %u, status = %s)" 
                                        % id_ % status_str[status_]);
                                boost::asio::read(*this, response_.data(), 
                                    boost::asio::transfer_at_least(response_.head().content_length.get() - response_.data().size()), ec);
                            }
                            if (ec) {
                                break;
                            }
                        } else {
                            boost::asio::read(*this, response_.data(), 
                                boost::asio::transfer_all(), ec);
                            if (ec == boost::asio::error::eof) {
                                ec = error_code();
                            } else {
                                break;
                            }
                        }
                        stat_.response_data_time = stat_.elapse();
                        status_ = established;
                    } else {
                        ec = already_open;
                        break;
                    }
                default:
                    break;
            }

            LOG_F(Logger::kLevelDebug1, "[resume] exiting... (id = %u, status = %s, ec = %s)" 
                % id_ % status_str[status_] % ec.message());

            if (ec != boost::asio::error::would_block) {
                post_handle(ec);
                LOG_F(Logger::kLevelDebug, "[resume] finish (id = %u, status = %s, ec = %s)" 
                    % id_ % status_str[status_] % ec.message());
            }

            return ec;
        }

        void HttpClient::async_start(
            response_type const & resp)
        {
            LOG_F(Logger::kLevelDebug1, "[async_start] (id = %u, status = %s)" 
                % id_ % status_str[status_]);

            is_async_ = true;
            resp_ = resp;
            handle_async(error_code());
        }

        void HttpClient::handle_async(
            error_code const & ec)
        {
            LOG_SECTION();

            LOG_F(Logger::kLevelDebug1, "[handle_async] (id = %u, status = %s, ec = %s)" 
                % id_ % status_str[status_] % ec.message());

            if (ec) {
                if (status_ == recving_resp_head) {
                    error_code ec1;
                    response_.clear_data();
                    bool block = !get_non_block(ec1);
                    if (block)
                        set_non_block(true, ec1);
                    boost::asio::read(*this, response_.data(), boost::asio::transfer_at_least(4096), ec1);
                    if (block)
                        set_non_block(false, ec1);
                    if (response_.data().size() > 4096) {
                        LOG_HEX(Logger::kLevelDebug, 
                            boost::asio::buffer_cast<unsigned char const *>(response_.data().data()), 4096);
                        LOG_STR(Logger::kLevelDebug, (format(response_.data().size() - 4096) + " bytes remain").c_str());
                    } else {
                        LOG_HEX(Logger::kLevelDebug, 
                            boost::asio::buffer_cast<unsigned char const *>(response_.data().data()), response_.data().size());
                    }
                }
                if (!is_fetch_ 
                    || ec != boost::asio::error::eof
                    || status_ != recving_resp_data 
                    || response_.head().content_length.is_initialized()) {
                        error_code ec1 = ec;
                        post_handle(ec1);
                        response(ec1);
                        return;
                }
            }

            bool from_established = status_ == established;

            switch (status_) {
                case closed:
                    stat_.reset();
                    status_ = connectting;
                    if (!addr_.host().empty()) {
                        async_connect(addr_, 
                            boost::bind(&HttpClient::handle_async, this, _1));
                    } else if (request_.head().host.is_initialized()) {
                        NetName addr(":80");
                        addr.from_string(request_.head().host.get());
                        async_connect(addr, 
                            boost::bind(&HttpClient::handle_async, this, _1));
                    } else {
                        error_code ec1 = not_bind;
                        post_handle(ec1);
                        response(ec1);
                    }
                    break;
                case connectting:
                    (HttpSocket::Statistics &)stat_ = HttpSocket::stat();
                    status_ = established;
                case established:
                    if (from_established) {
                        stat_.reset();
                        stat_.zero();
                    }
                    status_ = sending_req_head;
                    async_write(request_.head(), 
                        boost::bind(&HttpClient::handle_async, this, _1));
                    break;
                case sending_req_head:
                    stat_.request_head_time = stat_.elapse();
                    status_ = sending_req_data;
                    if (request_.data().size()) {
                        boost::asio::async_write(*this, request_.data(), 
                            boost::bind(&HttpClient::handle_async, this, _1));
                        break;
                    }
                case sending_req_data:
                    stat_.request_data_time = stat_.elapse();
                    status_ = recving_resp_head;
                    async_read(response_.head(), 
                        boost::bind(&HttpClient::handle_async, this, _1));
                    break;
                case recving_resp_head:
                    stat_.response_head_time = stat_.elapse();
                    {
                        error_code ec1;
                        if (handle_redirect(ec1)) {
                            is_async_ = false;
                            error_code ec2;
                            close(ec2);
                            return handle_async(ec1);
                        }
                    }
                    if (is_fetch_) {
                        status_ = recving_resp_data;
                        response_.clear_data();
                        if (response_.head().content_length.is_initialized()) {
                            if (response_.head().content_length.get() > 0) {
                                boost::asio::async_read(*this, response_.data(), 
                                    boost::asio::transfer_at_least(response_.head().content_length.get()), 
                                    boost::bind(&HttpClient::handle_async, this, _1));
                                break;
                            // } else { no break;
                            }
                        } else {
                            boost::asio::async_read(*this, response_.data(), 
                                boost::asio::transfer_all(), 
                                boost::bind(&HttpClient::handle_async, this, _1));
                            break;
                        }
                    } else {
                        status_ = opened;
                        {
                            error_code ec1;
                            post_handle(ec1);
                            response(ec1);
                        }
                        break;
                    }
                case recving_resp_data:
                    stat_.response_data_time = stat_.elapse();
                    assert(is_fetch_);
                    if (is_fetch_) {
                        status_ = established;
                        {
                            error_code ec1;
                            post_handle(ec1);
                            response(ec1);
                        }
                    }
                    break;
                default:
                    break;
            }
        }

        void HttpClient::response(
            error_code const & ec)
        {
            LOG_F(Logger::kLevelDebug, "[response] (id = %u, status = %s, ec = %s)" 
                % id_ % status_str[status_] % ec.message());

            get_io_service().post(
                boost::bind(resp_, ec));

            resp_ = response_type();
        }

        bool HttpClient::handle_redirect(
            error_code & ec)
        {
            size_t const redirect_codes[] = {
                http_error::moved_permanently, 
                http_error::moved_temporarily, 
                http_error::see_other,
                http_error::temporary_redirect, 
            };
            size_t const * iter = std::find(redirect_codes, 
                redirect_codes + sizeof(redirect_codes) / sizeof(redirect_codes[0]), 
                response_.head().err_code);
            if (iter == redirect_codes + sizeof(redirect_codes) / sizeof(redirect_codes[0])) {
                return false;
            }
            if (!response_.head().location.is_initialized()) {
                ec = format_error;
                return false;
            }
            Url location(response_.head().location.get());
            if (!location.host().empty()) {
                request_.head().host.reset(location.host_svc());
            }
            if (!location.path().empty()) {
                request_.head().path = location.path_all();
            }
            return true;
        }

        void HttpClient::post_handle(
            error_code & ec)
        {
            is_async_ = false;
            is_fetch_ = false;

            if (status_ == established 
                && response_.head().connection
                && response_.head().connection.get() == http_filed::Connection::close) {
                    close(ec);
                    LOG_F(Logger::kLevelDebug, "[post_handle] connection closed (id = %u, status = %s, ec = %s)" 
                        % id_ % status_str[status_] % ec.message());
            } else if (ec) {
                stat_.last_error = ec;
                switch (status_) {
                    case closed:
                    case connectting:
                        (HttpSocket::Statistics &)stat_ = HttpSocket::stat();
                        stat_.last_error = ec;
                        break;
                    case sending_req_head:
                        stat_.request_head_time = stat_.elapse();
                        break;
                    case sending_req_data:
                        stat_.request_data_time = stat_.elapse();
                        break;
                    case recving_resp_head:
                        stat_.response_head_time = stat_.elapse();
                        break;
                    case recving_resp_data:
                        stat_.response_data_time = stat_.elapse();
                        break;
                    default:
                        assert(0);
                        break;
                }
                error_code ec2;
                close(ec2);
            }

            if (!ec && (response_.head().err_code < ok 
                || response_.head().err_code >= multiple_choices)) {
                    ec = http_error::errors(response_.head().err_code);
                    LOG_F(Logger::kLevelDebug, "[post_handle] http error (id = %u, status = %s, ec = %s)" 
                        % id_ % status_str[status_] % ec.message());
            }
        }

    } // namespace protocol
} // namespace util
