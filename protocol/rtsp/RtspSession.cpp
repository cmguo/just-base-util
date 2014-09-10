// RtspSession.cpp

#include "util/Util.h"
#include "util/protocol/rtsp/RtspSession.h"
#include "util/protocol/rtsp/RtspSocket.hpp"
#include "util/protocol/rtsp/RtspError.h"
#include "util/protocol/rtsp/RtspMessageHelper.h"
#include "util/protocol/rtsp/RtspPacket.hpp"
#include "util/protocol/Message.hpp"

#include <framework/logger/Logger.h>
#include <framework/logger/FormatRecord.h>
#include <framework/logger/Section.h>
#include <framework/string/Format.h>
#include <framework/timer/TimeHelper.h>

namespace util
{
    namespace protocol
    {

        FRAMEWORK_LOGGER_DECLARE_MODULE_LEVEL("util.protocol.RtspSession", framework::logger::Warn);

        static size_t const DATA_BUFFER_SIZE = 10240;

        RtspSession::RtspSession(
            boost::asio::io_service & io_svc)
            : RtspSocket(io_svc)
            , stopping_(false)
        {
            static size_t gid = 0;
            id_ = gid++;
            seq_ = 0;
        }

        RtspSession::~RtspSession()
        {
            close();
        }

        void RtspSession::start()
        {
            on_start();
            recv_next();
        }

        void RtspSession::post(
            RtspRequest & request)
        {
            std::string date = framework::timer::TimeHelper::utc_time_str("%a, %d %b %Y %H:%M:%S %q");

            request.head()["CSeq"] = framework::string::format(++seq_);
            request.head()["Date"] = date;
            request.head().content_length.reset(request.data().size());

            post(RtspMessage(request));
        }

        void RtspSession::post(
            RtspResponse & response)
        {
            assert(!requests_.empty());
            RtspRequest const & request = requests_.front();

            std::string date = framework::timer::TimeHelper::utc_time_str("%a, %d %b %Y %H:%M:%S %q");

            response.head().err_msg = "OK";
            response.head()["CSeq"] = request.head()["CSeq"];
            response.head()["Date"] = date;
            response.head().content_length.reset(response.data().size());

            requests_.pop_front();

            post(RtspMessage(response));
        }

        void RtspSession::fail(
            boost::system::error_code const & ec)
        {
            LOG_SECTION();

            response_error(ec);
        }

        void RtspSession::stop()
        {
            stopping_ = true;
            release();
        }

        void RtspSession::post(
            RtspMessage const & msg)
        {
            send_msgs_.push_back(msg);
            if (send_msgs_.size() == 1) {
                send_next();
            }
        }

        void RtspSession::recv_next()
        {
            on_next();
            recv_msg_.reset();
            recv_msg_.type = RtspMessageType::DATA;
            async_read_msg(recv_msg_, 
                boost::bind(&RtspSession::handle_receive_message, this, _1, _2));
        }

        void RtspSession::send_next()
        {
            async_write_msg(send_msgs_.front(), 
                boost::bind(&RtspSession::handle_send_message, this, _1, _2));
        }

        void RtspSession::handle_receive_message(
            boost::system::error_code const & ec, 
            size_t bytes_transferred)
        {
            LOG_SECTION();

            LOG_DEBUG("[handle_receive_message] id = %u, ec = %s, bytes_transferred = %d" 
                % id_ % ec.message() % bytes_transferred);

            if (ec) {
                recv_msg_.type = RtspMessageType::NONE;
                handle_error(ec);
                return;
            }

            if (recv_msg_.is<RtspResponse>()) {
                on_recv(recv_msg_.as<RtspResponse>());
            } else {
                requests_.push_back(recv_msg_.as<RtspRequest>());
                on_recv(requests_.back());
            }

            recv_next();
        }

        void RtspSession::handle_send_message(
            boost::system::error_code const & ec, 
            size_t bytes_transferred)
        {
            LOG_SECTION();

            LOG_DEBUG("[handle_send_message] id = %u, ec = %s, bytes_transferred = %d" 
                % id_ % ec.message() % bytes_transferred);

            if (ec) {
                send_msgs_.clear();
                handle_error(ec);
                return;
            }

            RtspMessage & send_msg(send_msgs_.front());
            if (send_msg.is<RtspRequest>()) {
                on_sent(send_msg.as<RtspRequest>());
            } else {
                on_sent(send_msg.as<RtspResponse>());
            }

            send_msgs_.pop_front();
            if (!send_msgs_.empty()) {
                send_next();
            }
        }

        void RtspSession::handle_error(
            boost::system::error_code const & ec)
        {
            LOG_WARN("[handle_error] id = %u, ec = %s" % id_ % ec.message());

            on_error(ec);

            release();
        }

        void RtspSession::release()
        {
            if (stopping_ && recv_msg_.type == RtspMessageType::NONE && send_msgs_.empty()) {
                LOG_DEBUG("[release]");
                delete this;
            }
        }

        void RtspSession::response_error(
            boost::system::error_code const & ec)
        {
            if (requests_.empty()) {
                return;
            }
            RtspResponse response;
            RtspResponseHead & head = response.head();
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
            post(response);
        }

    } // namespace protocol
} // namespace util
