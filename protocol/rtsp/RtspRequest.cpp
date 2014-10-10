// RtspRequest.cpp

#include "util/Util.h"
#include "util/protocol/rtsp/RtspRequest.h"
#include "util/protocol/rtsp/RtspError.h"

namespace util
{
    namespace protocol
    {

        RtspRequestHead::RtspRequestHead(
            MethodEnum method)
            : CMsgRequestHead(method, "", 0x00000100)
        {
            protocol_ = &rtsp_protocol;
        }

        RtspRequestHead::RtspRequestHead(
            MethodEnum method, 
            std::string const & path, 
            size_t version)
            : CMsgRequestHead(method, path, version)
        {
            protocol_ = &rtsp_protocol;
        }

        RtspRequest::RtspRequest()
            : CMsgRequest(head_)
        {
        }

        RtspRequest::RtspRequest(
            RtspRequest const & r)
            : CMsgRequest(r, head_)
        {
            head_ = r.head_;
        }

    } // namespace protocol
} // namespace util
