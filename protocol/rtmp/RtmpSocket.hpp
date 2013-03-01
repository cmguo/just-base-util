// RtmpSocket.hpp

#ifndef _UTIL_PROTOCOL_RTMP_RTMP_SOCKET_HPP_
#define _UTIL_PROTOCOL_RTMP_RTMP_SOCKET_HPP_

#include "util/protocol/MessageSocket.hpp"
#include "util/protocol/rtmp/RtmpSocket.h"
#include "util/protocol/rtmp/RtmpFormat.h"
#include "util/protocol/rtmp/RtmpMessageHelper.h"

#include <framework/network/AsioHandlerHelper.h>
#include <framework/network/TcpSocket.hpp>

#include <boost/asio/read.hpp>
#include <boost/asio/write.hpp>

namespace util
{
    namespace protocol
    {

        struct RtmpMessageVector
        {
            RtmpMessageVector(
                std::vector<RtmpMessage> const & msgs)
                : msgs_(msgs)
            {
            }

            void to_data(
                StreamBuffer & buf, 
                void * ctx) const
            {
                for (size_t i = 0; i < msgs_.size(); ++i) {
                    msgs_[i].to_data(buf, ctx);
                }
            }

        private:
            std::vector<RtmpMessage> const & msgs_;
        };

        template <typename Handler>
        void RtmpSocket::async_write_msgs(
            std::vector<RtmpMessage> const & msgs, 
            Handler const & handler)
        {
            async_write_msg(RtmpMessageVector(msgs), handler);
        }


        namespace detail
        {

            template <typename Handler>
            struct rtmp_connect_handler
            {
                rtmp_connect_handler(
                     RtmpSocket & socket, 
                     Handler const & handler)
                     : socket_(socket)
                     , handler_(handler)
                {
                }

                void operator()(
                    boost::system::error_code const & ec)
                {
                    socket_.handle_connect(ec, handler_);
                }

                void operator()(
                    boost::system::error_code const & ec, 
                    size_t bytes_transferred)
                {
                    socket_.handle_connect(ec, handler_);
                }

            private:
                RtmpSocket & socket_;
                Handler handler_;
            };

            template <typename Handler>
            struct rtmp_accept_handler
            {
                rtmp_accept_handler(
                    RtmpSocket & socket, 
                    Handler const & handler)
                    : socket_(socket)
                    , handler_(handler)
                {
                }

                void operator()(
                    boost::system::error_code const & ec)
                {
                    socket_.handle_accept(ec, handler_);
                }

                void operator()(
                    boost::system::error_code const & ec, 
                    size_t bytes_transferred)
                {
                    socket_.handle_accept(ec, handler_);
                }

            private:
                RtmpSocket & socket_;
                Handler handler_;
            };

        } // namespace detail

        template <typename Handler>
        void RtmpSocket::async_connect(
            framework::network::NetName const & addr, 
            Handler const & handler)
        {
            MessageSocket::async_connect(addr, 
                detail::rtmp_connect_handler<Handler>(*this, handler));
        }

        template <typename Handler>
        void RtmpSocket::handle_connect(
            boost::system::error_code const & ec, 
            Handler const & handler)
        {
            if (ec) {
                handler(ec);
                return;
            }

            switch (status_) {
                case closed:
                    make_c01();
                    status_ = handshake1;
                    boost::asio::async_write(*this, snd_buf_.data(), 
                        boost::asio::transfer_all(), 
                        detail::rtmp_connect_handler<Handler>(*this, handler));
                    break;
                case handshake1:
                    snd_buf_.consume(snd_buf_.size());
                    status_ = handshake2;
                    boost::asio::async_read(*this, rcv_buf_.prepare(1 + HANDSHAKE_SIZE * 2), 
                        boost::asio::transfer_all(), 
                        detail::rtmp_connect_handler<Handler>(*this, handler));
                    break;
                case handshake2:
                    rcv_buf_.commit(1 + HANDSHAKE_SIZE * 2);
                    status_ = handshake3;
                    make_c2();
                    rcv_buf_.consume(1 + HANDSHAKE_SIZE * 2);
                    boost::asio::async_write(*this, snd_buf_.data(), 
                        boost::asio::transfer_all(), 
                        detail::rtmp_connect_handler<Handler>(*this, handler));
                    break;
                case handshake3:
                    snd_buf_.consume(snd_buf_.size());
                    status_ = established;
                    handler(ec);
                    break;
                default:
                    assert(false);
                    break;
            }
        }

        template <typename Handler>
        void RtmpSocket::async_accept(
            framework::network::Acceptor & acceptor, 
            Handler const & handler)
        {
            MessageSocket::async_accept(acceptor, 
                detail::rtmp_accept_handler<Handler>(*this, handler));
        }

        template <typename Handler>
        void RtmpSocket::handle_accept(
            boost::system::error_code const & ec, 
            Handler const & handler)
        {
            if (ec) {
                handler(ec);
                return;
            }

            switch (status_) {
                case closed:
                    status_ = handshake1;
                    boost::asio::async_read(*this, rcv_buf_.prepare(1 + HANDSHAKE_SIZE), 
                        boost::asio::transfer_all(), 
                        detail::rtmp_accept_handler<Handler>(*this, handler));
                    break;
                case handshake1:
                    rcv_buf_.commit(1 + HANDSHAKE_SIZE);
                    make_s012();
                    status_ = handshake2;
                    rcv_buf_.consume(1 + HANDSHAKE_SIZE);
                    boost::asio::async_write(*this, snd_buf_.data(), 
                        boost::asio::transfer_all(), 
                        detail::rtmp_accept_handler<Handler>(*this, handler));
                    break;
                case handshake2:
                    snd_buf_.consume(snd_buf_.size());
                    status_ = handshake3;
                    boost::asio::async_read(*this, rcv_buf_.prepare(HANDSHAKE_SIZE), 
                        boost::asio::transfer_all(), 
                        detail::rtmp_accept_handler<Handler>(*this, handler));
                    break;
                case handshake3:
                    rcv_buf_.commit(HANDSHAKE_SIZE);
                    status_ = established;
                    rcv_buf_.consume(HANDSHAKE_SIZE);
                    handler(ec);
                    break;
                default:
                    assert(false);
                    break;
            }
        }

    } // namespace protocol
} // namespace util

#endif // _UTIL_PROTOCOL_RTMP_RTMP_SOCKET_HPP_
