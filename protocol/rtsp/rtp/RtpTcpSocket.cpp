// RtpTcpSocket.cpp

#include "util/Util.h"
#include "util/protocol/rtsp/rtp/RtpTcpSocket.h"
#include "util/protocol/rtsp/RtspSocket.hpp"

namespace util
{
    namespace protocol
    {

        RtpTcpSocket::RtpTcpSocket(
            RtspSocket & rtsp_socket)
            : util::stream::Dual(rtsp_socket.get_io_service())
            , rtsp_socket_(rtsp_socket)
        {
        }

        bool RtpTcpSocket::create_port(
            boost::uint8_t interleaved[2], 
            boost::system::error_code & ec)
        {
            interleaved[0] = (boost::uint8_t)connections_.size();
            connections_.push_back(0);
            interleaved[1] = (boost::uint8_t)connections_.size();
            connections_.push_back(0);
            ec.clear();
            return true;
        }

        bool RtpTcpSocket::connect_port(
            boost::uint8_t interleaved[2],
            boost::system::error_code & ec)
        {
            size_t i = connections_.size() - 2;
            connections_[i] = interleaved[0];
            connections_[i + 1] = interleaved[1];
            ec.clear();
            return true;
        }

        std::size_t RtpTcpSocket::private_read_some(
            StreamMutableBuffers const & buffers,
            boost::system::error_code & ec)
        {
            return rtsp_socket_.read_raw_msg(
                buffers, ec);
        }

        void RtpTcpSocket::private_async_read_some(
            StreamMutableBuffers const & buffers, 
            StreamHandler const & handler)
        {
            rtsp_socket_.async_read_raw_msg(
                buffers,
                handler);
        }

        std::size_t RtpTcpSocket::private_write_some(
            StreamConstBuffers const & buffers, 
            boost::system::error_code & ec)
        {
            return rtsp_socket_.write_raw_msg(
                buffers, ec);
        }

        void RtpTcpSocket::private_async_write_some(
            StreamConstBuffers const & buffers, 
            StreamHandler const & handler)
        {
            rtsp_socket_.async_write_raw_msg(
                buffers,
                handler);
        }

        bool RtpTcpSocket::set_non_block(
            bool non_block, 
            boost::system::error_code & ec)
        {
            rtsp_socket_.set_read_parallel(true);
            return rtsp_socket_.set_non_block(non_block, ec);
        }

    } // namespace protocol
} // namespace util
