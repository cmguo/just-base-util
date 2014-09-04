// RtspMessage.h

#ifndef _UTIL_PROTOCOL_RTSP_RTSP_MESSAGE_H_
#define _UTIL_PROTOCOL_RTSP_RTSP_MESSAGE_H_

#include "util/protocol/rtsp/RtspMessageTraits.h"
#include "util/protocol/Message.h"

namespace util
{
    namespace protocol
    {

        class RtspMessageHeader
        {
        public:
            boost::uint32_t type;

        public:
            RtspMessageHeader(
                boost::uint32_t id = 0)
                : type(id)
            {
            }

            boost::uint32_t id() const
            {
                return type;
            }

            void id(
                boost::uint32_t n)
            {
                type = n;
            }

            template <typename Archive>
            void serialize(
                Archive & ar)
            {
            }
        };

        typedef Message<RtspMessageTraits> RtspMessage;

    } // namespace protocol
} // namespace util

#endif // _UTIL_PROTOCOL_RTSP_RTSP_MESSAGE_H_
