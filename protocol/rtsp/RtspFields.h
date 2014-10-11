// RtspFields.h

#ifndef _UTIL_PROTOCOL_RTSP_RTSP_FIELDS_H_
#define _UTIL_PROTOCOL_RTSP_RTSP_FIELDS_H_

#include "util/protocol/rtsp/RtspFieldRange.h"
#include "util/protocol/rtsp/RtspFieldRtpInfo.h"
#include "util/protocol/cmsg/CMsgFieldDef.h"

namespace util
{
    namespace protocol
    {
        namespace rtsp_field
        {

            CMSG_DECLARE_FIELD(public, "Public", std::string);
            CMSG_DECLARE_FIELD(transport, "Transport", std::string);
            CMSG_DECLARE_FIELD(rtp_info, "RTP-Info", std::string);
            CMSG_DECLARE_FIELD(range, "Range", Range);

        }
    } // namespace protocol
} // namespace util

#endif // _UTIL_PROTOCOL_RTSP_RTSP_FIELDS_H_
