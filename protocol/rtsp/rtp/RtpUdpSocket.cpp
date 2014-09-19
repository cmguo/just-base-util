// RtpUdpSocket.cpp

#include "util/Util.h"
#include "util/protocol/rtsp/rtp/RtpUdpSocket.h"
#include "util/protocol/rtsp/rtp/RtpUdpSocket.hpp"
#include "util/protocol/rtsp/RtspSocket.h"
#include "util/buffers/BuffersCopy.h"
#include "util/buffers/SubBuffers.h"

#include <framework/thread/MessageQueue.h>
#include <framework/logger/Logger.h>
#include <framework/logger/StreamRecord.h>

#include <boost/thread/thread.hpp>

using namespace util::buffers;

namespace util
{
    namespace protocol
    {

        FRAMEWORK_LOGGER_DECLARE_MODULE_LEVEL("util.protocol.RtpUdpSocket", framework::logger::Warn);

        RtpUdpSocket::RtpUdpSocket(
            RtspSocket & rtsp_socket)
            : util::stream::Dual(rtsp_socket.get_io_service())
            , rtsp_socket_(rtsp_socket)
            , read_handler_(new ReadHandler())
        {
        }

        RtpUdpSocket::~RtpUdpSocket()
        {
            delete read_handler_;
            for (size_t i = 0; i < sockets_.size(); ++i) {
                delete sockets_[i];
            }
        }

        bool RtpUdpSocket::create_port(
            boost::uint16_t port[], 
            boost::system::error_code & ec)
        {
            boost::asio::io_service & io_svc(get_io_service());
            std::auto_ptr<Socket> rtp(new Socket(io_svc));
            std::auto_ptr<Socket> rtcp(new Socket(io_svc));

            port[0] = 0;
            boost::asio::ip::udp::endpoint endpoint(
                rtsp_socket_.local_endpoint().address(), 0);
            while (true) {
                endpoint.port(port[0]);
                rtp->open(endpoint.protocol(), ec)
                    || rtp->bind(endpoint, ec);
                if (port[0] == 0 && !ec) {
                    port[0] = rtp->local_endpoint(ec).port();
                    if (port[0] & 1) {
                        --port[0];
                        ec = boost::asio::error::address_in_use;
                    }
                }
                if (ec == boost::asio::error::address_in_use) {
                    port[0] += 2;
                    rtp->close(ec);
                    continue;
                }
                if (ec) {
                    break;
                }
                port[1] = port[0] + 1;
                endpoint.port(port[1]);
                rtcp->open(endpoint.protocol(), ec)
                    || rtcp->bind(endpoint, ec);
                if (ec == boost::asio::error::address_in_use) {
                    port[0] += 2;
                    rtp->close(ec);
                    rtcp->close(ec);
                    continue;
                }
                break;
            }
            if (!ec) {
                rtp->index = (boost::uint8_t)sockets_.size();
                sockets_.push_back(rtp.release());
                rtcp->index = (boost::uint8_t)sockets_.size();
                sockets_.push_back(rtcp.release());
                return true;
            }
            return false;
        }

        bool RtpUdpSocket::connect_port(
            boost::uint16_t port[2],
            boost::system::error_code & ec)
        {
            size_t n = sockets_.size() - 2;
            boost::asio::ip::udp::endpoint endpoint(
                rtsp_socket_.remote_endpoint().address(), port[0]);
            sockets_[n]->connect(endpoint, ec);
            if (ec)
                return false;
            endpoint.port(port[1]);
            sockets_[n + 1]->connect(endpoint, ec);
            if (ec)
                return false;
            boost::uint8_t nat[] = {0xce, 0xfa, 0xed, 0xfe};
            sockets_[n]->send(boost::asio::buffer(nat), 0, ec);
            sockets_[n + 1]->send(boost::asio::buffer(nat), 0, ec);
            read_handler_->add_socket(sockets_[n]);
            read_handler_->add_socket(sockets_[n + 1]);
            return true;
        }

        std::size_t RtpUdpSocket::private_read_some(
            StreamMutableBuffers const & buffers,
            boost::system::error_code & ec)
        {
            return read_handler_->read_some(
                buffers, ec);
        }

        void RtpUdpSocket::private_async_read_some(
            StreamMutableBuffers const & buffers, 
            StreamHandler const & handler)
        {
            read_handler_->async_read_some(
                buffers,
                get_io_service().wrap(handler));
        }

        std::size_t RtpUdpSocket::private_write_some(
            StreamConstBuffers const & buffers, 
            boost::system::error_code & ec)
        {
            boost::uint8_t b[4];
            buffers_copy(boost::asio::buffer(b), buffers);
            size_t bytes_transferred = sockets_[b[1]]->send(
                sub_buffers(buffers, 4), 0, ec);
            if (bytes_transferred)
                bytes_transferred += 4;
            return bytes_transferred;
        }

        void RtpUdpSocket::private_async_write_some(
            StreamConstBuffers const & buffers, 
            StreamHandler const & handler)
        {
            boost::uint8_t b[4];
            buffers_copy(boost::asio::buffer(b), buffers);
            sockets_[b[1]]->async_send(
                sub_buffers(buffers, 4),
                handler_wrapper(get_io_service(), handler));
        }

        bool RtpUdpSocket::set_non_block(
            bool non_block, 
            boost::system::error_code & ec)
        {
            boost::asio::socket_base::non_blocking_io cmd(non_block);
            for (size_t i = 0; i < sockets_.size(); ++i) {
                sockets_[i]->io_control(cmd, ec);
                if (ec)
                    return false;
            }
            read_handler_->set_non_block(non_block);
            return true;
        }

    } // namespace protocol
} // namespace util
