// RtpPacket.h

#ifndef _UTIL_PROTOCOL_PTSP_RTP_PACKET_H_
#define _UTIL_PROTOCOL_PTSP_RTP_PACKET_H_

namespace util
{
    namespace protocol
    {

        struct RtpHead
        {
            RtpHead()
                : vpxcc(0)
                , mpt(0)
                , sequence(0)
                , timestamp(0)
                , ssrc(0)
            {
            }

            boost::uint8_t vpxcc;
            boost::uint8_t mpt;
            boost::uint16_t sequence;
            boost::uint32_t timestamp;
            boost::uint32_t ssrc;
        };

    } // namespace protocol
} // namespace util

#endif // _UTIL_PROTOCOL_PTSP_RTP_PACKET_H_
