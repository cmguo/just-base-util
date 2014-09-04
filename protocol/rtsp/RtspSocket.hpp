// RtspSocket.h

#ifndef _UTIL_PROTOCOL_RTSP_RTSP_SOCKET_HPP_
#define _UTIL_PROTOCOL_RTSP_RTSP_SOCKET_HPP_

#include "util/protocol/rtsp/RtspSocket.h"
#include "util/protocol/MessageSocket.hpp"

namespace util
{
    namespace protocol
    {

        struct RtspMessageVector
        {
            RtspMessageVector(
                std::vector<RtspMessage> const & msgs)
                : msgs_(msgs)
            {
            }

            void to_data(
                StreamBuffer & buf, 
                void * ctx) const;

        private:
            std::vector<RtspMessage> const & msgs_;
        };

        template <typename Handler>
        void RtspSocket::async_write_msgs(
            std::vector<RtspMessage> const & msgs, 
            Handler const & handler)
        {
            async_write_msg(RtspMessageVector(msgs), handler);
        }

    } // namespace protocol
} // namespace util

#endif // _UTIL_PROTOCOL_RTSP_RTSP_SOCKET_HPP_
