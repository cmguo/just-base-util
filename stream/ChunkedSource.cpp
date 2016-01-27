// ChunkedSource.cpp

#include "util/Util.h"
#include "util/stream/ChunkedSource.h"
#include "util/buffers/BuffersSize.h"
#include "util/buffers/SubBuffers.h"

#include <framework/logger/Logger.h>
#include <framework/logger/StreamRecord.h>

#include <boost/asio/io_service.hpp>
#include <boost/bind.hpp>
#include <boost/ref.hpp>

FRAMEWORK_LOGGER_DECLARE_MODULE_LEVEL("util.stream.ChunkedSource", framework::logger::Trace);

namespace util
{
    namespace stream
    {

        /* 
         * ��������һֱ���յ�CRLF
         * ������ݲ���
         *   len ��Ϊ����ȱ�ٵ��ֽ���
         *   ����false
         * ��������㹻
         *   ���buf
         *   �����head���ͣ�len����Ϊchunk���ֽ���
         *     �ر�ģ������ 0 �ֽ�chunk
         *       ���Ͻ������trailer�Ĺ���
         *       ����false
         *   ����true
         */
        bool ChunkedSource::recv_crlf(
            boost::asio::streambuf & buf, 
            std::size_t & len)
        {
            using namespace boost::asio;

            if (buf.size() == 0) {
                // ���bufΪ�գ����һ���ֽڣ���ʾ��ʼ����
                // ����� len ��ʾ���ͣ�head��0��tail��1
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
                            // ����2��ʾ����trailer
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
                            // ����trailer���Ѿ�������CRLF��
                            len = std::size_t(-1);
                        } else {
                            // ��һ��trailer����������
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
                    // ʣ���Chunkͷ������β������
                    std::size_t bytes_transferred = source_.read_some(rcv_buf_.prepare(rcv_left_), ec);
                    rcv_buf_.commit(bytes_transferred);
                    rcv_left_ -= bytes_transferred;
                    if (rcv_left_) {
                        // �ײ�sourceû�з����㹻���ݣ�Ӧ��would_block������������
                        // �˳�ѭ��������
                        break;
                    }
                    if (!recv_crlf(rcv_buf_, rcv_left_)) {
                        // Chunkͷ������β����û�����
                        // ����ѭ����Chunkͷ������β������
                        continue;
                    }
                    if (rcv_left_ == std::size_t(-1)) {
                        // �Ѿ���������Chunk
                        // �˳�ѭ��������eof
                        ec = boost::asio::error::eof;
                        break;
                    }
                }
                // ���ˣ�Chunkͷ������β�����꣬����rcv_left_�Ƿ�Ϊ0�����ж���ͷ������β��
                if (rcv_left_ == 0) {
                    // �ոս�����β���������µ�Chunk
                    if (bytes_left == 0)
                        break;
                    // ��û�дﵽ������������������
                    // ����ѭ����Chunkͷ��
                    recv_crlf(rcv_buf_, rcv_left_);
                } else {
                    // ������ͷ��������ʣ������
                    std::size_t bytes_transferred = source_.read_some(
                        buffers::sub_buffers(buffers, bytes_recv, rcv_left_), ec);
                    bytes_recv += bytes_transferred;
                    bytes_left -= bytes_transferred;
                    rcv_left_ -= bytes_transferred;
                    if (rcv_left_ == 0) {
                        // ���������ݣ�����β��
                        recv_crlf(rcv_buf_, rcv_left_ = 1);
                    }
                    // ֻҪ�������ݾ��˳�
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
                //LOG_TRACE("[recv_handler::start]");
                if (rcv_buf_.size()) {
                    //LOG_TRACE("[recv_handler::start] recv chunk, rcv_left: " << rcv_left_);
                    source_.async_read_some(rcv_buf_.prepare(rcv_left_), 
                        boost::bind(boost::ref(*this), _1, _2));
                } else {
                    if (rcv_left_ == 0) { 
                        if (bytes_left_ == 0) {
                            //LOG_TRACE("[recv_handler::start] sudden finish");
                            source_.get_io_service().post(
                                boost::bind(handler_, boost::system::error_code(), bytes_recv_));
                            delete this;
                            return;
                        }
                        ChunkedSource::recv_crlf(rcv_buf_, rcv_left_);
                        //LOG_TRACE("[recv_handler::start] recv chunk, rcv_left: " << rcv_left_);
                        source_.async_read_some(rcv_buf_.prepare(rcv_left_), 
                            boost::bind(boost::ref(*this), _1, _2));
                    } else {
                        //LOG_TRACE("[recv_handler::start] recv data, rcv_left: " << rcv_left_);
                        source_.async_read_some(buffers::sub_buffers(buffers_, bytes_recv_, rcv_left_), 
                            boost::bind(boost::ref(*this), _1, _2));
                    }
                }
            }

            void operator()(
                boost::system::error_code const & ec, 
                size_t bytes_transferred)
            {
                //LOG_TRACE("[recv_handler::operator()] bytes_transferred: " << bytes_transferred);
                if (ec) {
                    //LOG_TRACE("[recv_handler::operator()] callback, bytes_recv: " << bytes_recv_);
                    handler_(ec, bytes_recv_);
                    delete this;
                    return;
                }
                if (rcv_buf_.size()) {
                    // ʣ���Chunkͷ������β������
                    //LOG_TRACE("[recv_handler::operator()] commit chunk");
                    rcv_buf_.commit(bytes_transferred);
                    rcv_left_ -= bytes_transferred;
                    if (rcv_left_) {
                        //LOG_TRACE("[recv_handler::operator()] callback, bytes_recv: " << bytes_recv_);
                        handler_(ec, bytes_recv_);
                        delete this;
                        return;
                    }
                    if (!ChunkedSource::recv_crlf(rcv_buf_, rcv_left_)) {
                        //LOG_TRACE("[recv_handler::operator()] recv chunk, rcv_left: " << rcv_left_);
                        source_.async_read_some(rcv_buf_.prepare(rcv_left_), 
                            boost::bind(boost::ref(*this), _1, _2));
                        return;
                    }
                    if (rcv_left_ == std::size_t(-1)) {
                        //LOG_TRACE("[recv_handler::operator()] eof");
                        rcv_left_ = 0; // ����Ϊ��ʼ״̬��Ϊ�����Ľ�����׼��
                        //LOG_TRACE("[recv_handler::operator()] callback, bytes_recv: " << bytes_recv_);
                        handler_(boost::asio::error::eof, bytes_recv_);
                        delete this;
                        return;
                    }
                    if (rcv_left_ == 0) {
                        // �ոս�����β���������µ�Chunk
                        if (bytes_left_ == 0) {
                            //LOG_TRACE("[recv_handler::operator()] callback, bytes_recv: " << bytes_recv_);
                            handler_(ec, bytes_recv_);
                            delete this;
                            return;
                        }
                        //LOG_TRACE("[recv_handler::operator()] recv chunk, rcv_left: " << rcv_left_);
                        ChunkedSource::recv_crlf(rcv_buf_, rcv_left_);
                        source_.async_read_some(rcv_buf_.prepare(rcv_left_), 
                            boost::bind(boost::ref(*this), _1, _2));
                    } else {
                        //LOG_TRACE("[recv_handler::operator()] recv data, rcv_left: " << rcv_left_);
                        source_.async_read_some(buffers::sub_buffers(buffers_, bytes_recv_, rcv_left_), 
                            boost::bind(boost::ref(*this), _1, _2));
                    }
                } else {
                    //LOG_TRACE("[recv_handler::operator()] commit data");
                    // ������ͷ��������ʣ������
                    bytes_recv_ += bytes_transferred;
                    bytes_left_ -= bytes_transferred;
                    rcv_left_ -= bytes_transferred;
                    if (rcv_left_ == 0) {
                        // ���������ݣ�����β��
                        ChunkedSource::recv_crlf(rcv_buf_, rcv_left_ = 1);
                    }
                    //LOG_TRACE("[recv_handler::operator()] callback, bytes_recv: " << bytes_recv_);
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
