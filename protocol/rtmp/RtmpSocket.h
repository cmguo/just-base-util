// RtmpSocket.h

#ifndef _UTIL_PROTOCOL_RTMP_RTMP_SOCKET_H_
#define _UTIL_PROTOCOL_RTMP_RTMP_SOCKET_H_

#include "util/protocol/rtmp/RtmpMessage.h"
#include "util/protocol/rtmp/RtmpMessageParser.h"
#include "util/protocol/MessageSocket.h"

#include <framework/network/TcpSocket.h>

#include <boost/asio/streambuf.hpp>

namespace util
{
    namespace protocol
    {

        class RtmpMessageParser;

        namespace detail
        {

            template <typename Handler>
            struct rtmp_connect_handler;

            template <typename Handler>
            struct rtmp_accept_handler;

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
            RtmpMessageParser read_parser_;
            RtmpMessageParser write_parser_;
        };

    } // namespace protocol
} // namespace util

#endif // _UTIL_PROTOCOL_RTMP_RTMP_SOCKET_H_
