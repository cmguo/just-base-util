// MmspServer.cpp

#include "util/Util.h"
#include "util/stream/StreamTransfer.h"
#include "util/protocol/mmsp/MmspServer.h"
#include "util/protocol/mmsp/MmspSocket.hpp"
#include "util/protocol/mmsp/MmspError.h"
#include "util/protocol/mmsp/MmspMacToViewerMessage.h"
#include "util/protocol/mmsp/MmspViewerToMacMessage.h"
using namespace util::stream;

#include <framework/logger/Logger.h>
#include <framework/logger/FormatRecord.h>
#include <framework/logger/Section.h>

using namespace boost::system;

namespace util
{
    namespace protocol
    {

        FRAMEWORK_LOGGER_DECLARE_MODULE_LEVEL("util.protocol.MmspServer", framework::logger::Warn);

        static size_t const DATA_BUFFER_SIZE = 10240;

        MmspServer::MmspServer(
            boost::asio::io_service & io_svc)
            : MmspSocket(io_svc)
        {
            static size_t gid = 0;
            id_ = gid++;

            register_message<MmspDataCancelReadBlock>(mc_read);
            register_message<MmspDataCloseFile>(mc_read);
            register_message<MmspDataConnect>(mc_read);
            register_message<MmspDataConnectFunnel>(mc_read);
            register_message<MmspDataFunnelInfo>(mc_read);
            register_message<MmspDataOpenFile>(mc_read);
            register_message<MmspDataPong>(mc_read);
            register_message<MmspDataReadBlock>(mc_read);
            //register_message<MmspDataSecurityResponse>(mc_read);
            register_message<MmspDataStartPlaying>(mc_read);
            register_message<MmspDataStartStriding>(mc_read);
            register_message<MmspDataStopPlaying>(mc_read);
            register_message<MmspDataStreamSwitch>(mc_read);

            register_message<MmspDataPing>(mc_write);
            register_message<MmspDataReportConnectedEx>(mc_write);
            register_message<MmspDataReportConnectedFunnel>(mc_write);
            register_message<MmspDataReportDisconnectedFunnel>(mc_write);
            register_message<MmspDataReportEndOfStream>(mc_write);
            register_message<MmspDataReportFunnelInfo>(mc_write);
            register_message<MmspDataReportOpenFile>(mc_write);
            register_message<MmspDataReportReadBlock>(mc_write);
            register_message<MmspDataReportRedirect>(mc_write);
            //register_message<MmspDataSecurityChallenge>(mc_write);
            register_message<MmspDataReportStartPlaying>(mc_write);
            register_message<MmspDataReportStartStriding>(mc_write);
            register_message<MmspDataReportStreamChange>(mc_write);
            register_message<MmspDataReportStreamSwitch>(mc_write);
        }

        MmspServer::~MmspServer()
        {
            close();
        }

        void MmspServer::start()
        {
            async_read(request_, 
                boost::bind(&MmspServer::handle_receive_request, this, _1, _2));
        }

        void MmspServer::handle_receive_request(
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

            if (request_.header().MID == MmspViewerToMacMessage::PONG) {
                start();
                return;
            }

            response_.reset();
            local_process(
                boost::bind(&MmspServer::handle_local_process, this, _1));
        }

        void MmspServer::handle_local_process(
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

            async_write(response_, 
                boost::bind(&MmspServer::handle_send_response, this, _1, _2));
        }

        void MmspServer::handle_send_response(
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

            on_finish();

            request_.reset();
            response_.reset();

            start();
        }

       void MmspServer::handle_error(
            error_code const & ec)
        {
            LOG_DEBUG("[handle_error] id =%u, ec = %s" % id_ % ec);

            on_error(ec);

            post_process(
                boost::bind(&MmspServer::handle_post_process, this, _1));
        }

       void MmspServer::handle_post_process(
           error_code const & ec)
       {
           LOG_SECTION();

           LOG_DEBUG("[handle_post_process] id = %u, ec = %s" % id_ % ec.message());

           delete this;
       }

    } // namespace protocol
} // namespace util
