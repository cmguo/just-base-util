// RtmpSocket.h

#ifndef _UTIL_PROTOCOL_RTMP_RTMP_SOCKET_H_
#define _UTIL_PROTOCOL_RTMP_RTMP_SOCKET_H_

#include "util/protocol/rtmp/RtmpMessage.h"
#include "util/protocol/rtmp/RtmpMessageContext.h"
#include "util/protocol/rtmp/RtmpChunkParser.h"
#include "util/protocol/MessageSocket.h"

#include <framework/network/TcpSocket.h>

#include <boost/asio/streambuf.hpp>

namespace util
{
    namespace protocol
    {

        class RtmpChunk;
        class RtmpRawChunk;

        namespace detail
        {

            template <typename Handler>
            struct rtmp_connect_handler;

            template <typename Handler>
            struct rtmp_accept_handler;

            template <
                typename MutableBufferSequence, 
                typename Handler
            >
            struct rtmp_raw_msg_read_handler;

            template <
                typename Handler
            >
            struct rtmp_msg_read_handler;

        }

        class RtmpSocket
            : public MessageSocket
        {
        public:
            RtmpSocket(
                boost::asio::io_service & io_svc);

            ~RtmpSocket();

        public:
            enum StatusEnum
            {
                closed = 0, 
                handshake1, 
                handshake2, 
                handshake3,
                established,
            };

        public:
            void connect(
                framework::network::NetName const & addr);

            boost::system::error_code connect(
                framework::network::NetName const & addr, 
                boost::system::error_code & ec);

            template <typename Handler>
            void async_connect(
                framework::network::NetName const & addr, 
                Handler const & handler);

        public:
            void accept(
                framework::network::Acceptor & acceptor);

            boost::system::error_code accept(
                framework::network::Acceptor & acceptor, 
                boost::system::error_code & ec);

            template <typename Handler>
            void async_accept(
                framework::network::Acceptor & acceptor, 
                Handler const & handler);

        public:
            template <
                typename MutableBufferSequence
            >
            size_t read_raw_msg(
                MutableBufferSequence const & buffers, 
                boost::system::error_code & ec);

            template <
                typename MutableBufferSequence, 
                typename Handler
            >
            void async_read_raw_msg(
                MutableBufferSequence const & buffers, 
                Handler const & handler);

            size_t read_msg(
                RtmpMessage & msg, 
                boost::system::error_code & ec);

            template <
                typename Handler
            >
            void async_read_msg(
                RtmpMessage & msg, 
                Handler const & handler);

        public:
            size_t write_msgs(
                std::vector<RtmpMessage> const & msgs, 
                boost::system::error_code & ec);

            template <typename Handler>
            void async_write_msgs(
                std::vector<RtmpMessage> const & msgs, 
                Handler const & handler);

        public:
            bool process_protocol_message(
                RtmpMessage const & msg, 
                std::vector<RtmpMessage> & resp);

        private:
            template <
                typename MutableBufferSequence, 
                typename Handler
            >
            friend struct detail::rtmp_raw_msg_read_handler;

            template <
                typename Handler
            >
            friend struct detail::rtmp_msg_read_handler;

            template <
                typename MutableBufferSequence, 
                typename Handler
            >
            void handle_read_raw_msg(
                MutableBufferSequence const & buffers, 
                Handler const & handler, 
                RtmpRawChunk & chunk, 
                boost::system::error_code ec, 
                size_t bytes_transferred);

            template <
                typename Handler
            >
            void handle_read_msg(
                RtmpMessage & msg, 
                Handler const & handler, 
                RtmpChunk & chunk, 
                boost::system::error_code ec, 
                size_t bytes_transferred);

        private:
            static boost::uint32_t const RANDOM_SIZE = 1528;
            static boost::uint32_t const HANDSHAKE_SIZE = 8 + RANDOM_SIZE;

            void make_c01();
            
            void make_c2();

            void make_s012();

        private:
            template <typename Handler>
            friend struct detail::rtmp_connect_handler;

            template <typename Handler>
            friend struct detail::rtmp_accept_handler;

            template <typename Handler>
            void handle_connect(
                boost::system::error_code const & ec, 
                Handler const & handler);

            template <typename Handler>
            void handle_accept(
                boost::system::error_code const & ec, 
                Handler const & handler);

        private:
            StatusEnum status_;
            RtmpMessageContext context_;
            RtmpChunkParser read_parser_;
        };

    } // namespace protocol
} // namespace util

#endif // _UTIL_PROTOCOL_RTMP_RTMP_SOCKET_H_
