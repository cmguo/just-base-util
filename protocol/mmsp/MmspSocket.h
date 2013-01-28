// MmspSocket.h

#ifndef _UTIL_PROTOCOL_MMSP_MMSP_SOCKET_H_
#define _UTIL_PROTOCOL_MMSP_MMSP_SOCKET_H_

#include "util/protocol/mmsp/MmspTcpMessage.h"
#include "util/protocol/mmsp/MmspMessage.h"

#include <framework/network/TcpSocket.h>

#include <boost/asio/streambuf.hpp>

namespace util
{
    namespace protocol
    {

        namespace detail
        {
            template <typename Handler>
            struct mmsp_read_tcp_handler;
        }

        class MmspSocket
            : public framework::network::TcpSocket
        {
        public:
            typedef framework::network::TcpSocket super;

        public:
            MmspSocket(
                boost::asio::io_service & io_svc);

        public:
            void close();

            boost::system::error_code close(
                boost::system::error_code & ec);

        public:
            enum MessageClassEnum
            {
                mc_read = 1, 
                mc_write = 2, 
                mc_all = 3,
            };

            template <typename T>
            void register_message(
                MessageClassEnum cls);

        public:
            size_t write(
                MmspMessage const & msg);

            size_t write(
                MmspMessage const & msg, 
                boost::system::error_code & ec);

            template <typename Handler>
            void async_write(
                MmspMessage const & msg, 
                Handler const & handler);

            size_t read(
                MmspMessage & msg);

            size_t read(
                MmspMessage & msg, 
                boost::system::error_code & ec);

            template <typename Handler>
            void async_read(
                MmspMessage & msg, 
                Handler const & handler);

        private:
            template <typename Handler>
            friend struct detail::mmsp_read_tcp_handler;

            size_t write_msg(
                MmspMessage const & msg, 
                boost::system::error_code & ec);

            size_t read_msg(
                MmspMessage & msg, 
                boost::system::error_code & ec);

            template <typename T>
            static void write_msg_s(
                boost::asio::streambuf & buf, 
                MmspMessage const & msg, 
                boost::system::error_code & ec);

            template <typename T>
            static void read_msg_s(
                boost::asio::streambuf & buf, 
                MmspMessage & msg, 
                boost::system::error_code & ec);

        private:
            size_t read_tcp_msg(
                boost::system::error_code & ec);

            size_t write_tcp_msg(
                boost::system::error_code & ec);

            template <typename Handler>
            void async_read_tcp_msg(
                Handler const & handler);

            template <typename Handler>
            void async_write_tcp_msg(
                Handler const & handler);

        private:
            boost::asio::streambuf snd_buf_;
            boost::asio::streambuf rcv_buf_;

            MmspTcpMessageHeader write_tcp_msg_;
            MmspTcpMessageHeader read_tcp_msg_;

            size_t write_tcp_size_;
            size_t read_tcp_size_;

            typedef void (*write_msg_t)(
                boost::asio::streambuf & buf, 
                MmspMessage const & msg, 
                boost::system::error_code & ec);

            typedef void (*read_msg_t)(
                boost::asio::streambuf & buf, 
                MmspMessage & msg, 
                boost::system::error_code & ec);

            std::map<boost::uint32_t, write_msg_t> write_msg_funcs_;
            std::map<boost::uint32_t, read_msg_t> read_msg_funcs_;
        };

    } // namespace protocol
} // namespace util

#endif // _UTIL_PROTOCOL_MMSP_MMSP_SOCKET_H_
