// RtspMessageContext.h

#ifndef _UTIL_PROTOCOL_RTSP_RTSP_MESSAGE_CONTEXT_H_
#define _UTIL_PROTOCOL_RTSP_RTSP_MESSAGE_CONTEXT_H_

namespace util
{
    namespace protocol
    {

        struct RtspMessageContext
        {
            boost::uint8_t read_type;
            boost::uint8_t write_type;

            RtspMessageContext()
                : read_type(0)
                , write_type(0)
            {
            }
        };

    } // namespace protocol
} // namespace util

#endif // _UTIL_PROTOCOL_RTSP_RTSP_MESSAGE_CONTEXT_H_
