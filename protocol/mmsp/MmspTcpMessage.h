// MmspTcpMessage.h

#ifndef _UTIL_PROTOCOL_MMSP_MMSP_TCP_MESSAGE_H_
#define _UTIL_PROTOCOL_MMSP_MMSP_TCP_MESSAGE_H_

namespace util
{
    namespace protocol
    {

        struct MmspTcpMessageHeader
        {
        public:
            boost::uint8_t rep;                     // 必须为0x01
            boost::uint8_t version;                     // 必须为0x00
            boost::uint8_t versionMinor;                 // 必须为0x00
            boost::uint8_t padding;                     // 应该为0x00，接收端忽略
            boost::uint32_t sessionId;             // 命令签名，必须为0xb00bface
            boost::uint32_t messageLength;                // 命令长度：bytes
            boost::uint32_t seal;                   // 协议类型，必须为0x20534d4d
            boost::uint32_t chunkCount;            // 当前位置到结束的块的个数 X 8 byte
            boost::uint16_t seq;                    // 序列号，每发送一次加一
            boost::uint16_t MBZ;                    // 必须为0x0000
            boost::uint64_t timeSent;              // 时间戳，接收端忽略

            static boost::uint8_t const HEAD_SIZE = 32;

        public:
            MmspTcpMessageHeader()
                : rep(0x01)
                , version(0x00)
                , versionMinor(0x00)
                , padding(0x00)
                , sessionId(2953575118)
                , messageLength(0)
                , seal(0x20534d4d)
                , chunkCount(0)
                , seq(0)
                , MBZ(0x0000)
                , timeSent(0x0000000000000000)
            {}
            
        public:
            template <typename Archive>
            void serialize(
                Archive & ar)
            {
                ar & rep;
                ar & version;
                ar & versionMinor;
                ar & padding;
                ar & sessionId;
                ar & messageLength;
                ar & seal;
                ar & chunkCount;
                ar & seq;
                ar & MBZ;
                ar & timeSent;
            }
        };

    } // namespace protocol
} // namespace util

#endif // _UTIL_PROTOCOL_MMSP_MMSP_TCP_MESSAGE_H_
