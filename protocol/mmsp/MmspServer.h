// MmspServer.h

#ifndef _UTIL_PROTOCOL_MMSP_MMSP_SERVER_H_
#define _UTIL_PROTOCOL_MMSP_MMSP_SERVER_H_

#include "util/protocol/mmsp/MmspSocket.h"
#include "util/protocol/mmsp/MmspMessage.h"

#include <framework/network/ServerManager.h>

#include <boost/function.hpp>

namespace util
{
    namespace protocol
    {

        class MmspServer
            : protected MmspSocket
        {
        public:
            MmspServer(
                boost::asio::io_service & io_svc);

            virtual ~MmspServer();

        protected:
            typedef boost::function<void (
                boost::system::error_code const &
            )> response_type;

        protected:
            MmspMessage & request()
            {
                return request_;
            }

            MmspMessage & response()
            {
                return response_;
            }

        protected:
            virtual void local_process(
                response_type const & resp)
            {
                resp(boost::system::error_code());
            }

            virtual void post_process(
                response_type const & resp)
            {
                resp(boost::system::error_code());
            }

            virtual void on_error(
                boost::system::error_code const & ec)
            {
            }

            virtual void on_finish()
            {
            }

        private:
            template <
                typename Server, 
                typename Manager
            >
            friend class framework::network::ServerManager;

            void start();

        private:
            void handle_prepare(
                boost::system::error_code const & ec, 
                bool proxy);

            void handle_receive_request(
                boost::system::error_code const & ec, 
                size_t bytes_transferred);

            void handle_local_process(
                boost::system::error_code const & ec);

            void handle_send_response(
                boost::system::error_code const & ec, 
                size_t bytes_transferred);

            void handle_error(
                boost::system::error_code const & ec);

            void handle_post_process(
                boost::system::error_code const & ec);

        private:
            size_t id_;
            MmspMessage request_;
            MmspMessage response_;
        };

    } // namespace protocol
} // namespace util

#endif // _UTIL_PROTOCOL_MMSP_MMSP_SERVER_H_
