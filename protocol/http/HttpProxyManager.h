// HttpProxyManager.h

#ifndef _UTIL_PROTOCOL_HTTP_HTTP_PROXY_MANAGER_H_
#define _UTIL_PROTOCOL_HTTP_HTTP_PROXY_MANAGER_H_

#include <framework/network/NetName.h>
#include <framework/network/Acceptor.h>
#include <framework/network/TcpSocket.hpp>

#include <boost/bind.hpp>
#include <boost/asio/ip/tcp.hpp>

namespace util
{
    namespace protocol
    {

        struct DefaultHttpProxyManager;

        template <
            typename HttpProxy, 
            typename Manager = DefaultHttpProxyManager
        >
        class HttpProxyManager
        {
        public:
            HttpProxyManager(
                boost::asio::io_service & io_svc) 
                : acceptor_(io_svc)
                , proxy_(NULL)
            {
            }

            HttpProxyManager(
                boost::asio::io_service & io_svc, 
                framework::network::NetName const & addr)
                : acceptor_(io_svc)
                , addr_(addr)
                , proxy_(NULL)
            {
            }

            void start()
            {
                proxy_ = create(this, (Manager *)NULL);
                proxy_->http_to_client_.async_accept(addr_, acceptor_, 
                    boost::bind(&HttpProxyManager::handle_accept_client, this, _1));
            }

            boost::system::error_code start(
                framework::network::NetName & addr, 
                boost::system::error_code & ec)
            {
                if(std::string::npos == addr.host_svc().find("+"))
                {
                    return start1(addr,ec);
                }
                else
                {
                    framework::network::NetName addrtmp = addr;
                    return start1(addrtmp,addr,ec);
                }
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
            static HttpProxy * create(
                HttpProxyManager * mgr, 
                HttpProxyManager * mgr2)
            {
                return new HttpProxy(static_cast<Manager &>(*mgr));
            }

            static HttpProxy * create(
                HttpProxyManager * mgr, 
                DefaultHttpProxyManager * mgr2)
            {
                return new HttpProxy(mgr->io_svc());
            }

        private:
            void handle_accept_client(
                boost::system::error_code const & ec)
            {
                if (!ec) {
                    proxy_->start();
                    proxy_ = create(this, (Manager *)NULL);
                    proxy_->http_to_client_.async_accept(addr_, acceptor_, 
                        boost::bind(&HttpProxyManager::handle_accept_client, this, _1));
                } else {
                    proxy_->on_error(ec);
                    delete proxy_;
                }
            }

            boost::system::error_code start1(
                framework::network::NetName const & addr, 
                boost::system::error_code & ec)
            {
                if (!framework::network::acceptor_open<boost::asio::ip::tcp>(acceptor_, addr.endpoint(), ec)) {
                    addr_ = addr;
                    proxy_ = create(this, (Manager *)NULL);
                    proxy_->http_to_client_.async_accept(addr_, acceptor_, 
                        boost::bind(&HttpProxyManager::handle_accept_client, this, _1));
                }
                return ec;
            }

            boost::system::error_code start1(
                framework::network::NetName const & addr,
                framework::network::NetName & addr_out, 
                boost::system::error_code & ec)
            {
                addr_out = addr;

                unsigned short iPort= addr_out.port();
                do 
                {
                    ec.clear();
                    start1(addr_out,ec);
                    if(ec)
                    {
                        addr_out.port(++iPort);
                        stop();
                    }
                } while (ec && (iPort-addr.port() < 20));
                return ec;
            }

        private:
            boost::asio::ip::tcp::acceptor acceptor_;
            framework::network::NetName addr_;
            HttpProxy * proxy_;
        };

    } // namespace protocol
} // namespace util

#endif // _UTIL_PROTOCOL_HTTP_HTTP_PROXY_MANAGER_H_
