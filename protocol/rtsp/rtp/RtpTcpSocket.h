// RtpTcpSocket.h

#ifndef _UTIL_PROTOCOL_RTSP_RTP_RTP_TCP_SOCKET_H_
#define _UTIL_PROTOCOL_RTSP_RTP_RTP_TCP_SOCKET_H_

#include <util/stream/Dual.h>

namespace util
{
    namespace protocol
    {

        class RtspSocket;

        using util::stream::StreamMutableBuffers;
        using util::stream::StreamConstBuffers;
        using util::stream::StreamHandler;

        class RtpTcpSocket
            : public util::stream::Dual
        {
        public:
            RtpTcpSocket(
                RtspSocket & rtsp_socket);

        public:
            bool create_port(
                boost::uint8_t interleaved[2], 
                boost::system::error_code & ec);

            bool connect_port(
                boost::uint8_t interleaved[2],
                boost::system::error_code & ec);

        private:
            virtual std::size_t private_read_some(
                StreamMutableBuffers const & buffers,
                boost::system::error_code & ec);

            virtual void private_async_read_some(
                StreamMutableBuffers const & buffers, 
                StreamHandler const & handler);

            virtual std::size_t private_write_some(
                StreamConstBuffers const & buffers, 
                boost::system::error_code & ec);

            virtual void private_async_write_some(
                StreamConstBuffers const & buffers, 
                StreamHandler const & handler);

        private:
            virtual bool set_non_block(
                bool non_block, 
                boost::system::error_code & ec);

        private:
            RtspSocket & rtsp_socket_;
            std::vector<boost::uint8_t> connections_;
        };

    } // namespace protocol
} // namespace util

#endif // _UTIL_PROTOCOL_RTSP_RTP_RTP_TCP_SOCKET_H_
