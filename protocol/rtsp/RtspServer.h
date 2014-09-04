// RtspServer.h

#ifndef _UTIL_PROTOCOL_RTSP_RTSP_SERVER_H_
#define _UTIL_PROTOCOL_RTSP_RTSP_SERVER_H_

#include "util/protocol/rtsp/RtspSocket.h"
#include "util/protocol/rtsp/RtspRequest.h"
#include "util/protocol/rtsp/RtspResponse.h"
#include "util/stream/StreamTransfer.h"

#include <framework/network/ServerManager.h>

#include <boost/function.hpp>

namespace util
{
    namespace protocol
    {

        class RtspServer
            : protected RtspSocket
        {
        public:
            RtspServer(
                boost::asio::io_service & io_svc);

            virtual ~RtspServer();

        protected:
            typedef boost::function<void ( 
                boost::system::error_code const &
            )> response_type;

        protected:
            RtspRequest & request()
            {
                return recv_msg_.as<RtspRequest>();
            }

            RtspResponse & response()
            {
                return send_msgs_[0].as<RtspResponse>();
            }

            void post_request(
                RtspRequest const & request);

        protected:
            virtual void on_start()
            {
            }

            virtual void on_next()
            {
            }

            virtual void local_process_request(
                response_type const & resp)
            {
                resp(boost::system::error_code());
            }

            virtual void local_process_response(
                RtspResponse & response, 
                boost::system::error_code & ec)
            {
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
            void start();

            void next();

        private:
            void handle_prepare(
                boost::system::error_code const & ec, 
                bool proxy);

            void handle_receive_message(
                boost::system::error_code const & ec, 
                size_t bytes_transferred);

            void handle_local_process(
                boost::system::error_code const & ec);

            void handle_send_message(
                boost::system::error_code const & ec, 
                size_t bytes_transferred);

            void handle_response_error(
                boost::system::error_code const & ec, 
                size_t bytes_transferred);

            void handle_post_process(
                boost::system::error_code const & ec);

            void handle_error(
                boost::system::error_code const & ec);

            void response_error(
                boost::system::error_code const & ec);

        private:
            template <
                typename HttpProxy, 
                typename Manager
            >
            friend class framework::network::ServerManager;

            size_t id_;
            RtspMessage recv_msg_;
            std::vector<RtspMessage> send_msgs_;
        };

    } // namespace protocol
} // namespace util

#endif // _UTIL_PROTOCOL_RTSP_RTSP_SERVER_H_
