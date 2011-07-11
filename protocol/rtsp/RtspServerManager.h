// RtspServerManager.h

#ifndef _UTIL_PROTOCOL_RTSP_server_MANAGER_H_
#define _UTIL_PROTOCOL_RTSP_server_MANAGER_H_

#include <boost/bind.hpp>

#include <framework/network/NetName.h>

#include <boost/asio/ip/tcp.hpp>

namespace util
{
    namespace protocol
    {

        template <typename RtspSERVER>
        class RtspServerManager
        {
        public:
            RtspServerManager(
                boost::asio::io_service & io_svc, 
                framework::network::NetName const & addr)
                : acceptor_(io_svc)
                , addr_(addr)
                , server_(NULL)
            {
            }

            void start()
            {
                server_ = new RtspSERVER(acceptor_.get_io_service());
                server_->async_accept(addr_, acceptor_, 
                    boost::bind(&RtspServerManager::handle_accept_client, this, _1));
            }

        private:
            void handle_accept_client(
                boost::system::error_code const & ec)
            {
                if (!ec) {
                    server_->start();
                    server_ = new RtspSERVER(acceptor_.get_io_service());
                    server_->async_accept(addr_, acceptor_, 
						boost::bind(&RtspServerManager::handle_accept_client, this, _1));
                } else {
                    server_->on_error(ec);
                    delete server_;
                }
            }

        private:
            boost::asio::ip::tcp::acceptor acceptor_;
            framework::network::NetName addr_;
            RtspSERVER * server_;
        };

    } // namespace protocol
} // namespace util

#endif // _UTIL_PROTOCOL_RTSP_server_MANAGER_H_
