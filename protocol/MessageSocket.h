// MessageSocket.h

#ifndef _UTIL_PROTOCOL_MESSAGE_SOCKET_H_
#define _UTIL_PROTOCOL_MESSAGE_SOCKET_H_

#include <framework/network/TcpSocket.h>

#include <boost/asio/streambuf.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/condition_variable.hpp>

namespace util
{
    namespace protocol
    {

        class MessageParser;

        namespace detail
        {

            template <
                typename MutableBufferSequence, 
                typename Handler
            >
            struct raw_msg_read_handler;

            template <
                typename ConstBufferSequence, 
                typename Handler
            >
            struct raw_msg_write_handler;

            template <
                typename Message, 
                typename Handler
            >
            struct msg_read_handler;

            template <
                typename Message, 
                typename Handler
            >
            struct msg_write_handler;

        } // namespace detail

        class MessageSocket
            : public framework::network::TcpSocket
        {
        public:
            MessageSocket(
                boost::asio::io_service & io_svc, 
                MessageParser & parser);

            ~MessageSocket();

        public:
            void set_read_parallel(
                bool b);

            void set_write_parallel(
                bool b);

        public:
            void close();

            boost::system::error_code close(
                boost::system::error_code & ec);

        public:
            template <
                typename MutableBufferSequence
            >
            size_t read_raw_msg(
                MutableBufferSequence const & buffers, 
                boost::system::error_code & ec);

            template <
                typename MutableBufferSequence, 
                typename Handler
            >
            void async_read_raw_msg(
                MutableBufferSequence const & buffers, 
                Handler const & handler);

        public:
            template <
                typename ConstBufferSequence
            >
            size_t write_raw_msg(
                ConstBufferSequence const & buffers, 
                boost::system::error_code & ec);

            template <
                typename ConstBufferSequence, 
                typename Handler
            >
            void async_write_raw_msg(
                ConstBufferSequence const & buffers, 
                Handler const & handler);

        public:
            template <
                typename Message
            >
            size_t read_msg(
                Message const & msg, 
                boost::system::error_code & ec);

            template <
                typename Message, 
                typename Handler
            >
            void async_read_msg(
                Message & msg, 
                Handler const & handler);

        public:
            template <
                typename Message
            >
            size_t write_msg(
                Message const & msg, 
                boost::system::error_code & ec);

            template <
                typename Message, 
                typename Handler
            >
            void async_write_msg(
                Message const & msg, 
                Handler const & handler);

        private:
            template <
                typename MutableBufferSequence, 
                typename Handler
            >
            friend struct detail::raw_msg_read_handler;

            template <
                typename ConstBufferSequence, 
                typename Handler
            >
            friend struct detail::raw_msg_write_handler;

            template <
                typename Message, 
                typename Handler
            >
            friend struct detail::msg_read_handler;

            template <
                typename Message, 
                typename Handler
            >
            friend struct detail::msg_write_handler;

            template <
                typename MutableBufferSequence, 
                typename Handler
            >
            void handle_read_raw_msg(
                MutableBufferSequence const & buffers, 
                Handler const & handler, 
                boost::system::error_code ec, 
                size_t bytes_transferred);

            template <
                typename ConstBufferSequence, 
                typename Handler
            >
            void handle_write_raw_msg(
                ConstBufferSequence const & buffers, 
                Handler const & handler, 
                boost::system::error_code ec, 
                size_t bytes_transferred);

            template <
                typename Message, 
                typename Handler
            >
            void handle_read_msg(
                Message & msg, 
                Handler const & handler, 
                boost::system::error_code ec, 
                size_t bytes_transferred);

            template <
                typename Message, 
                typename Handler
            >
            void handle_write_msg(
                Message const & msg, 
                Handler const & handler, 
                boost::system::error_code ec, 
                size_t bytes_transferred);

        private:
            MessageParser & parser_;

            typedef boost::function<void (
                boost::system::error_code ec, 
                size_t bytes_transferred
                )> wait_resp_t;

            struct MessageStatus
            {
                size_t size;
                size_t pos;
                size_t wait;
                wait_resp_t resp;

                size_t left() const
                {
                    return size - pos;
                }
            };

        protected:
            boost::mutex mutex_;
            boost::condition_variable cond_;

            boost::asio::streambuf snd_buf_;
            boost::asio::streambuf rcv_buf_;
            std::deque<size_t> pend_rcv_sizes_;

        private:
            MessageStatus read_status_;
            MessageStatus write_status_;

            bool non_block_;
            bool read_parallel_;
            bool write_parallel_;
        };

    } // namespace protocol
} // namespace util

#endif // _UTIL_PROTOCOL_MESSAGE_SOCKET_H_
