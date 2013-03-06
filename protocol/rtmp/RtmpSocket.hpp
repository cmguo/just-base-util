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
                void * ctx) const;

        private:
            std::vector<RtmpMessage> const & msgs_;
        };

        struct RtmpChunk
        {
            RtmpChunk()
                : finish(false)
                , cs_id(0)
            {
            }

            void from_data(
                StreamBuffer & buf, 
                void * vctx);

            bool finish;
            boost::uint16_t cs_id;
        };

        struct RtmpRawChunk
        {
            RtmpRawChunk()
                : finish(false)
                , pos(0)
                , cs_id(0)
            {
            }

            template <
                typename MutableBufferSequence
            >
            void from_data(
                MutableBufferSequence const & buffers, 
                boost::uint32_t size, 
                RtmpMessageContext * ctx) const
            {
                boost::uint8_t const * p = NULL;
                size_t n = pos;
                typename MutableBufferSequence::const_iterator iter = buffers.begin();
                for (; iter != buffers.end(); ++iter) {
                    if (n < boost::asio::buffer_size(*iter)) {
                        p = boost::asio::buffer_cast<boost::uint8_t const *>(*iter) + n;
                        n = boost::asio::buffer_size(*iter) - n;
                        break;
                    }
                }
                assert(p);
                RtmpChunkBasicHeader h;
                h.one_byte = p[0];
                if (h.cs_id0 < 2) {
                    if (h.cs_id0 == 0) {
                        h.cs_id1 = get_char(buffers, iter, p, n);
                    } else {
                        boost::uint8_t b1 = get_char(buffers, iter, p, n);
                        boost::uint8_t b2 = get_char(buffers, iter, p, n);
                        h.cs_id2 = (boost::uint16_t)b1 << 8 | b2;
                    }
                }
                assert(pos == 0 || cs_id == h.cs_id);
                cs_id = h.cs_id();
                finish = ctx->read_chunk(cs_id).put_data(size, ctx->read_chunk_size());
                pos += size;
            }

            template <
                typename MutableBufferSequence
            >
            static boost::uint8_t get_char(
                MutableBufferSequence const & buffers, 
                typename MutableBufferSequence::const_iterator & iter, 
                boost::uint8_t const *& p, 
                size_t & n)
            {
                while (n == 0) {
                    ++iter;
                    assert(iter != buffers.end());
                    p = boost::asio::buffer_cast<boost::uint8_t const *>(*iter);
                    n = boost::asio::buffer_size(*iter);
                }
                --n;
                return *p++;
            }

            bool finish;
            size_t pos;
            boost::uint16_t cs_id;
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
                    make_c01(snd_buf_);
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
                    check_s012(rcv_buf_);
                    status_ = handshake3;
                    make_c2(snd_buf_);
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
                    check_c01(rcv_buf_);
                    rcv_buf_.consume(1 + HANDSHAKE_SIZE);
                    status_ = handshake2;
                    make_s012(snd_buf_);
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
                    check_c2(rcv_buf_);
                    rcv_buf_.consume(HANDSHAKE_SIZE);
                    status_ = established;
                    handler(ec);
                    break;
                default:
                    assert(false);
                    break;
            }
        }

        template <
            typename MutableBufferSequence
        >
        size_t RtmpSocket::read_raw_msg(
            MutableBufferSequence const & buffers, 
            boost::system::error_code & ec)
        {
            RtmpRawChunk chunk;
            size_t bytes_read = 0;
            while ((bytes_read = MessageSocket::read_raw_msg(
                util::buffers::sub_buffers(buffers, chunk.pos), ec))) {
                    chunk.from_data(buffers, bytes_read, context_);
                    if (chunk.finish) {
                        return chunk.pos;
                    }
            }
            return 0;
        }

        namespace detail
        {

            template <
                typename MutableBufferSequence, 
                typename Handler
            >
            struct rtmp_raw_msg_read_handler
            {
                rtmp_raw_msg_read_handler(
                    RtmpSocket & socket, 
                    MutableBufferSequence & buffers, 
                    Handler handler, 
                    RtmpRawChunk & chunk)
                    : socket_(socket)
                    , buffers_(buffers)
                    , handler_(handler)
                    , chunk_(chunk)
                {
                }

                void operator()(
                    boost::system::error_code ec, 
                    size_t bytes_transferred) const
                {
                    socket_.handle_read_raw_msg(buffers_, handler_, ec, bytes_transferred);
                }

                PASS_DOWN_ASIO_HANDLER_FUNCTION(rtmp_raw_msg_read_handler, handler_)

            private:
                RtmpSocket & socket_;
                MutableBufferSequence & buffers_;
                Handler handler_;
                RtmpRawChunk chunk_;
            };

        } // namespace detail

        template <
            typename MutableBufferSequence, 
            typename Handler
        >
        void RtmpSocket::async_read_raw_msg(
            MutableBufferSequence const & buffers, 
            Handler const & handler)
        {
            RtmpRawChunk chunk;
            MessageSocket::async_read_raw_msg(buffers, 
                detail::rtmp_raw_msg_read_handler<MutableBufferSequence, Handler>(*this, buffers, handler, chunk));
        }

        template <
            typename MutableBufferSequence, 
            typename Handler
        >
        void RtmpSocket::handle_read_raw_msg(
            MutableBufferSequence const & buffers, 
            Handler const & handler, 
            RtmpRawChunk & chunk, 
            boost::system::error_code ec, 
            size_t bytes_transferred)
        {
            if (ec) {
                handler(ec, 0);
                return;
            }
            chunk.from_data(buffers, bytes_transferred, context_);
            if (chunk.finish) {
                return chunk.pos;
            }
            MessageSocket::async_read_raw_msg(buffers, 
                detail::rtmp_raw_msg_read_handler<MutableBufferSequence, Handler>(*this, buffers, handler, chunk));
        }

        namespace detail
        {

            template <
                typename Handler
            >
            struct rtmp_msg_read_handler
            {
                rtmp_msg_read_handler(
                    RtmpSocket & socket, 
                    RtmpMessage & msg, 
                    Handler handler, 
                    RtmpChunk & chunk)
                    : socket_(socket)
                    , msg_(msg)
                    , handler_(handler)
                    , chunk_(chunk)
                {
                }

                void operator()(
                    boost::system::error_code ec, 
                    size_t bytes_transferred) const
                {
                    socket_.handle_read_msg(msg_, handler_, chunk_, ec, bytes_transferred);
                }

                PASS_DOWN_ASIO_HANDLER_FUNCTION(rtmp_msg_read_handler, handler_)

            private:
                RtmpSocket & socket_;
                RtmpMessage & msg_;
                Handler handler_;
                RtmpChunk & chunk_;
            };

        } // namespace detail

        template <
            typename Handler
        >
        void RtmpSocket::async_read_msg(
            RtmpMessage & msg, 
            Handler const & handler)
        {
            RtmpChunk * chunk = new RtmpChunk;
            MessageSocket::async_read_msg(*chunk, 
                detail::rtmp_msg_read_handler<Handler>(*this, msg, handler, *chunk));
        }

        template <
            typename Handler
        >
        void RtmpSocket::handle_read_msg(
            RtmpMessage & msg, 
            Handler const & handler, 
            RtmpChunk & chunk, 
            boost::system::error_code ec, 
            size_t bytes_transferred)
        {
            if (chunk.finish) {
                RtmpChunkMessage & cm(context_.read_chunk(chunk.cs_id));
                boost::uint32_t size = cm.data.size();
                msg.from_data(cm.data, &context_);
                handler(ec, size);
                return;
            }
            if (ec) {
                handler(ec, 0);
                return;
            }
            MessageSocket::async_read_msg(chunk, 
                detail::rtmp_msg_read_handler<Handler>(*this, msg, handler, chunk));
        }

    } // namespace protocol
} // namespace util

#endif // _UTIL_PROTOCOL_RTMP_RTMP_SOCKET_HPP_
