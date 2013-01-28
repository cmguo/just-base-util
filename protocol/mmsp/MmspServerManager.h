// MmspServerManager.h

#ifndef _UTIL_PROTOCOL_MMSP_MMSP_SERVER_MANAGER_H_
#define _UTIL_PROTOCOL_MMSP_MMSP_SERVER_MANAGER_H_

#include <framework/network/NetName.h>
#include <framework/network/Acceptor.h>
#include <framework/network/TcpSocket.hpp>

#include <boost/asio/ip/tcp.hpp>
#include <boost/bind.hpp>

namespace util
{
    namespace protocol
    {

        struct DefaultMmspServerManager;

        template <
            typename MmspServer, 
            typename Manager = DefaultMmspServerManager
        >
        class MmspServerManager
        {
        public:
            MmspServerManager(
                boost::asio::io_service & io_svc) 
                : acceptor_(io_svc)
                , server_(NULL)
            {
            }

            MmspServerManager(
                boost::asio::io_service & io_svc, 
                framework::network::NetName const & addr)
                : acceptor_(io_svc)
                , addr_(addr)
                , server_(NULL)
            {
            }

            void start()
            {
                server_ = create(this, (Manager *)NULL);
                server_->async_accept(addr_, acceptor_, 
                    boost::bind(&MmspServerManager::handle_accept_client, this, _1));
            }

            boost::system::error_code start(
                framework::network::NetName const & addr, 
                boost::system::error_code & ec)
            {
                if (!framework::network::acceptor_open<boost::asio::ip::tcp>(acceptor_, addr.endpoint(), ec)) {
                    addr_ = addr;
                    server_ = create(this, (Manager *)NULL);
                    server_->async_accept(addr_, acceptor_, 
                        boost::bind(&MmspServerManager::handle_accept_client, this, _1));
                }
                return ec;
            }

            void stop()
            {
                boost::system::error_code ec;
                acceptor_.close(ec);
            }

        public:
            boost::asio::io_service & io_svc()
            {
                return acceptor_.get_io_service();
            }

        private:
            static MmspServer * create(
                MmspServerManager * mgr, 
                MmspServerManager * mgr2)
            {
                return new MmspServer(static_cast<Manager &>(*mgr));
            }

            static MmspServer * create(
                MmspServerManager * mgr, 
                DefaultMmspServerManager * mgr2)
            {
                return new MmspServer(mgr->io_svc());
            }

        private:
            void handle_accept_client(
                boost::system::error_code const & ec)
            {
                if (!ec) {
                    server_->start();
                    server_ = create(this, (Manager *)NULL);
                    server_->async_accept(addr_, acceptor_, 
                        boost::bind(&MmspServerManager::handle_accept_client, this, _1));
                } else {
                    server_->on_error(ec);
                    delete server_;
                }
            }

        private:
            boost::asio::ip::tcp::acceptor acceptor_;
            framework::network::NetName addr_;
            MmspServer * server_;
        };

    } // namespace protocol
} // namespace util

#endif // _UTIL_PROTOCOL_MMSP_MMSP_SERVER_MANAGER_H_
