// HttpChunkedSocket.h

#ifndef _UTIL_PROTOCOL_HTTP_CHUNKED_SOCKET_H_
#define _UTIL_PROTOCOL_HTTP_CHUNKED_SOCKET_H_

#include "util/buffers/BufferSize.h"
#include "util/buffers/SubBuffers.h"

#include <framework/network/AsioHandlerHelper.h>

#include <boost/asio/streambuf.hpp>
#include <boost/asio/socket_base.hpp>
#include <boost/bind.hpp>
#include <boost/ref.hpp>

namespace util
{
    namespace protocol
    {

        class HttpPacket;

        namespace detail
        {

        } // namespace detail

        static char const hex_chr[] = "0123456789ABCDEF";

        static void make_chunk_head(
            boost::asio::streambuf & buf, 
            std::size_t len)
        {
            using namespace boost::asio;

            char * hex_buf = buffer_cast<char *>(buf.prepare(10));
            char * p = hex_buf + 10;
            *--p = '\n';
            *--p = '\r';
            while (len) {
                --p;
                *p = hex_chr[len & 0x0000000f];
                len >>= 4;
            }
            buf.commit(10);
            buf.consume(p - hex_buf);
        }

        static void make_chunk_tail(
            boost::asio::streambuf & buf)
        {
            using namespace boost::asio;

            char * p = buffer_cast<char *>(buf.prepare(2));
            *p++ = '\r';
            *p++ = '\n';
            buf.commit(2);
        }

        static void make_chunk_eof(
            boost::asio::streambuf & buf)
        {
            using namespace boost::asio;

            char * p = buffer_cast<char *>(buf.prepare(5));
            *p++ = '0';
            *p++ = '\r';
            *p++ = '\n';
            *p++ = '\r';
            *p++ = '\n';
            buf.commit(5);
        }

        bool recv_crlf(
            boost::asio::streambuf & buf, 
            std::size_t & len)
        {
            using namespace boost::asio;

            if (buf.size() == 0) {
                // 如果buf为空，填充一个字节，表示开始接收
                // 输入的 len 表示类型，head：0，tail：1
                buf.sputc((char)len);
                len = 2;
                return false;
            } else {
                char const * hex_buf = buffer_cast<char const *>(buf.data()) + buf.size() - 1;
                if (*hex_buf == '\n') {
                    hex_buf = buffer_cast<char const *>(buf.data());
                    if (hex_buf == 0) {
                        len = 0;
                        ++hex_buf;
                        while (true) {
                            char h = *hex_buf;
                            if (h >= '0' && h <= '9')
                                h -= '0';
                            else if (h >= 'A' && h <= 'F')
                                h -= ('A' - 10);
                            else if (h >= 'a' && h <= 'f')
                                h -= ('a' - 10);
                            else
                                break;
                            len = (len << 4) | h;
                            ++hex_buf;
                        }
                        if (len == 0) {
                            // 类型2表示最后的trailer
                            buf.reset();
                            buf.sputc((char)2);
                            len = 2;
                            return false;
                        }
                    } else if (*hex_buf == 1) {
                        buf.reset();
                        len = 0;
                    } else if (*hex_buf == 2) {
                        len = buf.size();
                        buf.reset();
                        if (len == 3) {
                            // 不是trailer，已经是最后的CRLF了
                            len = std::size_t(-1);
                        } else {
                            // 是一个trailer，继续接收
                            buf.sputc((char)2);
                            len = 2;
                            return false;
                        }
                    }
                    return true;
                } else if (*hex_buf == '\r') {
                    len = 1;
                    return false;
                } else {
                    len = 2;
                    return false;
                }
            }
        }

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
            struct eof_t {};

            static eof_t eof() { return eof_t(); }

        public:
            // 重写receive，async_receive，read_some，async_receive
            template <typename MutableBufferSequence>
            std::size_t receive(
                const MutableBufferSequence & buffers)
            {
                boost::system::error_code ec;
                std::size_t s = recv(buffers, 0, ec);
                boost::asio::detail::throw_error(ec);
                return s;
            }

            template <typename MutableBufferSequence>
            std::size_t receive(const MutableBufferSequence& buffers, 
                boost::asio::socket_base::message_flags flags)
            {
                boost::system::error_code ec;
                std::size_t s = recv(buffers, flags, ec);
                boost::asio::detail::throw_error(ec);
                return s;
            }

            template <typename MutableBufferSequence>
            std::size_t receive(
                const MutableBufferSequence& buffers, 
                boost::asio::socket_base::message_flags flags, 
                boost::system::error_code& ec)
            {
                std::size_t bytes_recv = 0;
                std::size_t bytes_left = buffers::buffers_size(buffers);
                while (true) {
                    if (rcv_buf_.size()) {
                        // 剩余的Chunk头部或者尾部数据
                        std::size_t bytes_transferred = socket_.receive(rcv_buf_.prepare(rcv_left_), flags, ec);
                        rcv_buf_.consume(bytes_transferred);
                        rcv_left_ -= bytes_transferred;
                        if (rcv_left_) {
                            break;
                        }
                        if (!recv_crlf(rcv_buf_, rcv_left_)) {
                            continue;
                        }
                        if (rcv_left_ == std::size_t(-1)) {
                            ec = boost::asio::error::eof;
                            break;
                        }
                    }
                    if (rcv_left_ == 0) {
                        // 刚刚接收了尾部，重组新的Chunk
                        if (bytes_left == 0)
                            break;
                        recv_crlf(rcv_buf_, rcv_left_);
                    } else {
                        // 接收完头部，接收剩余数据
                        std::size_t bytes_transferred = socket_.receive(
                            buffers::sub_buffers(buffers, bytes_recv, rcv_left_), flags, ec);
                        bytes_recv += bytes_transferred;
                        bytes_left -= bytes_transferred;
                        rcv_left_ -= bytes_transferred;
                        if (rcv_left_ == 0) {
                            // 接收完数据，构建尾部
                            recv_crlf(rcv_buf_, rcv_left_ = 1);
                        }
                        // 只要读到数据就退出
                        break;
                    }
                }
                return bytes_recv;
            }

            template <typename MutableBufferSequence, typename ReadHandler>
            struct recv_handler
            {
                typedef void result_type;

                recv_handler(
                    Source & source, 
                    std::size_t & rcv_left, 
                    boost::asio::streambuf & rcv_buf_, 
                    MutableBufferSequence const & buffers, 
                    ReadHandler handler)
                    : socket_(source)
                    , rcv_left_(rcv_left)
                    , rcv_buf_(rcv_buf_)
                    , buffers_(buffers)
                    , handler_(handler)
                    , bytes_recv_(0)
                    , bytes_left_(buffers::buffers_size(buffers))
                {
                }

                void start()
                {
                    if (rcv_buf_.size()) {
                        socket_.async_receive(rcv_buf_.prepare(rcv_left_), flags_, 
                            boost::bind(boost::ref(*this), _1, _2));
                    } else {
                        if (rcv_left_ == 0) { 
                            if (bytes_left_ == 0) {
                                socket_.get_io_service().post(
                                    boost::bind(handler_, boost::system::error_code(), bytes_recv_));
                                delete this;
                                return;
                            }
                            rcv_left_ = bytes_left_;
                            recv_crlf(rcv_buf_, rcv_left_);
                            socket_.async_receive(buffers::sub_buffers(buffers_, bytes_recv_, bytes_left_), flags_, 
                                boost::bind(boost::ref(*this), _1, _2));
                        } else {
                            socket_.async_receive(buffers::sub_buffers(buffers_, bytes_recv_, bytes_left_), flags_, 
                                boost::bind(boost::ref(*this), _1, _2));
                        }
                    }
                }

                void operator()(
                    boost::system::error_code const & ec, 
                    size_t bytes_transferred)
                {
                    if (ec) {
                        handler_(ec, bytes_recv_);
                        delete this;
                        return;
                    }
                    if (rcv_buf_.size()) {
                        // 剩余的Chunk头部或者尾部数据
                        rcv_buf_.consume(bytes_transferred);
                        rcv_left_ -= bytes_transferred;
                        if (rcv_left_) {
                            handler_(ec, bytes_recv_);
                            delete this;
                            return;
                        }
                        if (!recv_crlf(rcv_buf_, rcv_left_)) {
                            socket_.async_receive(rcv_buf_.prepare(rcv_left_), flags_, 
                                boost::bind(boost::ref(*this), _1, _2));
                            return;
                        }
                        if (rcv_left_ == std::size_t(-1)) {
                            handler_(boost::asio::error::eof, bytes_recv_);
                            delete this;
                            return;
                        }
                        if (rcv_left_ == 0) {
                            // 刚刚接收了尾部，重组新的Chunk
                            if (bytes_left_ == 0) {
                                handler_(ec, bytes_recv_);
                                delete this;
                                return;
                            }
                            recv_crlf(rcv_buf_, rcv_left_);
                            socket_.async_receive(rcv_buf_.prepare(rcv_left_), flags_, 
                                boost::bind(boost::ref(*this), _1, _2));
                        } else {
                            socket_.async_receive(buffers::sub_buffers(buffers_, bytes_recv_, bytes_left_), flags_, 
                                boost::bind(boost::ref(*this), _1, _2));
                        }
                    } else {
                        // 接收完头部，接收剩余数据
                        bytes_recv_ += bytes_transferred;
                        bytes_left_ -= bytes_transferred;
                        rcv_left_ -= bytes_transferred;
                        if (rcv_left_ == 0) {
                            // 接收完数据，构建尾部
                            recv_crlf(rcv_buf_, rcv_left_ = 1);
                        }
                        handler_(ec, bytes_recv_);
                        delete this;
                        return;
                    }
                }

                PASS_DOWN_ASIO_HANDLER_FUNCTION(recv_handler, handler_)

            private:
                Source & socket_;
                std::size_t & rcv_left_;
                boost::asio::streambuf & rcv_buf_;
                MutableBufferSequence buffers_;
                boost::asio::socket_base::message_flags flags_;
                ReadHandler handler_;
                std::size_t bytes_recv_;
                std::size_t bytes_left_;
            };

            template <typename MutableBufferSequence, typename ReadHandler>
            void async_receive(
                const MutableBufferSequence& buffers,
                ReadHandler handler)
            {
                async_receive(buffers, 0, handler);
            }

            template <typename MutableBufferSequence, typename ReadHandler>
            void async_receive(
                const MutableBufferSequence& buffers,
                boost::asio::socket_base::message_flags flags, 
                ReadHandler handler)
            {
                (new recv_handler<ConstBufferSequence, WriteHandler>(
                    socket_, snd_left_, snd_buf_, buffers, flags, handler))->start();
            }

            template <typename MutableBufferSequence>
            std::size_t read_some(
                const MutableBufferSequence& buffers)
            {
                boost::system::error_code ec;
                std::size_t s = recv(buffers, 0, ec);
                boost::asio::detail::throw_error(ec);
                return s;
            }

            template <typename MutableBufferSequence>
            std::size_t read_some(
                const MutableBufferSequence& buffers,
                boost::system::error_code& ec)
            {
                return recv(buffers, 0, ec);
            }

            template <typename MutableBufferSequence, typename ReadHandler>
            void async_receive(
                const MutableBufferSequence& buffers,
                ReadHandler handler)
            {
                async_receive(buffers, 0, handler);
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
                        make_chunk_head(snd_buf_, snd_left_);
                    } else {
                        // 发送完头部，发送剩余数据
                        std::size_t len = socket_.send(buffers::sub_buffers(buffers, bytes_send, snd_left_));
                        bytes_send += len;
                        bytes_left -= len;
                        snd_left_ -= len;
                        if (snd_left_)
                            break;
                        // 发送完数据，构建尾部
                        make_chunk_tail(snd_buf_);
                    }
                }
                return bytes_send;
            }

            std::size_t send(
                const eof_t &, 
                boost::asio::socket_base::message_flags flags, 
                boost::system::error_code & ec)
            {
                assert(snd_left_ == 0);
                if (snd_buf_.size() == 0) {
                    make_chunk_eof(snd_buf_);
                }
                if (snd_buf_.size()) {
                    std::size_t len = socket_.send(snd_buf_.data(), flags, ec);
                    snd_buf_.consume(len);
                    if (snd_buf_.size() == 0) {
                        // 如果再继续send eof，就会失败
                        snd_left_ = (size_t)-1;
                    }
                }
                return 0;
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

            template <typename ConstBufferSequence, typename WriteHandler>
            struct send_handler
            {
                typedef void result_type;

                send_handler(
                    Socket & socket, 
                    std::size_t & snd_left, 
                    boost::asio::streambuf & snd_buf_, 
                    ConstBufferSequence const & buffers, 
                    boost::asio::socket_base::message_flags flags, 
                    WriteHandler handler)
                    : socket_(socket)
                    , snd_left_(snd_left)
                    , snd_buf_(snd_buf_)
                    , buffers_(buffers)
                    , flags_(flags)
                    , handler_(handler)
                    , bytes_send_(0)
                    , bytes_left_(buffers::buffer_size(buffers))
                {
                }

                void start()
                {
                    if (snd_buf_.size()) {
                        socket_.async_send(snd_buf_.data(), flags_,
                            boost::bind(boost::ref(*this)));
                    } else {
                        if (snd_left_ == 0) { 
                            if (bytes_left_ == 0) {
                                socket_.get_io_service().post(
                                    boost::bind(handler_, boost::system::error_code(), bytes_send_));
                                delete this;
                                return;
                            }
                            snd_left_ = bytes_left_;
                            make_chunk_head(snd_buf_, snd_left_);
                            socket_.async_send(buffers::sub_buffers(buffers_, bytes_send_, bytes_left_), flags_, 
                                 boost::bind(boost::ref(*this), _1, _2));
                        } else {
                            socket_.async_send(buffers::sub_buffers(buffers_, bytes_send_, bytes_left_), flags_,
                                 boost::bind(boost::ref(*this), _1, _2));
                        }
                    }
                }

                void operator()(
                    boost::system::error_code const & ec, 
                    size_t bytes_transferred)
                {
                    if (snd_buf_.size()) {
                        // 剩余的Chunk头部或者尾部数据
                        snd_buf_.consume(bytes_transferred);
                        if (snd_buf_.size()) {
                            handler_(ec, bytes_send_);
                            delete this;
                            return;
                        }
                        if (snd_left_ == 0) {
                            // 刚刚发送了尾部，重组新的Chunk
                            if (bytes_left_ == 0) {
                                handler_(ec, bytes_send_);
                                delete this;
                                return;
                            }
                            snd_left_ = bytes_left_;
                            make_chunk_head(snd_buf_, snd_left_);
                            socket_.async_send(snd_buf_.data(), flags_,
                                boost::bind(boost::ref(*this), _1, _2));
                        } else {
                            socket_.async_send(buffers::sub_buffers(buffers_, bytes_send_, bytes_left_), flags_,
                                boost::bind(boost::ref(*this), _1, _2));
                        }
                    } else {
                        // 发送完头部，发送剩余数据
                        bytes_send_ += bytes_transferred;
                        bytes_left_ -= bytes_transferred;
                        snd_left_ -= bytes_transferred;
                        if (snd_left_) {
                            handler_(ec, bytes_send_);
                            delete this;
                            return;
                        }
                        // 发送完数据，构建尾部
                        make_chunk_tail(snd_buf_);
                        socket_.async_send(snd_buf_.data(), flags_,
                            boost::bind(boost::ref(*this), _1, _2));
                    }
                }

                PASS_DOWN_ASIO_HANDLER_FUNCTION(send_handler, handler_)

            private:
                Socket & socket_;
                std::size_t & snd_left_;
                boost::asio::streambuf & snd_buf_;
                ConstBufferSequence buffers_;
                boost::asio::socket_base::message_flags flags_;
                WriteHandler handler_;
                std::size_t bytes_send_;
                std::size_t bytes_left_;
            };

            template <typename WriteHandler>
            class eof_send_handler
            {
                eof_send_handler(
                    boost::asio::streambuf & snd_buf, 
                    WriteHandler handler)
                    : snd_buf_(snd_buf)
                    , handler_(handler)
                {
                }

                void operator()(
                    boost::system::error_code const & ec, 
                    size_t bytes_transferred)
                {
                    snd_buf_.consume(bytes_transferred);
                    handler_(ec, bytes_transferred);
                }

                inline friend void* asio_handler_allocate(
                    std::size_t size,
                    eof_send_handler * this_handler)
                {
                    return boost_asio_handler_alloc_helpers::allocate(
                        size, &this_handler->handler_);
                }

                inline friend void asio_handler_deallocate(
                    void * pointer, 
                    std::size_t size,
                    eof_send_handler * this_handler)
                {
                    boost_asio_handler_alloc_helpers::deallocate(
                        pointer, size, &this_handler->handler_);
                }

                template <typename Function>
                inline friend void asio_handler_invoke(
                    const Function & function,
                    eof_send_handler * this_handler)
                {
                    boost_asio_handler_invoke_helpers::invoke(
                        function, &this_handler->handler_);
                }

            private:
                boost::asio::streambuf & snd_buf_;
                WriteHandler handler_;
            };

            template <typename ConstBufferSequence, typename WriteHandler>
            void async_send(
                const ConstBufferSequence& buffers, 
                WriteHandler handler)
            {
                async_send(buffers, 0, handler);
            }

            template <typename ConstBufferSequence, typename WriteHandler>
            void async_send(
                const ConstBufferSequence& buffers, 
                boost::asio::socket_base::message_flags flags, 
                WriteHandler handler)
            {
                (new send_handler<ConstBufferSequence, WriteHandler>(
                    socket_, snd_left_, snd_buf_, buffers, flags, handler))->start();
            }

            template <typename WriteHandler>
            void async_send(
                const eof_t &, 
                boost::asio::socket_base::message_flags flags, 
                WriteHandler handler)
            {
                assert(snd_left_ == 0);
                if (snd_buf_.size() == 0) {
                    make_chunk_eof(snd_buf_);
                }
                socket_.async_send(snd_buf_.data(), flags,
                    eof_send_handler<WriteHandler>(handler));
            }

            template <typename ConstBufferSequence, typename WriteHandler>
            void async_write_some(
                const ConstBufferSequence& buffers, 
                WriteHandler handler)
            {
                async_send(buffers, 0, handler);
            }

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
