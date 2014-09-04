// RtspSocket.h

#ifndef _UTIL_PROTOCOL_RTSP_RTSP_SOCKET_H_
#define _UTIL_PROTOCOL_RTSP_RTSP_SOCKET_H_

#include "util/protocol/rtsp/RtspMessage.h"
#include "util/protocol/rtsp/RtspMessageParser.h"
#include "util/protocol/MessageSocket.h"

namespace util
{
    namespace protocol
    {

        class RtspSocket
            : public MessageSocket
        {
        public:
            RtspSocket(
                boost::asio::io_service & io_svc);

        public:
            size_t write_msgs(
                std::vector<RtspMessage> const & msgs, 
                boost::system::error_code & ec);

            template <typename Handler>
            void async_write_msgs(
                std::vector<RtspMessage> const & msgs, 
                Handler const & handler);

        private:
            RtspMessageParser read_parser_;
        };

    } // namespace protocol
} // namespace util

#endif // _UTIL_PROTOCOL_RTSP_RTSP_SOCKET_H_
