// RtmpServer.h

#ifndef _UTIL_PROTOCOL_RTMP_RTMP_SERVER_H_
#define _UTIL_PROTOCOL_RTMP_RTMP_SERVER_H_

#include "util/protocol/rtmp/RtmpSocket.h"
#include "util/protocol/rtmp/RtmpMessage.h"

#include <framework/network/ServerManager.h>

#include <boost/function.hpp>

namespace util
{
    namespace protocol
    {

        class RtmpServer
            : public RtmpSocket
        {
        public:
            RtmpServer(
                boost::asio::io_service & io_svc);

            virtual ~RtmpServer();

        protected:
            typedef boost::function<void (
                boost::system::error_code const &
            )> response_type;

        protected:
            RtmpMessage & request()
            {
                return request_;
            }

            std::vector<RtmpMessage> & response()
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

        protected:
            size_t id_;
            RtmpMessage request_;
            std::vector<RtmpMessage> response_;

            bool window_send_;
        };

    } // namespace protocol
} // namespace util

#endif // _UTIL_PROTOCOL_RTMP_RTMP_SERVER_H_
