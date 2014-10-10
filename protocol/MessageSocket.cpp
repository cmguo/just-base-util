// MessageSocket.cpp

#include "util/Util.h"
#include "util/protocol/MessageSocket.h"
#include "util/protocol/MessageSocket.hpp"

#include "util/serialization/Array.h"

namespace util
{
    namespace protocol
    {

        MessageSocket::MessageSocket(
            boost::asio::io_service & io_svc, 
            MessageParser & parser, 
            void * ctx)
            : framework::network::TcpSocket(io_svc)
            , parser_(parser)
            , ctx_(ctx)
            , non_block_(false)
            , read_parallel_(false)
            , write_parallel_(false)
        {
        }

        MessageSocket::~MessageSocket()
        {
            boost::system::error_code ec;
            close(ec);
        }

        void MessageSocket::cancel()
        {
            cancel_parallel();
            framework::network::TcpSocket::cancel();
        }

        boost::system::error_code MessageSocket::cancel(
            boost::system::error_code & ec)
        {
            cancel_parallel();
            return framework::network::TcpSocket::cancel(ec);
        }

        void MessageSocket::close()
        {
            cancel_parallel();
            framework::network::TcpSocket::close();
        }

        boost::system::error_code MessageSocket::close(
            boost::system::error_code & ec)
        {
            cancel_parallel();
            return framework::network::TcpSocket::close(ec);
        }

        void MessageSocket::cancel_parallel()
        {
            boost::mutex::scoped_lock lc(mutex_);
            snd_buf_.reset();
            rcv_buf_.reset();
            if (read_parallel_ && !read_status_.resp.empty()) {
                get_io_service().post(
                    boost::bind(read_status_.resp, boost::asio::error::operation_aborted, 0));
                read_status_.resp.clear();
            }
            if (write_parallel_ && !write_status_.resp.empty()) {
                get_io_service().post(
                    boost::bind(write_status_.resp, boost::asio::error::operation_aborted, 0));
                write_status_.resp.clear();
            }
            cond_.notify_all();
        }

        void MessageSocket::set_read_parallel(
            bool b)
        {
            read_parallel_ = b;
        }

        void MessageSocket::set_write_parallel(
            bool b)
        {
            write_parallel_ = b;
        }

    } // namespace protocol
} // namespace util
