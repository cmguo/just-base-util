// RtpUdpSocket.cpp

#include "util/buffers/BuffersCopy.h"
#include "util/buffers/SubBuffers.h"

#include <framework/thread/MessageQueue.h>

#include <boost/thread/thread.hpp>
#include <boost/thread/condition.hpp>
#include <boost/bind.hpp>

using namespace util::buffers;

namespace util
{
    namespace protocol
    {

        struct RtpUdpSocket::Socket
            : boost::asio::ip::udp::socket
        {
            Socket(
                boost::asio::io_service & io_svc)
                : boost::asio::ip::udp::socket(io_svc)
                , next_ready(NULL)
            {
            }

            boost::uint8_t index;
            Socket * next_ready;
        };

        class RtpUdpSocket::handler_wrapper
        {
        public:
            handler_wrapper(
                boost::asio::io_service & io_svc, 
                StreamHandler const handler)
                : io_svc_(io_svc)
                , handler_(handler)
            {
            }

            void operator()(
                boost::system::error_code const & ec, 
                size_t bytes_transferred) const
            {
                if (bytes_transferred)
                    bytes_transferred += 4;
                io_svc_.post(boost::asio::detail::bind_handler(
                        handler_, ec, bytes_transferred));
            }

        private:
            boost::asio::io_service & io_svc_;
            StreamHandler const handler_;
        };

        class RtpUdpSocket::ReadHandler
        {
        public:
            struct noop_deleter { void operator()(void*) {} };

            ReadHandler()
                : cancel_token_(static_cast<void*>(0), noop_deleter())
                , ready_sockets_(NULL)
                , ready_sockets_tail_(NULL)
                , non_block_(false)
            {
            }

            ~ReadHandler()
            {
                cancel_token_.reset(static_cast<void*>(0), noop_deleter());
            }

        public:
            void set_non_block(
                bool non_block)
            {
                non_block_ = non_block;
            }

            void add_socket(
                Socket * socket)
            {
                poll_read(socket);
            }

            struct Assign
            {
                Assign(
                    boost::system::error_code & ec, 
                    size_t & bytes_transferred)
                    : ec_(ec)
                    , bytes_transferred_(bytes_transferred)
                {
                }

                void operator()(
                    boost::system::error_code const & ec, 
                    size_t bytes_transferred) const
                {
                    ec_ = ec;
                    bytes_transferred_ = bytes_transferred;
                }

                boost::system::error_code & ec_;
                size_t & bytes_transferred_;
            };

            std::size_t read_some(
                StreamMutableBuffers const & buffers,
                boost::system::error_code & ec)
            {
                size_t bytes_transferred = 0;
                ReadTask task;
                task.buffers = buffers;
                task.handler = StreamHandler(Assign(ec, bytes_transferred));
                boost::mutex::scoped_lock(mutex_);
                if (ready_sockets_ && handle_task(task, ec)) {
                    return bytes_transferred;
                }
                if (non_block_) {
                    ec = boost::asio::error::would_block;
                    return 0;
                }
                tasks_.push_back(task);
                while (task.handler.shared_count() > 1) {
                    cond_.wait(mutex_);
                }
                return bytes_transferred;
            }

            void async_read_some(
                StreamMutableBuffers const & buffers, 
                StreamHandler const & handler)
            {
                ReadTask task;
                task.buffers = buffers;
                task.handler = handler;
                boost::mutex::scoped_lock(mutex_);
                boost::system::error_code ec;
                if (ready_sockets_ && handle_task(task, ec))
                    return;
                tasks_.push_back(task);
            }

        private:
            struct ReadTask
            {
                StreamMutableBuffers buffers;
                StreamHandler handler;
            };

        private:
            // in lock
            bool handle_task(
                ReadTask const & task, 
                boost::system::error_code & ec)
            {
                while (ready_sockets_) {
                    size_t bytes_transferred = ready_sockets_->receive(
                        sub_buffers(task.buffers, 4), 0, ec);
                    if (bytes_transferred) {
                        //std::cout << "[RtpUdpSocket::ReadHandler::handle_task] socket: " << ready_sockets_ << " bytes_transferred: " << bytes_transferred << std::endl;
                        boost::uint8_t head_[] = {'$', ready_sockets_->index,
                            boost::uint8_t(bytes_transferred >> 8),
                            boost::uint8_t(bytes_transferred)
                        };
                        buffers_copy(task.buffers, boost::asio::buffer(head_));
                        bytes_transferred += 4;       
                        task.handler(ec, bytes_transferred);
                        return true;
                    } else {
                        //std::cerr << "[RtpUdpSocket::ReadHandler::handle_task] socket: " << ready_sockets_ << " ec: " << ec.message() << std::endl;
                        Socket * socket = ready_sockets_;
                        ready_sockets_ = ready_sockets_->next_ready;
                        socket->next_ready = NULL;
                        poll_read(socket);
                    }
                }
                return false;
            }

            void poll_read(
                Socket * socket)
            {
                //std::cout << "[RtpUdpSocket::ReadHandler::poll_read] socket: " << socket << std::endl;
                socket->async_receive(
                    boost::asio::null_buffers(), 0,
                    boost::bind(&ReadHandler::on_read, this, boost::weak_ptr<void>(cancel_token_), _1, socket));
            }

            void on_read(
                boost::weak_ptr<void> const & cancel_token, 
                boost::system::error_code ec, 
                Socket * socket)
            {
                //std::cout << "[RtpUdpSocket::ReadHandler::on_read] socket: " << socket << std::endl;

                if (cancel_token.expired()) {
                    return;
                }

                if (ec) {
                    std::cerr << "[RtpUdpSocket::ReadHandler::on_read] ec: " << ec.message() << std::endl;
                    return;
                }

                boost::mutex::scoped_lock(mutex_);
                assert(socket->next_ready == NULL);
                if (ready_sockets_) {
                    ready_sockets_tail_->next_ready = socket;
                    ready_sockets_tail_ = socket;
                } else {
                    ready_sockets_ = ready_sockets_tail_ = socket;
                }

                while (!tasks_.empty() && ready_sockets_) {
                    if (handle_task(tasks_.front(), ec)) {
                        tasks_.pop_front();
                    } else {
                        break;
                    }
                }

                cond_.notify_all();
            }

        private:
            boost::shared_ptr<void> cancel_token_;
            Socket * ready_sockets_;
            Socket * ready_sockets_tail_;
            std::deque<ReadTask> tasks_;
            boost::mutex mutex_;
            boost::condition cond_;
            bool non_block_;
        };

    } // namespace protocol
} // namespace util
