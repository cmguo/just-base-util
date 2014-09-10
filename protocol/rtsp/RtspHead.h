// RtspHead.h

#ifndef _UTIL_PROTOCOL_RTSP_RTSP_HEAD_H_
#define _UTIL_PROTOCOL_RTSP_RTSP_HEAD_H_

#include "util/protocol/mine/MineHead.h"
#include "util/protocol/rtsp/RtspFieldRange.h"
#include "util/protocol/rtsp/RtspFieldConnection.h"

#include "util/serialization/NVPair.h"

namespace util
{
    namespace protocol
    {

        class RtspHead
            : public MineHeadT<RtspHead>
        {
        public:
            boost::optional<std::string> public_;
            boost::optional<rtsp_field::Range> range;
            boost::optional<std::string> rtp_info;
            boost::optional<std::string> transport;

        public:
            template <typename Archive>
            void serialize(
                Archive & ar)
            {
                ar 
                    & SERIALIZATION_NVP_NAME("Public", public_)
                    & SERIALIZATION_NVP_NAME("Range", range)
                    & SERIALIZATION_NVP_NAME("RTP-Info", rtp_info)
                    & SERIALIZATION_NVP_NAME("Transport", transport)
                    ;
                MineHeadT<RtspHead>::serialize(ar);
            }

        private:
            virtual bool get_line(
                std::string & line) const = 0;

            virtual bool set_line(
                std::string const & line) = 0;
        };

    } // namespace protocol
} // namespace util

#endif // _UTIL_PROTOCOL_RTSP_RTSP_HEAD_H_
