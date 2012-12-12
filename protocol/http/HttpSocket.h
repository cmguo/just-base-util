// HttpSocket.h

#ifndef _UTIL_PROTOCOL_HTTP_HTTP_SOCKET_H_
#define _UTIL_PROTOCOL_HTTP_HTTP_SOCKET_H_

#include "util/protocol/http/HttpHead.h"
#include "util/stream/Socket.h"

#include <framework/network/TcpSocket.h>

#include <boost/asio/streambuf.hpp>

namespace util
{
    namespace protocol
    {

        class HttpPacket;

        class HttpSocket
            : public framework::network::TcpSocket
        {
        public:
            typedef framework::network::TcpSocket super;

        public:
            HttpSocket(
                boost::asio::io_service & io_svc);

        public:
            void close();

            boost::system::error_code close(
                boost::system::error_code & ec);

        public:
            size_t write(
                HttpHead & head);

            size_t write(
                HttpHead & head, 
                boost::system::error_code & ec);

            template <typename Handler>
            void async_write(
                HttpHead & head, 
                Handler const & handler);

            size_t read(
                HttpHead & head);

            size_t read(
                HttpHead & head, 
                boost::system::error_code & ec);

            template <typename Handler>
            void async_read(
                HttpHead & head, 
                Handler const & handler);

        public:
            // ÷ÿ–¥receive£¨async_receive£¨read_some£¨async_read_some
            template <typename MutableBufferSequence>
            std::size_t receive(
                const MutableBufferSequence & buffers);

            template <typename MutableBufferSequence>
            std::size_t receive(const MutableBufferSequence& buffers, 
                socket_base::message_flags flags);

            template <typename MutableBufferSequence>
            std::size_t receive(
                const MutableBufferSequence& buffers, 
                socket_base::message_flags flags, 
                boost::system::error_code& ec);

            template <typename MutableBufferSequence, typename ReadHandler>
            void async_receive(
                const MutableBufferSequence& buffers,
                ReadHandler handler);

            template <typename MutableBufferSequence, typename ReadHandler>
            void async_receive(
                const MutableBufferSequence& buffers,
                socket_base::message_flags flags, 
                ReadHandler handler);

            template <typename MutableBufferSequence>
            std::size_t read_some(
                const MutableBufferSequence& buffers);

            template <typename MutableBufferSequence>
            std::size_t read_some(
                const MutableBufferSequence& buffers,
                boost::system::error_code& ec);

            template <typename MutableBufferSequence, typename ReadHandler>
            void async_read_some(
                const MutableBufferSequence& buffers,
                ReadHandler handler);

            /// Start an asynchronous read. The buffer into which the data will be read
            /// must be valid for the lifetime of the asynchronous operation.
            template <typename MutableBufferSequence>
            std::size_t copy(
                const MutableBufferSequence& buffers);

        protected:
            void set_source(
                HttpHead const & head);

            void reset_source(
                HttpHead const & head);

            void set_sink(
                HttpHead const & head);

            void reset_sink(
                HttpHead const & head);

            util::stream::Source & source();

            util::stream::Sink & sink();

        private:
            BOOST_STATIC_CONSTANT(size_t, BUF_SIZE = 2048);

        private:
            bool non_block_;
            boost::asio::streambuf snd_buf_;
            boost::asio::streambuf rcv_buf_;

        private:
            util::stream::Socket<HttpSocket> stream_;
            util::stream::Source * source_;
            util::stream::Sink * sink_;
        };

    } // namespace protocol
} // namespace util

#endif // _UTIL_PROTOCOL_HTTP_HTTP_SOCKET_H_
