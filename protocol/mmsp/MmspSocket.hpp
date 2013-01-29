// MmspSocket.hpp

#ifndef _UTIL_PROTOCOL_MMSP_MMSP_SOCKET_HPP_
#define _UTIL_PROTOCOL_MMSP_MMSP_SOCKET_HPP_

#include "util/protocol/mmsp/MmspSocket.h"
#include "util/protocol/mmsp/MmspFormat.h"
#include "util/archive/ArchiveBuffer.h"

#include <framework/network/AsioHandlerHelper.h>

#include <boost/asio/read.hpp>
#include <boost/asio/write.hpp>

namespace util
{
    namespace protocol
    {


        template <typename T>
        void MmspSocket::register_message(
            MessageClassEnum cls)
        {
            if (cls & mc_write) {
                write_msg_funcs_[T::StaticId] = write_msg_s<T>;
            }
            if (cls & mc_read) {
                read_msg_funcs_[T::StaticId] = read_msg_s<T>;
            }
        }

        namespace detail
        {

            template <typename Handler>
            struct mmsp_read_tcp_handler
            {
                mmsp_read_tcp_handler(
                    MmspSocket & sock, 
                    MmspMessage & msg, 
                    Handler handler)
                    : sock_(sock)
                    , msg_(msg)
                    , handler_(handler)
                {
                }

                void operator()(
                    boost::system::error_code ec, 
                    size_t bytes_transferred)
                {
                    if (!ec) {
                        bytes_transferred = sock_.read_msg(msg_, ec);
                    } else {
                        bytes_transferred = 0;
                    }
                    handler_(ec, bytes_transferred);
                }

                PASS_DOWN_ASIO_HANDLER_FUNCTION(mmsp_read_tcp_handler, handler_)

            private:
                MmspSocket & sock_;
                MmspMessage & msg_;
                Handler handler_;
            };

        } // namespace detail

        template <typename Handler>
        void MmspSocket::async_write(
            MmspMessage const & msg, 
            Handler const & handler)
        {
            boost::system::error_code ec;
            size_t bytes_transferred = write_msg(msg, ec);
            if (ec) {
                get_io_service().post(
                    boost::asio::detail::bind_handler(handler, ec, bytes_transferred));
            } else {
                async_write_tcp_msg(
                    boost::bind(handler, _1, bytes_transferred));
            }
        }

        template <typename Handler>
        void MmspSocket::async_read(
            MmspMessage & msg, 
            Handler const & handler)
        {
            if (read_tcp_size_ == 0) {
                async_read_tcp_msg(
                    detail::mmsp_read_tcp_handler<Handler>(*this, msg, handler));
            } else {
                boost::system::error_code ec;
                size_t bytes_transferred = read_msg(msg, ec);
                get_io_service().post(
                    boost::asio::detail::bind_handler(handler, ec, bytes_transferred));
            }
        }

        template <typename T>
        void MmspSocket::write_msg_s(
            boost::asio::streambuf & buf, 
            MmspMessage const & msg, 
            boost::system::error_code & ec)
        {
            util::archive::LittleEndianBinaryOArchive<> oa(buf);
            T const & t = msg.as<T>();
            oa << t;
        }

        template <typename T>
        void MmspSocket::read_msg_s(
            boost::asio::streambuf & buf, 
            MmspMessage & msg, 
            boost::system::error_code & ec)
        {
            util::archive::LittleEndianBinaryIArchive<> ia(buf);
            T & t = msg.get<T>();
            ia >> t;
        }

        namespace detail
        {

            struct mmsp_read_condition
            {
                mmsp_read_condition(
                    boost::asio::streambuf & buf, 
                    MmspTcpMessageHeader & header, 
                    size_t & size)
                    : buf_(buf)
                    , header_(header)
                    , size_(size)
                    , min_size_(MmspTcpMessageHeader::HEAD_SIZE)
                {
                }

                bool operator()(
                    boost::system::error_code const & ec, 
                    size_t bytes_transferred)
                {
                    return ec || check();
                }

            private:
                bool check()
                {
                    if (buf_.size() < min_size_) {
                        return false;
                    }
                    if (min_size_ == MmspTcpMessageHeader::HEAD_SIZE) {
                        char const * buf =
                            boost::asio::buffer_cast<char const *>(buf_.data());
                        boost::uint32_t len;
                        memcpy(&len, buf + 8, 4);
                        len = framework::system::BytesOrder::little_endian_to_host(len);
                        size_ = len - 16;
                        min_size_ = len + 16;
                    }
                    if (buf_.size() < min_size_) {
                        return false;
                    }
                    util::archive::LittleEndianBinaryIArchive<> ia(buf_);
                    ia >> header_;
                    return true;
                }

            private:
                boost::asio::streambuf & buf_;
                MmspTcpMessageHeader & header_;
                size_t & size_;
                size_t min_size_;
            };

            struct mmsp_write_condition
            {
                mmsp_write_condition(
                    boost::asio::streambuf & buf, 
                    MmspTcpMessageHeader & header, 
                    size_t & size)
                    : buf_(buf)
                    , size_(size)
                    , min_size_(MmspTcpMessageHeader::HEAD_SIZE + size)
                {
                    header.messageLength = size + 16;
                    header.chunkCount = size / 8 + 2;
                    ++header.seq;
                    char * cbuf = (char *)
                        boost::asio::buffer_cast<char const *>(buf_.data());
                    util::archive::ArchiveBuffer<> abuf(cbuf, MmspTcpMessageHeader::HEAD_SIZE);
                    util::archive::LittleEndianBinaryOArchive<> oa(abuf);
                    oa << header;
                }

                bool operator()(
                    boost::system::error_code const & ec, 
                    size_t bytes_transferred)
                {
                    return ec || check(bytes_transferred);
                }

            private:
                bool check(
                    size_t bytes_transferred)
                {
                    if (bytes_transferred < min_size_)  {
                        min_size_ -= bytes_transferred;
                        return false;
                    }
                    size_ = 0;
                    return true;
                }

            private:
                boost::asio::streambuf & buf_;
                size_t & size_;
                size_t min_size_;
            };

        } // namespace detail

        template <typename Handler>
        void MmspSocket::async_write_tcp_msg(
            Handler const & handler)
        {
            boost::asio::async_write(
                (super &)*this, 
                snd_buf_, 
                detail::mmsp_write_condition(snd_buf_, write_tcp_msg_, write_tcp_size_), 
                handler);
        }

        template <typename Handler>
        void MmspSocket::async_read_tcp_msg(
            Handler const & handler)
        {
            boost::asio::async_read(
                (super &)*this, 
                rcv_buf_, 
                detail::mmsp_read_condition(rcv_buf_, read_tcp_msg_, read_tcp_size_), 
                handler);
        }

    } // namespace protocol
} // namespace util

#endif // _UTIL_PROTOCOL_MMSP_MMSP_SOCKET_HPP_
