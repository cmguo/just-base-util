// RtspResponse.h

#ifndef _UTIL_PROTOCOL_RTSP_RTSP_RESPONSE_H_
#define _UTIL_PROTOCOL_RTSP_RTSP_RESPONSE_H_

#include "util/protocol/rtsp/RtspPacket.h"
#include "util/protocol/rtsp/RtspMessageData.h"
#include "util/protocol/rtsp/RtspFieldRange.h"

#include "util/protocol/cmsg/CMsgResponse.h"

namespace util
{
    namespace protocol
    {
        class RtspResponseHead
            : public CMsgResponseHead
        {
        public:
            RtspResponseHead(
                size_t err_code = 200, 
                size_t version = 0x00000100);

            RtspResponseHead(
                size_t err_code, 
                std::string const & err_msg, 
                size_t version = 0x00000100);
        };

        class RtspResponse
            : public CMsgResponse
            , public RtspMessageData<RtspResponse, RtspMessageType::RESPONSE>
        {
        public:
            RtspResponse();

            RtspResponse(
                RtspResponse const & r);

        public:
            RtspResponseHead & head()
            {
                return head_;
            }

            RtspResponseHead const & head() const
            {
                return head_;
            }

        private:
            RtspResponseHead head_;
        };

    } // namespace protocol
} // namespace util

#endif // _UTIL_PROTOCOL_RTSP_RTSP_RESPONSE_H_
