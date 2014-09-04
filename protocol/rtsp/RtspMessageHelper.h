// RtspMessageHelper.h

#ifndef _UTIL_PROTOCOL_RTSP_RTSP_MESSAGE_HELPER_H_
#define _UTIL_PROTOCOL_RTSP_RTSP_MESSAGE_HELPER_H_

#include "util/protocol/MessageHelper.h"

namespace util
{
    namespace protocol
    {

        class RtspMessageHelper
        {
        public:
            typedef RtspMessageTraits::header_type header_type;

            typedef RtspMessageTraits::i_archive_t i_archive_t;

            typedef RtspMessageTraits::o_archive_t o_archive_t;

            typedef RtspMessageTraits::context_t context_t;

        public:
            RtspMessageHelper(
                i_archive_t & ar, 
                header_type & header, 
                context_t * ctx)
            {
                char c = ar.rdbuf()->sgetc();
                if (c >= '1' && c <= '9') {
                    header.id(RtspMessageType::RESPONSE);
                } else {
                    header.id(RtspMessageType::REQUEST);
                }
            }

            RtspMessageHelper(
                o_archive_t & ar, 
                header_type const & header, 
                context_t * ctx)
            {
            }

            ~RtspMessageHelper()
            {
            }

        public:
            void begin_data() {};
        };

    } // namespace protocol
} // namespace util

#endif // _UTIL_PROTOCOL_RTSP_RTSP_MESSAGE_HELPER_H_
