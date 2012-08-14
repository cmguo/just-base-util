// ChunkedSource.cpp

#include "util/Util.h"
#include "util/stream/ChunkedSource.h"
#include "util/buffers/BuffersSize.h"
#include "util/buffers/SubBuffers.h"

#include <boost/asio/io_service.hpp>
#include <boost/bind.hpp>
#include <boost/ref.hpp>

namespace util
{
    namespace stream
    {

        /* 
         * 接收数据一直到收到CRLF
         * 如果数据不足
         *   len 设为至少缺少的字节数
         *   返回false
         * 如果数据足够
         *   清空buf
         *   如果是head类型，len设置为chunk的字节数
         *     特别的，如果是 0 字节chunk
         *       马上进入接收trailer的过程
         *       返回false
         *   返回true
         */
        bool ChunkedSource::recv_crlf(
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
                    if (*hex_buf == 0) {
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
                        buf.reset();
                        if (len == 0) {
                            // 类型2表示最后的trailer
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

        std::size_t ChunkedSource::private_read_some(
            StreamMutableBuffers const & buffers, 
            boost::system::error_code & ec)
        {
            std::size_t bytes_recv = 0;
            std::size_t bytes_left = buffers::buffers_size(buffers);
            while (true) {
                if (rcv_buf_.size()) {
                    // 剩余的Chunk头部或者尾部数据
                    std::size_t bytes_transferred = source_.read_some(rcv_buf_.prepare(rcv_left_), ec);
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
                    std::size_t bytes_transferred = source_.read_some(
                        buffers::sub_buffers(buffers, bytes_recv, rcv_left_), ec);
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

        struct ChunkedSource::recv_handler
        {
            typedef void result_type;

            recv_handler(
                Source & source, 
                std::size_t & rcv_left, 
                boost::asio::streambuf & rcv_buf_, 
                StreamMutableBuffers const & buffers, 
                StreamHandler handler)
                : source_(source)
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
                    source_.async_read_some(rcv_buf_.prepare(rcv_left_), 
                        boost::bind(boost::ref(*this), _1, _2));
                } else {
                    if (rcv_left_ == 0) { 
                        if (bytes_left_ == 0) {
                            source_.get_io_service().post(
                                boost::bind(handler_, boost::system::error_code(), bytes_recv_));
                            delete this;
                            return;
                        }
                        ChunkedSource::recv_crlf(rcv_buf_, rcv_left_);
                        source_.async_read_some(rcv_buf_.prepare(rcv_left_), 
                            boost::bind(boost::ref(*this), _1, _2));
                    } else {
                        source_.async_read_some(buffers::sub_buffers(buffers_, bytes_recv_, rcv_left_), 
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
                    if (!ChunkedSource::recv_crlf(rcv_buf_, rcv_left_)) {
                        source_.async_read_some(rcv_buf_.prepare(rcv_left_), 
                            boost::bind(boost::ref(*this), _1, _2));
                        return;
                    }
                    if (rcv_left_ == std::size_t(-1)) {
                        rcv_left_ = 0; // 重置为初始状态，为后续的接受做准备
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
                        ChunkedSource::recv_crlf(rcv_buf_, rcv_left_);
                        source_.async_read_some(rcv_buf_.prepare(rcv_left_), 
                            boost::bind(boost::ref(*this), _1, _2));
                    } else {
                        source_.async_read_some(buffers::sub_buffers(buffers_, bytes_recv_, rcv_left_), 
                            boost::bind(boost::ref(*this), _1, _2));
                    }
                } else {
                    // 接收完头部，接收剩余数据
                    bytes_recv_ += bytes_transferred;
                    bytes_left_ -= bytes_transferred;
                    rcv_left_ -= bytes_transferred;
                    if (rcv_left_ == 0) {
                        // 接收完数据，构建尾部
                        ChunkedSource::recv_crlf(rcv_buf_, rcv_left_ = 1);
                    }
                    handler_(ec, bytes_recv_);
                    delete this;
                    return;
                }
            }

        private:
            Source & source_;
            std::size_t & rcv_left_;
            boost::asio::streambuf & rcv_buf_;
            StreamMutableBuffers buffers_;
            StreamHandler handler_;
            std::size_t bytes_recv_;
            std::size_t bytes_left_;
        };

        void ChunkedSource::private_async_read_some(
            StreamMutableBuffers const & buffers, 
            StreamHandler const & handler)
        {
            (new recv_handler(
                source_, rcv_left_, rcv_buf_, buffers, handler))->start();
        }

        std::size_t ChunkedSource::read_eof(
            boost::system::error_code & ec)
        {
            boost::asio::streambuf buf;
            return boost::asio::read(*this, buf, boost::asio::transfer_all(), ec);
        }

    } // namespace protocol
} // namespace util
