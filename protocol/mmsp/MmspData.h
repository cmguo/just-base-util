// MmspTcpMessage.h

#ifndef _UTIL_PROTOCOL_MMSP_MMSP_DATA_H_
#define _UTIL_PROTOCOL_MMSP_MMSP_DATA_H_

namespace util
{
    namespace protocol
    {

        struct MmspDataHeader
        {
        public:
            boost::uint32_t LocationId;
            boost::uint8_t playIncarnation;
            boost::uint8_t AFFlags;
            boost::uint16_t PacketSize;

            static boost::uint8_t const HEAD_SIZE = 8;

        public:
            MmspDataHeader()
                : LocationId(0x00000000)
                , playIncarnation(0x00)
                , AFFlags(0x00)
                , PacketSize(0)
            {}
            
        public:
            template <typename Archive>
            void serialize(
                Archive & ar)
            {
                ar & LocationId;
                ar & playIncarnation;
                ar & AFFlags;
                ar & PacketSize;
            }
        };

    } // namespace protocol
} // namespace util

#endif // _UTIL_PROTOCOL_MMSP_MMSP_DATA_H_
