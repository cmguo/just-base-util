// RtspResponse.cpp

#include "util/Util.h"
#include "util/protocol/rtsp/RtspResponse.h"

namespace util
{
    namespace protocol
    {

        RtspResponseHead::RtspResponseHead(
            size_t err_code, 
            size_t version)
            : CMsgResponseHead(version, err_code, "")
        {
            protocol_ = &rtsp_protocol;
        }

        RtspResponseHead::RtspResponseHead(
            size_t err_code, 
            std::string const & err_msg, 
            size_t version)
            : CMsgResponseHead(version, err_code, err_msg)
        {
            protocol_ = &rtsp_protocol;
        }

        RtspResponse::RtspResponse()
            : CMsgResponse(head_)
        {
        }

        RtspResponse::RtspResponse(
            RtspResponse const & r)
            : CMsgResponse(r, head_)
        {
            head_ = r.head_;
        }

    } // namespace protocol
} // namespace util
