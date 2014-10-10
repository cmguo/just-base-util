// RtspRequest.h

#ifndef _UTIL_PROTOCOL_RTSP_RTSP_REQUEST_H_
#define _UTIL_PROTOCOL_RTSP_RTSP_REQUEST_H_

#include "util/protocol/rtsp/RtspPacket.h"
#include "util/protocol/rtsp/RtspMessageData.h"
#include "util/protocol/rtsp/RtspFieldRange.h"

#include "util/protocol/cmsg/CMsgRequest.h"

namespace util
{
    namespace protocol
    {

        class RtspRequestHead
            : public CMsgRequestHead
        {
        public:
            enum MethodEnum
            {
                describe, 
                announce, 
                get_parameter, 
                set_parameter, 
                options, 
                redirect, 
                setup, 
                play, 
                record, 
                pause, 
                teardown, 
                invalid_method, 
            };

        public:
            RtspRequestHead(
                MethodEnum method = invalid_method);

            RtspRequestHead(
                MethodEnum method, 
                std::string const & path, 
                size_t version = 0x00000100);

        public:
            //static std::string const method_str[];
        };

        class RtspRequest
            : public CMsgRequest
            , public RtspMessageData<RtspRequest, RtspMessageType::REQUEST>
        {
        public:
            RtspRequest();

            RtspRequest(
                RtspRequest const & r);

        public:
            RtspRequestHead & head()
            {
                return head_;
            }

            RtspRequestHead const & head() const
            {
                return head_;
            }

        private:
            RtspRequestHead head_;
        };

    } // namespace protocol
} // namespace util

#endif // _UTIL_PROTOCOL_RTSP_RTSP_REQUEST_H_
