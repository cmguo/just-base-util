// ChunkedSink.cpp

#include "util/Util.h"
#include "util/stream/ChunkedSink.h"
#include "util/buffers/BuffersSize.h"
#include "util/buffers/SubBuffers.h"

#include <boost/asio/io_service.hpp>
#include <boost/bind.hpp>
#include <boost/ref.hpp>

namespace util
{
    namespace stream
    {

        static char const hex_chr[] = "0123456789ABCDEF";

        void ChunkedSink::make_chunk_head(
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

        void ChunkedSink::make_chunk_tail(
            boost::asio::streambuf & buf)
        {
            using namespace boost::asio;

            char * p = buffer_cast<char *>(buf.prepare(2));
            *p++ = '\r';
            *p++ = '\n';
            buf.commit(2);
        }

        void ChunkedSink::make_chunk_eof(
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

        std::size_t ChunkedSink::private_write_some(
            StreamConstBuffers const & buffers, 
            boost::system::error_code & ec)
        {
            std::size_t bytes_send = 0;
            std::size_t bytes_left = buffers::buffers_size(buffers);
            while (true) {
                if (snd_buf_.size()) {
                    // 剩余的Chunk头部或者尾部数据
                    std::size_t len = sink_.write_some(snd_buf_.data(), ec);
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
                    std::size_t len = sink_.write_some(
                        buffers::sub_buffers(buffers, bytes_send, snd_left_), ec);
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

        std::size_t ChunkedSink::write_eof(
            boost::system::error_code & ec)
        {
            assert(snd_left_ == 0 && snd_buf_.size() == 0);
            if (snd_buf_.size() == 0) {
                make_chunk_eof(snd_buf_);
            }
            boost::asio::write(sink_, snd_buf_, boost::asio::transfer_all(), ec);
            return 0;
        }

        struct ChunkedSink::send_handler
        {
            typedef void result_type;

            send_handler(
                Sink & sink, 
                std::size_t & snd_left, 
                boost::asio::streambuf & snd_buf_, 
                StreamConstBuffers const & buffers, 
                StreamHandler handler)
                : sink_(sink)
                , snd_left_(snd_left)
                , snd_buf_(snd_buf_)
                , buffers_(buffers)
                , handler_(handler)
                , bytes_send_(0)
                , bytes_left_(buffers::buffers_size(buffers))
            {
            }

            void start()
            {
                if (snd_buf_.size()) {
                    sink_.async_write_some(snd_buf_.data(), 
                        boost::bind(boost::ref(*this), _1, _2));
                } else {
                    if (snd_left_ == 0) { 
                        if (bytes_left_ == 0) {
                            sink_.get_io_service().post(
                                boost::bind(handler_, boost::system::error_code(), bytes_send_));
                            delete this;
                            return;
                        }
                        snd_left_ = bytes_left_;
                        ChunkedSink::make_chunk_head(snd_buf_, snd_left_);
                        sink_.async_write_some(snd_buf_.data(), 
                            boost::bind(boost::ref(*this), _1, _2));
                    } else {
                        sink_.async_write_some(buffers::sub_buffers(buffers_, bytes_send_, snd_left_), 
                            boost::bind(boost::ref(*this), _1, _2));
                    }
                }
            }

            void operator()(
                boost::system::error_code const & ec, 
                size_t bytes_transferred)
            {
                if (ec) {
                    handler_(ec, bytes_send_);
                    delete this;
                    return;
                }
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
                        ChunkedSink::make_chunk_head(snd_buf_, snd_left_);
                        sink_.async_write_some(snd_buf_.data(), 
                            boost::bind(boost::ref(*this), _1, _2));
                    } else {
                        sink_.async_write_some(buffers::sub_buffers(buffers_, bytes_send_, snd_left_), 
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
                    ChunkedSink::make_chunk_tail(snd_buf_);
                    sink_.async_write_some(snd_buf_.data(), 
                        boost::bind(boost::ref(*this), _1, _2));
                }
            }

        private:
            Sink & sink_;
            std::size_t & snd_left_;
            boost::asio::streambuf & snd_buf_;
            StreamConstBuffers buffers_;
            StreamHandler handler_;
            std::size_t bytes_send_;
            std::size_t bytes_left_;
        };

        void ChunkedSink::private_async_write_some(
            StreamConstBuffers const & buffers, 
            StreamHandler const & handler)
        {
            (new send_handler(
                sink_, snd_left_, snd_buf_, buffers, handler))->start();
        }

    } // namespace protocol
} // namespace util
