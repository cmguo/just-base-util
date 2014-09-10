// RtspSession.h

#ifndef _UTIL_PROTOCOL_RTSP_RTSP_SESSION_H_
#define _UTIL_PROTOCOL_RTSP_RTSP_SESSION_H_

#include "util/protocol/rtsp/RtspSocket.h"
#include "util/protocol/rtsp/RtspRequest.h"
#include "util/protocol/rtsp/RtspResponse.h"

namespace util
{
    namespace protocol
    {

        class RtspSession
            : protected RtspSocket
        {
        public:
            RtspSession(
                boost::asio::io_service & io_svc);

            virtual ~RtspSession();

        protected:
            void start();

            void post(
                RtspRequest & request);

            void post(
                RtspResponse & response);

            void fail(
                boost::system::error_code const & ec);

            // call this when finish session
            void stop();

        protected:
            virtual void on_start()
            {
            }

            virtual void on_next()
            {
            }

            virtual void on_recv(
                RtspRequest const & request)
            {
            }

            virtual void on_recv(
                RtspResponse const & response)
            {
            }

            virtual void on_error(
                boost::system::error_code const & ec)
            {
            }

            virtual void on_sent(
                RtspRequest const & request)
            {
            }

            virtual void on_sent(
                RtspResponse const & response)
            {
            }

            virtual void on_stop()
            {
            }

        private:
            void post(
                RtspMessage const & msg);

            void recv_next();

            void send_next();

            void release();

        private:
            void handle_receive_message(
                boost::system::error_code const & ec, 
                size_t bytes_transferred);

            void handle_send_message(
                boost::system::error_code const & ec, 
                size_t bytes_transferred);

            void handle_response_error(
                boost::system::error_code const & ec, 
                size_t bytes_transferred);

            void handle_error(
                boost::system::error_code const & ec);

            void response_error(
                boost::system::error_code const & ec);

        private:
            size_t id_;
            size_t seq_;
            bool stopping_;
            std::deque<RtspRequest> requests_;
            RtspMessage recv_msg_;
            std::deque<RtspMessage> send_msgs_;
        };

    } // namespace protocol
} // namespace util

#endif // _UTIL_PROTOCOL_RTSP_RTSP_SESSION_H_
