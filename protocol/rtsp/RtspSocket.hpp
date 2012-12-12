// RtspSocket.h

#ifndef _UTIL_PROTOCOL_RTSP_RTSP_SOCKET_HPP_
#define _UTIL_PROTOCOL_RTSP_RTSP_SOCKET_HPP_

#include "util/protocol/rtsp/RtspSocket.h"

#include <framework/network/AsioHandlerHelper.h>

#include <boost/asio/read_until.hpp>
#include <boost/asio/write.hpp>

namespace util
{
    namespace protocol
    {

        namespace rtsp_detail
        {

            template <typename Handler>
            struct receive_handler
            {
                receive_handler(
                    RtspHead & head, 
                    boost::asio::streambuf & buf, 
                    Handler handler)
                    : head_(head)
                    , rcv_buf_(buf)
                    , handler_(handler)
                {
                }

                void operator()(
                    boost::system::error_code const & ec, 
                    size_t bytes_transferred)
                {
                    if (!ec) {
                        size_t old_size = rcv_buf_.size();
                        std::istream is(&rcv_buf_);
                        boost::system::error_code ec1;
                        head_.set_content(is, ec1);
                        if (ec1)
                            handler_(ec1, old_size - rcv_buf_.size());
                        else
                            handler_(ec, old_size - rcv_buf_.size());
                    } else {
                        handler_(ec, 0);
                    }
                }

                PASS_DOWN_ASIO_HANDLER_FUNCTION(receive_handler, handler_)

            private:
                RtspHead & head_;
                boost::asio::streambuf & rcv_buf_;
                Handler handler_;
            };

        } // namespace detail

        template <typename Handler>
        void RtspSocket::async_write(
            RtspHead & head, 
            Handler const & handler)
        {
            std::ostream os(&snd_buf_);
            head.get_content(os);
            boost::asio::async_write((super &)*this, snd_buf_, handler);
        }

        template <typename Handler>
        void RtspSocket::async_read(
            RtspHead & head, 
            Handler const & handler)
        {
            boost::asio::async_read_until((super &)*this, rcv_buf_, "\r\n\r\n", 
                rtsp_detail::receive_handler<Handler>(head, rcv_buf_, handler));
        }

        template <typename MutableBufferSequence>
        std::size_t RtspSocket::receive(
            const MutableBufferSequence & buffers)
        {
            using namespace boost::asio;

            if (rcv_buf_.size() > 0) {
                return copy(buffers);
            } else {
                return super::receive(buffers);
            }
        }

        template <typename MutableBufferSequence>
        std::size_t RtspSocket::receive(const MutableBufferSequence& buffers, 
            socket_base::message_flags flags)
        {
            using namespace boost::asio;

            if (rcv_buf_.size() > 0) {
                return copy(buffers);
            } else {
                return super::receive(buffers, flags);
            }
        }

        template <typename MutableBufferSequence>
        std::size_t RtspSocket::receive(
            const MutableBufferSequence& buffers, 
            socket_base::message_flags flags, 
            boost::system::error_code& ec)
        {
            using namespace boost::asio;

            if (rcv_buf_.size() > 0) {
                ec = boost::system::error_code();
                return copy(buffers);
            } else {
                return super::receive(buffers, flags, ec);
            }
        }

        template <typename MutableBufferSequence, typename ReadHandler>
        void RtspSocket::async_receive(
            const MutableBufferSequence& buffers,
            ReadHandler handler)
        {
            if (rcv_buf_.size() > 0) {
                std::size_t length = copy(buffers);
                get_io_service().post(boost::asio::detail::bind_handler(
                    handler, boost::system::error_code(), length));
            } else {
                super::async_receive(buffers, handler);
            }
        }

        template <typename MutableBufferSequence, typename ReadHandler>
        void RtspSocket::async_receive(
            const MutableBufferSequence& buffers,
            socket_base::message_flags flags, 
            ReadHandler handler)
        {
            if (rcv_buf_.size() > 0) {
                std::size_t length = copy(buffers);
                get_io_service().post(boost::asio::detail::bind_handler(
                    handler, boost::system::error_code(), length));
            } else {
                super::async_receive(buffers, flags, handler);
            }
        }

        template <typename MutableBufferSequence>
        std::size_t RtspSocket::read_some(
            const MutableBufferSequence& buffers)
        {
            using namespace boost::asio;

            if (rcv_buf_.size() > 0) {
                return copy(buffers);
            } else {
                return super::read_some(buffers);
            }
        }

        template <typename MutableBufferSequence>
        std::size_t RtspSocket::read_some(
            const MutableBufferSequence& buffers,
            boost::system::error_code& ec)
        {
            using namespace boost::asio;

            if (rcv_buf_.size() > 0) {
                ec = boost::system::error_code();
                return copy(buffers);
            } else {
                return super::read_some(buffers, ec);
            }
        }

        template <typename MutableBufferSequence, typename ReadHandler>
        void RtspSocket::async_read_some(
            const MutableBufferSequence& buffers,
            ReadHandler handler)
        {
            if (rcv_buf_.size() > 0) {
                std::size_t length = copy(buffers);
                get_io_service().post(boost::asio::detail::bind_handler(
                    handler, boost::system::error_code(), length));
            } else {
                super::async_read_some(buffers, handler);
            }
        }

        /// Start an asynchronous read. The buffer into which the data will be read
        /// must be valid for the lifetime of the asynchronous operation.
        template <typename MutableBufferSequence>
        std::size_t RtspSocket::copy(
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

    } // namespace protocol
} // namespace util

#endif // _UTIL_PROTOCOL_RTSP_RTSP_SOCKET_HPP_
