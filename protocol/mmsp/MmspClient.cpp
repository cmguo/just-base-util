// MmspClient.cpp

#include "util/Util.h"
#include "util/protocol/mmsp/MmspClient.h"
#include "util/protocol/mmsp/MmspSocket.hpp"
#include "util/protocol/mmsp/MmspError.h"
#include "util/protocol/mmsp/MmspMessage.hpp"
#include "util/protocol/mmsp/MmspViewerToMacMessage.h"
#include "util/protocol/mmsp/MmspMacToViewerMessage.h"
using namespace util::protocol::mmsp_error;

#include <framework/system/LogicError.h>
#include <framework/logger/Logger.h>
#include <framework/logger/FormatRecord.h>
#include <framework/logger/DataRecord.h>
#include <framework/logger/Section.h>
#include <framework/string/Url.h>
#include <framework/network/NetName.h>
using namespace framework::logger;
using namespace framework::string;
using namespace framework::network;
using namespace framework::system::logic_error;

#include <boost/bind.hpp>
#include <boost/asio/streambuf.hpp>
#include <boost/asio/read.hpp>
using namespace boost::system;

namespace util
{
    namespace protocol
    {

        FRAMEWORK_LOGGER_DECLARE_MODULE("util.protocol.MmspClient");

        static char const SERVICE_NAME[] = "1755";

        std::string const MmspClient::con_status_str[] = {
            "closed", 
            "connectting", 
            "net_connectting", 
            "established", 
            "broken", 
        };

        std::string const MmspClient::req_status_str[] = {
            "send_pending", 
            "sending_req", 
            "recving_resp", 
            "finished", 
        };

        MmspClient::MmspClient(
            boost::asio::io_service & io_svc)
            : MmspSocket(io_svc)
            , status_(closed)
            , request_status_(finished)
        {
            static size_t gid = 0;
            id_ = gid++;
            addr_.svc(SERVICE_NAME);
        }

        MmspClient::~MmspClient()
        {
            error_code ec;
            if (status_ >= established) {
                status_ = broken;
            }
            MmspSocket::close(ec);
            status_ = closed;
        }

        boost::system::error_code MmspClient::connect(
            framework::string::Url const & url, 
            boost::system::error_code & ec)
        {
            LOG_DEBUG("[connect] (id = %u, url = %s)" 
                % id_ % url.to_string());

            if (requests_.empty()) {
                make_connect_requests(url);
                request_status_ = send_pending;
            }
            resume_connect(ec);
            return ec;
        }

        void MmspClient::async_connect(
            framework::string::Url const & url, 
            response_type const & resp)
        {
            LOG_DEBUG("[async_connect] (id = %u, url = %s)" 
                % id_ % url.to_string());

            resp_ = resp;
            make_connect_requests(url);

            error_code ec;
            handle_async_connect(ec);
        }

        void MmspClient::close()
        {
            error_code ec;
            MmspSocket::close();
            status_ = closed;
            dump("close", ec);
        }

        error_code MmspClient::close(
            error_code & ec)
        {
            MmspSocket::close(ec);
            status_ = closed;
            dump("close", ec);
            return ec;
        }

        boost::system::error_code MmspClient::play(
            boost::system::error_code & ec)
        {
            LOG_DEBUG("[play] (id = %u, url = %s)" % id_);

            if (requests_.empty()) {
                make_play_requests(file_name_);
                request_status_ = send_pending;
            }
            resume_request(ec);
            return ec;
        }

        void MmspClient::async_play(
            response_type const & resp)
        {
            LOG_DEBUG("[async_play] (id = %u, url = %s)" % id_);

            resp_ = resp;
            make_play_requests(file_name_);
            async_reqeust();
        }

        void MmspClient::async_reqeust()
        {
            request_status_ = send_pending;
            handle_async_reqeust(error_code());
        }

        void MmspClient::make_connect_requests(
            framework::string::Url const & url)
        {
            url_ = url;
            file_name_ = url_.path_all().substr(1);
            addr_.from_string(url_.host_svc());

            requests_.resize(2);

            {
                MmspMessage & msg = requests_[0];
                MmspDataConnect & req = msg.get<MmspDataConnect>();
                req.playIncarnation = 0xf0f0f0ef; // MMS_DISABLE_PACKET_PAIR
                req.subscriberName = "NSPlayer/7.0.0.1956; {0xbabac001-0x97eb-0x8607-0xdfd8270fe217a55f}; Host: " + url_.host_svc();
            }

            {
                MmspMessage & msg = requests_[1];
                MmspDataConnectFunnel & req = msg.get<MmspDataConnectFunnel>();
                req.funnelName = "\\\\192.168.0.1\\TCP\\1234";
            }
        }

        void MmspClient::make_play_requests(
            std::string const & content)
        {
            requests_.resize(3);

            {
                MmspMessage & msg = requests_[0];
                MmspDataOpenFile & req = msg.get<MmspDataOpenFile>();
                req.playIncarnation = 1;
                req.fileName = file_name_;
            }

            {
                MmspMessage & msg = requests_[1];
                MmspDataReadBlock & req = msg.get<MmspDataReadBlock>();
                req.openFileId = 1;
                req.playIncarnation = 2;
            }

            {
                MmspMessage & msg = requests_[2];
                MmspDataStartPlaying & req = msg.get<MmspDataStartPlaying>();
                req.openFileId = 1;
                req.playIncarnation = 3;
            }
        }

        void MmspClient::resume_connect(
            boost::system::error_code & ec)
        {
            switch (status_) {
                case closed:
                    status_ = connectting;
                case connectting:
                    if (MmspSocket::connect(addr_, ec))
                        break;
                    status_ = net_connectting;
                case net_connectting:
                    resume_request(ec);
                    if (ec)
                        break;
                    status_ = established;
                    break;
                case established:
                case broken:
                    assert(0);
                    break;
            }
        }

        void MmspClient::resume_request(
            boost::system::error_code & ec)
        {
            while (true) {
                switch (request_status_) {
                case send_pending:
                    request_status_ = sending_req;
                case sending_req:
                    if (write_msg(requests_.front(), ec) == 0)
                        break;
                    request_status_ = recving_resp;
                case recving_resp:
                    if (read_msg(response_, ec) == 0)
                        break;
                    if (process_protocol_message(response_, proto_responses_)) {
                        request_status_ = sending_req;
                        if (write_msg(proto_responses_, ec) == 0)
                            break;
                        request_status_ = recving_resp;
                        break;
                    }
                    if (!post_response(ec)) {
                        request_status_ = recving_resp;
                        break;
                    }
                    request_status_ = finished;
                    requests_.pop_front();
                    if (!requests_.empty()) {
                        request_status_ = send_pending;
                    }
                    break;
                default:
                    assert(false);
                    break;
                }
                if (ec || request_status_ == finished) {
                    break;
                }
            }
        }

        void MmspClient::handle_async_connect(
            error_code const & ec)
        {
            LOG_SECTION();

            dump("handle_async_connect", ec);

            if (ec) {
                status_ = broken;
                response(ec);
                return;
            }

            switch (status_) {
                case closed:
                    status_ = connectting;
                    MmspSocket::async_connect(addr_, 
                        boost::bind(&MmspClient::handle_async_connect, this, _1));
                    break;
                case connectting:
                    status_ = net_connectting;
                    async_reqeust();
                    break;
                case net_connectting:
                    status_ = established;
                    response(ec);
                    break;
                case established:
                case broken:
                    assert(0);
                    break;
            }
        }

        void MmspClient::handle_async_reqeust(
            error_code ec)
        {
            LOG_SECTION();

            dump_request("handle_async_reqeust", ec);

            if (ec) {
                if (request_status_ == recving_resp) {
                    error_code ec1;
                    boost::asio::streambuf buf;
                    bool block = !get_non_block(ec1);
                    if (block)
                        set_non_block(true, ec1);
                    boost::asio::read(*this, buf, boost::asio::transfer_at_least(4096), ec1);
                    if (block)
                        set_non_block(false, ec1);
                    LOG_DATA(Debug, ("recving_resp_head", buf.data()));
                }
                status_ = broken;
                requests_.clear();
                response(ec);
                return;
            }

            switch (request_status_) {
                case send_pending:
                    request_status_ = sending_req;
                    async_write_msg(requests_.front(), 
                        boost::bind(&MmspClient::handle_async_reqeust, this, _1));
                    break;
                case sending_req:
                    request_status_ = recving_resp;
                    async_read_msg(response_, 
                        boost::bind(&MmspClient::handle_async_reqeust, this, _1));
                    break;
                case recving_resp:
                    if (process_protocol_message(response_, proto_responses_)) {
                        request_status_ = sending_req;
                        async_write_msg(proto_responses_, 
                            boost::bind(&MmspClient::handle_async_reqeust, this, _1));
                        break;
                    }
                    if (!post_response(ec)) {
                        request_status_ = sending_req;
                        handle_async_reqeust(ec);
                        break;
                    }
                    request_status_ = finished;
                    requests_.pop_front();
                    if (requests_.empty()) {
                        response(ec);
                    } else {
                        request_status_ = send_pending;
                        handle_async_reqeust(ec);
                    }
                    break;
                default:
                    assert(false);
                    break;
            }
        }

        bool MmspClient::post_response(
            boost::system::error_code & ec)
        {
            boost::uint32_t req_id = requests_.front().id();
            switch (req_id) {
                case MmspViewerToMacMessage::CONNECT:
                    {
                        MmspDataReportConnectedEx const & resp(response_.as<MmspDataReportConnectedEx>());
                        if (resp.hr) {
                            ec = mmsp_error::format_error;
                        }
                    }
                    break;
                case MmspViewerToMacMessage::CONNECT_FUNNEL:
                    {
                        MmspDataReportConnectedFunnel const & resp(response_.as<MmspDataReportConnectedFunnel>());
                        if (resp.hr) {
                            ec = mmsp_error::format_error;
                        }
                    }
                    break;
                case MmspViewerToMacMessage::OPEN_FILE:
                    {
                        MmspDataReportOpenFile const & resp(response_.as<MmspDataReportOpenFile>());
                        if (resp.hr) {
                            ec = mmsp_error::format_error;
                        }
                    }
                    break;
                case MmspViewerToMacMessage::READ_BLOCK:
                    {
                        MmspDataReportReadBlock const & resp(response_.as<MmspDataReportReadBlock>());
                        if (resp.hr) {
                            ec = mmsp_error::format_error;
                        }
                    }
                    break;
                case MmspViewerToMacMessage::START_PLAYING:
                    if (response_.is<MmspDataReportStartPlaying>()) {
                        MmspDataReportStartPlaying const & resp(response_.as<MmspDataReportStartPlaying>());
                        if (resp.hr) {
                            ec = mmsp_error::format_error;
                        }
                    } else {
                        return false;
                    }
                    break;
            }
            return !ec;
        }

        void MmspClient::response(
            boost::system::error_code const & ec)
        {
            response_type tmp;
            tmp.swap(resp_);
            tmp(ec);
        }

        void MmspClient::tick(
            boost::system::error_code & ec)
        {
            if (request_status_ != finished) {
                resume_request(ec);
                if (ec)
                    return;
            }
            if (proto_responses_.empty()) {
                //MmspSocket::tick(proto_responses_);
                if (read_msg(response_, ec)) {
                    process_protocol_message(response_, proto_responses_);
                    response_.reset();
                }
            }
            if (!proto_responses_.empty() && write_msg(proto_responses_, ec) == 0) {
                return;
            }
            proto_responses_.reset();
        }

       void MmspClient::dump(
            char const * function, 
            boost::system::error_code const & ec)
        {
            LOG_TRACE("[%s] (id = %u, status = %s, ec = %s)" 
                % function % id_ % con_status_str[status_] % ec.message());
        }

        void MmspClient::dump_request(
            char const * function, 
            boost::system::error_code const & ec)
        {
            LOG_TRACE("[%s] (id = %u, req_id = %s, req_status = %s, ec = %s)" 
                % function % id_ % requests_.front().id() % req_status_str[request_status_] % ec.message());
        }

        void MmspClient::close_socket(
            error_code & ec)
        {
            MmspSocket::close(ec);
        }

    } // namespace protocol
} // namespace util
