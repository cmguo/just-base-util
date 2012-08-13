// Chunked.cpp

#include "util/Util.h"
#include "util/stream/Chunked.h"
#include "util/buffers/BuffersSize.h"
#include "util/buffers/SubBuffers.h"

#include <boost/asio/socket_base.hpp>
#include <boost/bind.hpp>
#include <boost/ref.hpp>

namespace util
{
    namespace protocol
    {

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

            // 重写receive，async_receive，read_some，async_read_some
            template <typename MutableBufferSequence>
            std::size_t receive(
                const MutableBufferSequence & buffers)
            {
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

            std::size_t Chunked::private_write_some(
                StreamConstBuffers const & buffers, 
                boost::system::error_code & ec)
            {
                std::size_t bytes_send = 0;
                std::size_t bytes_left = buffers::buffers_size(buffers);
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

            template <typename ConstBufferSequence, typename WriteHandler>
            struct send_handler
            {
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
                                 boost::bind(boost::ref(*this)));
                        } else {
                            socket_.async_send(buffers::sub_buffers(buffers_, bytes_send_, bytes_left_), flags_,
                                 boost::bind(boost::ref(*this)));
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
                                boost::bind(boost::ref(*this)));
                        } else {
                            socket_.async_send(buffers::sub_buffers(buffers_, bytes_send_, bytes_left_), flags_,
                                boost::bind(boost::ref(*this)));
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
                            boost::bind(boost::ref(*this)));
                    }
                }

                inline friend void* asio_handler_allocate(
                    std::size_t size,
                    send_handler * this_handler)
                {
                    return boost_asio_handler_alloc_helpers::allocate(
                        size, &this_handler->handler_);
                }

                inline friend void asio_handler_deallocate(
                    void * pointer, 
                    std::size_t size,
                    send_handler * this_handler)
                {
                    boost_asio_handler_alloc_helpers::deallocate(
                        pointer, size, &this_handler->handler_);
                }

                template <typename Function>
                inline friend void asio_handler_invoke(
                    const Function & function,
                    send_handler * this_handler)
                {
                    boost_asio_handler_invoke_helpers::invoke(
                        function, &this_handler->handler_);
                }

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

        virtual std::size_t private_read_some(
            StreamMutableBuffers const & buffers,
            boost::system::error_code & ec)
        {
            return socket_.read_some(buffers, ec);
        }

        virtual void private_async_read_some(
            StreamMutableBuffers const & buffers, 
            StreamHandler const & handler)
        {
            socket_.async_read_some(buffers, handler);
        }

        virtual std::size_t private_write_some(
            StreamConstBuffers const & buffers, 
            boost::system::error_code & ec)
        {
            return socket_.write_some(buffers, ec);
        }

        virtual void private_async_write_some(
            StreamConstBuffers const & buffers, 
            StreamHandler const & handler)
        {
            socket_.async_write_some(buffers, handler);
        }

    } // namespace protocol
} // namespace util

#endif // _UTIL_STREAM_CHUNKED_H_
