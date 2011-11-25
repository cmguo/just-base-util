// HttpChunkedSocket.h

#ifndef _UTIL_PROTOCOL_HTTP_CHUNKED_SOCKET_H_
#define _UTIL_PROTOCOL_HTTP_CHUNKED_SOCKET_H_

#include "util/buffers/BufferSize.h"
#include "util/buffers/SubBuffers.h"

#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/streambuf.hpp>
#include <boost/asio/read_until.hpp>
#include <boost/asio/write.hpp>
#include <boost/asio/socket_base.hpp>

namespace util
{
    namespace protocol
    {

        class HttpPacket;

        namespace detail
        {

        } // namespace detail

        static char const hex_chr[] = "0123456789ABCDEF";

        template <
            typename Socket
        >
        class HttpChunkedSocket
        {

        public:
            HttpChunkedSocket(
                Socket & socket)
                : socket_(socket)
                , snd_left_(0)
                , rcv_left_(0)
            {
            }

        public:
            void close()
            {
                snd_left_ = 0;
                snd_buf_.reset();
                rcv_left_ = 0;
                rcv_buf_.reset();
                socket_.close();
            }

            boost::system::error_code close(
                boost::system::error_code & ec)
            {
                snd_left_ = 0;
                snd_buf_.reset();
                rcv_left_ = 0;
                rcv_buf_.reset();
                return socket_.close(ec);
            }

        public:
            struct eof {};

        public:
            // 重写receive，async_receive，read_some，async_read_some
            template <typename MutableBufferSequence>
            std::size_t receive(
                const MutableBufferSequence & buffers)
            {
                using namespace boost::asio;

                if (rcv_buf_.size() > 0) {
                    return copy(buffers);
                } else {
                    return socket_.receive(buffers);
                }
            }

            template <typename MutableBufferSequence>
            std::size_t receive(const MutableBufferSequence& buffers, 
                boost::asio::socket_base::message_flags flags)
            {
                using namespace boost::asio;

                if (rcv_buf_.size() > 0) {
                    return copy(buffers);
                } else {
                    return socket_.receive(buffers, flags);
                }
            }

            template <typename MutableBufferSequence>
            std::size_t receive(
                const MutableBufferSequence& buffers, 
                boost::asio::socket_base::message_flags flags, 
                boost::system::error_code& ec)
            {
                using namespace boost::asio;

                if (rcv_buf_.size() > 0) {
                    ec = boost::system::error_code();
                    return copy(buffers);
                } else {
                    return socket_.receive(buffers, flags, ec);
                }
            }

            template <typename MutableBufferSequence, typename ReadHandler>
            void async_receive(
                const MutableBufferSequence& buffers,
                ReadHandler handler)
            {
                if (rcv_buf_.size() > 0) {
                    std::size_t length = copy(buffers);
                    socket_.get_io_service().post(boost::asio::detail::bind_handler(
                        handler, boost::system::error_code(), length));
                } else {
                    socket_.async_receive(buffers, handler);
                }
            }

            template <typename MutableBufferSequence, typename ReadHandler>
            void async_receive(
                const MutableBufferSequence& buffers,
                boost::asio::socket_base::message_flags flags, 
                ReadHandler handler)
            {
                if (rcv_buf_.size() > 0) {
                    std::size_t length = copy(buffers);
                    socket_.get_io_service().post(boost::asio::detail::bind_handler(
                        handler, boost::system::error_code(), length));
                } else {
                    socket_.async_receive(buffers, flags, handler);
                }
            }

            template <typename MutableBufferSequence>
            std::size_t read_some(
                const MutableBufferSequence& buffers)
            {
                using namespace boost::asio;

                if (rcv_buf_.size() > 0) {
                    return copy(buffers);
                } else {
                    return socket_.read_some(buffers);
                }
            }

            template <typename MutableBufferSequence>
            std::size_t read_some(
                const MutableBufferSequence& buffers,
                boost::system::error_code& ec)
            {
                using namespace boost::asio;

                if (rcv_buf_.size() > 0) {
                    ec = boost::system::error_code();
                    return copy(buffers);
                } else {
                    return socket_.read_some(buffers, ec);
                }
            }

            template <typename MutableBufferSequence, typename ReadHandler>
            void async_read_some(
                const MutableBufferSequence& buffers,
                ReadHandler handler)
            {
                if (rcv_buf_.size() > 0) {
                    std::size_t length = copy(buffers);
                    socket_.get_io_service().post(boost::asio::detail::bind_handler(
                        handler, boost::system::error_code(), length));
                } else {
                    socket_.async_read_some(buffers, handler);
                }
            }

            template <typename MutableBufferSequence>
            std::size_t copy(
                const MutableBufferSequence& buffers)
            {
                using namespace std; // For memcpy.
                using namespace boost::asio;

                std::size_t bytes_avail = rcv_buf_.size();
                std::size_t bytes_copied = 0;

                typename MutableBufferSequence::const_iterator iter = buffers.begin();
                typename MutableBufferSequence::const_iterator end = buffers.end();
                for (; iter != end && bytes_avail > 0; ++iter) {
                    std::size_t max_length = buffer_size(*iter);
                    std::size_t length = (max_length < bytes_avail)
                        ? max_length : bytes_avail;
                    memcpy(buffer_cast<void *>(*iter), buffer_cast<char const *>(rcv_buf_.data()) + bytes_copied, length);
                    bytes_copied += length;
                    bytes_avail -= length;
                }

                rcv_buf_.consume(bytes_copied);
                return bytes_copied;
            }

            // 重写send，async_send，write_some，async_write_some

            template <typename ConstBufferSequence>
            std::size_t send(
                const ConstBufferSequence & buffers)
            {
                boost::system::error_code ec;
                std::size_t s = send(buffers, 0, ec);
                boost::asio::detail::throw_error(ec);
                return s;
            }

            template <typename ConstBufferSequence>
            std::size_t send(
                const ConstBufferSequence & buffers, 
                boost::asio::socket_base::message_flags flags)
            {
                boost::system::error_code ec;
                std::size_t s = send(buffers, flags, ec);
                boost::asio::detail::throw_error(ec);
                return s;
            }

            template <typename ConstBufferSequence>
            std::size_t send(
                const ConstBufferSequence & buffers, 
                boost::asio::socket_base::message_flags flags, 
                boost::system::error_code & ec)
            {
                using namespace boost::asio;

                std::size_t bytes_send = 0;
                std::size_t bytes_left = buffers::buffer_size(buffers);
                while (true) {
                    if (snd_buf_.size()) {
                        // 剩余的Chunk头部或者尾部数据
                        std::size_t len = socket_.send(snd_buf_.data(), flags, ec);
                        snd_buf_.consume(len);
                        if (snd_buf_.size()) {
                            break;
                        }
                    }
                    if (snd_left_ == 0) {
                        // 刚刚发送了尾部，重组新的Chunk
                        if (bytes_left == 0)
                            break;
                        snd_left_ = bytes_left;
                        std::size_t len1 = snd_left_;
                        char * hex_buf = buffer_cast<char *>(snd_buf_.prepare(10));
                        char * p = hex_buf + 10;
                        *--p = '\n';
                        *--p = '\r';
                        while (len1) {
                            --p;
                            *p = hex_chr[len1 & 0x0000000f];
                            len1 >>= 4;
                        }
                        snd_buf_.commit(10);
                        snd_buf_.consume(p - hex_buf);
                    } else {
                        // 发送完头部，发送剩余数据
                        std::size_t len = socket_.send(buffers::sub_buffers(buffers, bytes_send, snd_left_));
                        bytes_send += len;
                        bytes_left -= len;
                        snd_left_ -= len;
                        if (snd_left_)
                            break;
                        // 发送完数据，构建尾部
                        char * crlf = buffer_cast<char *>(snd_buf_.prepare(2));
                        *crlf++ = '\r';
                        *crlf++ = '\n';
                        snd_buf_.commit(2);
                    }
                }
                return bytes_send;
            }

            std::size_t send(
                const eof &, 
                boost::asio::socket_base::message_flags flags, 
                boost::system::error_code & ec)
            {
                assert(snd_left_ == 0);
                if (snd_buf_.size() == 0) {
                    char * crlf = buffer_cast<char *>(snd_buf_.prepare(5));
                    *crlf++ = '0';
                    *crlf++ = '\r';
                    *crlf++ = '\n';
                    *crlf++ = '\r';
                    *crlf++ = '\n';
                    snd_buf_.commit(5);
                }
                if (snd_buf_.size()) {
                    std::size_t len = socket_.send(snd_buf_.data(), flags, ec);
                    snd_buf_.consume(len);
                    if (snd_buf_.size() == 0) {
                        // 如果再继续send eof，就会失败
                        snd_left_ = (size_t)-1;
                    }
                }
            }

            template <typename ConstBufferSequence>
            std::size_t write_some(
                const ConstBufferSequence & buffers)
            {
                boost::system::error_code ec;
                std::size_t s = send(buffers, 0, ec);
                boost::asio::detail::throw_error(ec);
                return s;
            }

            template <typename ConstBufferSequence>
            std::size_t write_some(
                const ConstBufferSequence & buffers, 
                boost::system::error_code & ec)
            {
                return send(buffers, 0, ec);
            }

        private:
            BOOST_STATIC_CONSTANT(size_t, BUF_SIZE = 2048);

        private:
            Socket & socket_;
            size_t snd_left_;
            boost::asio::streambuf snd_buf_;
            size_t rcv_left_;
            boost::asio::streambuf rcv_buf_;
        };

    } // namespace protocol
} // namespace util

#endif // _UTIL_PROTOCOL_HTTP_CHUNKED_SOCKET_H_
