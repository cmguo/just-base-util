// RtspMessageData.h

#ifndef _UTIL_PROTOCOL_RTSP_RTSP_MESSAGE_DATA_H_
#define _UTIL_PROTOCOL_RTSP_RTSP_MESSAGE_DATA_H_

#include "util/protocol/rtsp/RtspMessageTraits.h"
#include "util/protocol/MessageData.h"

namespace util
{
    namespace protocol
    {

        template <
            typename T, 
            RtspMessageTraits::id_type id
        >
        struct RtspMessageData
            : MessageData<RtspMessageTraits, T, id>
        {
        };

        struct RtspMessageType
        {
            enum Enum
            {
                NONE, 
                REQUEST, 
                RESPONSE, 
                DATA, 
            };
        };

    } // namespace protocol
} // namespace util

#endif // _UTIL_PROTOCOL_RTSP_RTSP_MESSAGE_DATA_H_
