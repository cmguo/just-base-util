// RtspServer.cpp

#include "util/Util.h"
#include "util/stream/StreamTransfer.h"
#include "util/protocol/rtsp/RtspServer.h"
#include "util/protocol/rtsp/RtspSocket.hpp"
#include "util/protocol/rtsp/RtspError.h"
#include "util/protocol/rtsp/RtspMessageHelper.h"
#include "util/protocol/mine/MineHead.hpp"
#include "util/protocol/Message.hpp"
using namespace util::stream;

#include <framework/logger/Logger.h>
#include <framework/logger/FormatRecord.h>
#include <framework/logger/Section.h>
#include <framework/system/LogicError.h>
#include <framework/string/Url.h>
using namespace framework::network;
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

        FRAMEWORK_LOGGER_DECLARE_MODULE_LEVEL("util.protocol.RtspServer", framework::logger::Warn);

        static size_t const DATA_BUFFER_SIZE = 10240;

        RtspServer::RtspServer(
            boost::asio::io_service & io_svc)
            : RtspSocket(io_svc)
        {
            static size_t gid = 0;
            id_ = gid++;
        }

        RtspServer::~RtspServer()
        {
            close();
        }

        void RtspServer::start()
        {
            on_start();
            next();
        }

        void RtspServer::next()
        {
            on_next();
            if (!send_msgs_.empty()) {
                error_code ec;
                handle_local_process(ec);
                return;
            }
            async_read_msg(recv_msg_, 
                boost::bind(&RtspServer::handle_receive_message, this, _1, _2));
        }

        void RtspServer::handle_receive_message(
            error_code const & ec, 
            size_t bytes_transferred)
        {
            LOG_SECTION();

            LOG_DEBUG("[handle_receive_message] id = %u, ec = %s, bytes_transferred = %d" 
                % id_ % ec.message() % bytes_transferred);

            if (ec) {
                handle_error(ec);
                return;
            }

            send_msgs_.clear();

            if (recv_msg_.is<RtspResponse>()) {
                error_code ec1;
                local_process_response(recv_msg_.as<RtspResponse>(), ec1);
                recv_msg_.reset();
                handle_local_process(ec1);
                return;
            }

            send_msgs_.push_back(RtspResponse());
            local_process_request(
                boost::bind(&RtspServer::handle_local_process, this, _1));
        }

        void RtspServer::handle_local_process(
            error_code const & ec)
        {
            LOG_SECTION();

            LOG_DEBUG("[handle_local_process] id = %u, ec = %s" % id_ % ec.message());

            if (ec) {
                response_error(ec);
                return;
            }

            if (recv_msg_.is<RtspRequest>()) {
                RtspResponse & response(send_msgs_[0].as<RtspResponse>());
                response.head().err_msg = "OK";
                response.head()["CSeq"] = recv_msg_.as<RtspRequest>().head()["CSeq"];
            }
            for (size_t i = 0; i < send_msgs_.size(); ++i) {
                if (send_msgs_[i].is<RtspResponse>()) {
                    RtspResponse & response(send_msgs_[i].as<RtspResponse>());
                    response.head().content_length.reset(response.data().size());
                } else {
                    RtspRequest & request(send_msgs_[i].as<RtspRequest>());
                    request.head().content_length.reset(request.data().size());
                }
            }

            async_write_msgs(send_msgs_, 
                boost::bind(&RtspServer::handle_send_message, this, _1, _2));
        }

        void RtspServer::handle_send_message(
            error_code const & ec, 
            size_t bytes_transferred)
        {
            LOG_SECTION();

            LOG_DEBUG("[handle_send_message] id = %u, ec = %s, bytes_transferred = %d" 
                % id_ % ec.message() % bytes_transferred);

            if (ec) {
                handle_error(ec);
                return;
            }

            on_finish();

            recv_msg_.reset();
            send_msgs_.clear();

            next();
        }

        void RtspServer::handle_response_error(
            boost::system::error_code const & ec, 
            size_t bytes_transferred)
        {
            LOG_SECTION();

            LOG_DEBUG("[handle_response_error] id = %u, ec = %s, bytes_transferred = %d" 
                % id_ % ec.message() % bytes_transferred);

            delete this;
        }

       void RtspServer::handle_error(
            error_code const & ec)
        {
            LOG_DEBUG("[handle_error] id = %u, ec = %s" % id_ % ec.message());

            on_error(ec);

            post_process(
                boost::bind(&RtspServer::handle_post_process, this, _1));
        }

       void RtspServer::handle_post_process(
           error_code const & ec)
       {
           LOG_SECTION();

           LOG_DEBUG("[handle_post_process] id = %u, ec = %s" % id_ % ec.message());

           delete this;
       }

        void RtspServer::response_error(
            error_code const & ec)
        {
            if (!send_msgs_[0].is<RtspResponse>()) {
                return;
            }
            RtspResponse & response = send_msgs_[0].as<RtspResponse>();
            RtspResponseHead & head = response.head();
            head = RtspResponseHead(); // clear
            if (ec.category() == rtsp_error::get_category()) {
                head.err_code = ec.value();
            } else if (ec.category() == boost::asio::error::get_system_category()
                || ec.category() == boost::asio::error::get_netdb_category()
                || ec.category() == boost::asio::error::get_addrinfo_category()
                || ec.category() == boost::asio::error::get_misc_category()) {
                response.head().err_code = rtsp_error::service_unavailable;
            } else {
                head.err_code = rtsp_error::internal_server_error;
            }
            head.err_msg = ec.message();
            head.content_length.reset(0);
            on_error(ec);
            RtspSocket::async_write_msg(send_msgs_[0], 
                boost::bind(&RtspServer::handle_response_error, this, _1, _2));
        }

    } // namespace protocol
} // namespace util
