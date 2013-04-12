// MmspMessage.h

#ifndef _UTIL_PROTOCOL_MMSP_MMSP_MESSAGE_H_
#define _UTIL_PROTOCOL_MMSP_MMSP_MESSAGE_H_

#include "util/protocol/mmsp/MmspTcpMessage.h"
#include "util/protocol/mmsp/MmspMessageTraits.h"
#include "util/protocol/Message.h"

namespace util
{
    namespace protocol
    {

        class MmspMessageHeader
            : MmspTcpMessageHeader
        {
        public:
            boost::uint32_t chunkLen;              // 当前位置到结束的块的个数 X 8 byte
            boost::uint32_t MID;                    // 命令方向和ID

        public:
            MmspMessageHeader(
                boost::uint32_t id = 0)
                : chunkLen(0)
                , MID(id)
            {
            }

            static boost::uint32_t chunk_size(
                boost::uint32_t & n)
            {
                boost::uint32_t align_size = n & 0x07;
                align_size = (align_size == 0) ? 0 : 8 - align_size;
                align_size += n;
                n = align_size - n;
                return align_size / 8;
            }

            boost::uint32_t id() const
            {
                return MID;
            }

            void id(
                boost::uint32_t n)
            {
                MID = n;
            }

            boost::uint32_t data_size() const
            {
                return (chunkLen - 1) * 8;
            }

            void data_size(
                boost::uint32_t n)
            {
                chunkLen = n / 8 + 1;
                chunkCount = chunkLen + 2;
                messageLength = chunkCount * 8;
            }

            static boost::uint8_t const HEAD_SIZE = 8;

            template <typename Archive>
            void serialize(
                Archive & ar)
            {
                MmspTcpMessageHeader::serialize(ar);

                ar & chunkLen;
                ar & MID;
            }
        };

        typedef Message<MmspMessageTraits> MmspMessage;

    } // namespace protocol
} // namespace util

#endif // _UTIL_PROTOCOL_MMSP_MMSP_MESSAGE_H_
