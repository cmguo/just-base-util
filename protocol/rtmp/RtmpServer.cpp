// RtmpServer.cpp

#include "util/Util.h"
#include "util/protocol/rtmp/RtmpServer.h"
#include "util/protocol/rtmp/RtmpMessage.hpp"
#include "util/protocol/rtmp/RtmpSocket.hpp"
#include "util/protocol/rtmp/RtmpError.h"

#include <framework/logger/Logger.h>
#include <framework/logger/FormatRecord.h>
#include <framework/logger/Section.h>

#include <boost/bind.hpp>
using namespace boost::system;

namespace util
{
    namespace protocol
    {

        FRAMEWORK_LOGGER_DECLARE_MODULE_LEVEL("util.protocol.RtmpServer", framework::logger::Warn);

        static size_t const DATA_BUFFER_SIZE = 10240;

        RtmpServer::RtmpServer(
            boost::asio::io_service & io_svc)
            : RtmpSocket(io_svc)
        {
            static size_t gid = 0;
            id_ = gid++;
        }

        RtmpServer::~RtmpServer()
        {
        }

        void RtmpServer::start()
        {
            async_read_msg(request_, 
                boost::bind(&RtmpServer::handle_receive_request, this, _1, _2));
        }

        static void delete_msg(
            RtmpMessage * msg)
        {
            delete msg;
        }

        void RtmpServer::handle_receive_request(
            error_code const & ec, 
            size_t bytes_transferred)
        {
            LOG_SECTION();

            LOG_DEBUG("[handle_receive_request_head] id =%u, ec = %s, bytes_transferred = %d" 
                % id_ % ec % bytes_transferred);

            if (ec) {
                handle_error(ec);
                return;
            }

            response_.clear();

            if (process_protocol_message(request_, response_)) {
                request_.reset();
                handle_local_process(ec);
                return;
            }

            // pre process
            if (!window_send_) {
                RtmpMessage msg;
                msg.set(RtmpProtocolControlMessageWindowAcknowledgementSize(250000));
                response_.push_back(msg);
                msg.set(RtmpProtocolControlMessageSetPeerBandwidth(250000, 2)); //Dynamic
                response_.push_back(msg);
                window_send_ = true;
            }

            local_process(
                boost::bind(&RtmpServer::handle_local_process, this, _1));
        }

        void RtmpServer::handle_local_process(
            error_code const & ec)
        {
            LOG_SECTION();

            LOG_DEBUG("[handle_local_process] id =%u, ec = %s" % id_ % ec);

            if (ec) {
                handle_error(ec);
                return;
            }

            if (response_.empty()) {
                handle_send_response(ec, 0);
                return;
            }

            async_write_msgs(response_, 
                boost::bind(&RtmpServer::handle_send_response, this, _1, _2));
        }

        void RtmpServer::handle_send_response(
            error_code const & ec, 
            size_t bytes_transferred)
        {
            LOG_SECTION();

            LOG_DEBUG("[handle_send_response] id =%u, ec = %s, bytes_transferred = %d" 
                % id_ % ec % bytes_transferred);

            if (ec) {
                handle_error(ec);
                return;
            }

            if (!request_.empty()) {
                on_finish();
                request_.reset();
            }

            response_.clear();

            start();
        }

       void RtmpServer::handle_error(
            error_code const & ec)
        {
            LOG_DEBUG("[handle_error] id =%u, ec = %s" % id_ % ec);

            on_error(ec);

            post_process(
                boost::bind(&RtmpServer::handle_post_process, this, _1));
        }

       void RtmpServer::handle_post_process(
           error_code const & ec)
       {
           LOG_SECTION();

           LOG_DEBUG("[handle_post_process] id = %u, ec = %s" % id_ % ec.message());

           delete this;
       }

    } // namespace protocol
} // namespace util
