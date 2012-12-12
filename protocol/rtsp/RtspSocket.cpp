// RtspSocket.cpp

#include "util/Util.h"
#include "util/protocol/rtsp/RtspSocket.h"
#include "util/protocol/rtsp/RtspSocket.hpp"

#include <boost/asio/read_until.hpp>
#include <boost/asio/write.hpp>

namespace util
{
    namespace protocol
    {

        RtspSocket::RtspSocket(
            boost::asio::io_service & io_svc)
            : super(io_svc)
        {
        }

        void RtspSocket::close()
        {
            snd_buf_.reset();
            rcv_buf_.reset();
            super::close();
        }

        boost::system::error_code RtspSocket::close(
            boost::system::error_code & ec)
        {
            snd_buf_.reset();
            rcv_buf_.reset();
            return super::close(ec);
        }

        size_t RtspSocket::write(
            RtspHead & head)
        {
            if (snd_buf_.size() == 0) {
                std::ostream os(&snd_buf_);
                head.get_content(os);
            }
            return boost::asio::write((super &)*this, snd_buf_);
        }

        size_t RtspSocket::write(
            RtspHead & head, 
            boost::system::error_code & ec)
        {
            if (snd_buf_.size() == 0) {
                std::ostream os(&snd_buf_);
                head.get_content(os);
            }
            return boost::asio::write((super &)*this, snd_buf_, boost::asio::transfer_all(), ec);
        }

        size_t RtspSocket::read(
            RtspHead & head)
        {
            boost::asio::read_until((super &)*this, rcv_buf_, "\r\n\r\n");
            size_t old_size = rcv_buf_.size();
            std::istream is(&rcv_buf_);
            head.set_content(is);
            return old_size - rcv_buf_.size();
        }

        size_t RtspSocket::read(
            RtspHead & head, 
            boost::system::error_code & ec)
        {
            boost::asio::read_until((super &)*this, rcv_buf_, "\r\n\r\n", ec);
            if (!ec) {
                size_t old_size = rcv_buf_.size();
                std::istream is(&rcv_buf_);
                head.set_content(is);
                return old_size - rcv_buf_.size();
            }
            return 0;
        }

} // namespace protocol
} // namespace util
