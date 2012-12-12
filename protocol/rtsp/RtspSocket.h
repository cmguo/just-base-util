// RtspSocket.h

#ifndef _UTIL_PROTOCOL_RTSP_RTSP_SOCKET_H_
#define _UTIL_PROTOCOL_RTSP_RTSP_SOCKET_H_

#include "util/protocol/rtsp/RtspHead.h"

#include <framework/network/TcpSocket.h>

#include <boost/asio/streambuf.hpp>

namespace util
{
    namespace protocol
    {

        class RtspPacket;

        class RtspSocket
            : public framework::network::TcpSocket
        {
        public:
            typedef framework::network::TcpSocket super;

        public:
            RtspSocket(
                boost::asio::io_service & io_svc);

        public:
            void close();

            boost::system::error_code close(
                boost::system::error_code & ec);

        public:
            size_t write(
                RtspHead & head);

            size_t write(
                RtspHead & head, 
                boost::system::error_code & ec);

            template <typename Handler>
            void async_write(
                RtspHead & head, 
                Handler const & handler);

            size_t read(
                RtspHead & head);

            size_t read(
                RtspHead & head, 
                boost::system::error_code & ec);

            template <typename Handler>
            void async_read(
                RtspHead & head, 
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

        private:
            BOOST_STATIC_CONSTANT(size_t, BUF_SIZE = 2048);

        private:
            boost::asio::streambuf snd_buf_;
            boost::asio::streambuf rcv_buf_;
        };

    } // namespace protocol
} // namespace util

#endif // _UTIL_PROTOCOL_RTSP_RTSP_SOCKET_H_
