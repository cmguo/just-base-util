// RtspPacket.h

#ifndef _UTIL_PROTOCOL_RTSP_RTSP_PACKET_H_
#define _UTIL_PROTOCOL_RTSP_RTSP_PACKET_H_

#include "util/protocol/rtsp/RtspFields.h"

#include "util/protocol/cmsg/CMsgPacket.h"
#include "util/protocol/cmsg/CMsgProtocol.h"

namespace util
{
    namespace protocol
    {

        typedef CMsgPacket RtspPacket;

        extern CMsgProtocol rtsp_protocol;

    } // namespace protocol
} // namespace util

#endif // _UTIL_PROTOCOL_RTSP_RTSP_PACKET_H_
