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
            boost::uint8_t rep;                     // ����Ϊ0x01
            boost::uint8_t version;                     // ����Ϊ0x00
            boost::uint8_t versionMinor;                 // ����Ϊ0x00
            boost::uint8_t padding;                     // Ӧ��Ϊ0x00�����ն˺���
            boost::uint32_t sessionId;             // ����ǩ��������Ϊ0xb00bface
            boost::uint32_t messageLength;                // ����ȣ�bytes
            boost::uint32_t seal;                   // Э�����ͣ�����Ϊ0x20534d4d
            boost::uint32_t chunkCount;            // ��ǰλ�õ������Ŀ�ĸ��� X 8 byte
            boost::uint16_t seq;                    // ���кţ�ÿ����һ�μ�һ
            boost::uint16_t MBZ;                    // ����Ϊ0x0000
            boost::uint64_t timeSent;              // ʱ��������ն˺���

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
