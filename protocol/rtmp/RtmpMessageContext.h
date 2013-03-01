// RtmpMessageContext.h

#ifndef _UTIL_PROTOCOL_RTMP_RTMP_MESSAGE_CONTEXT_H_
#define _UTIL_PROTOCOL_RTMP_RTMP_MESSAGE_CONTEXT_H_

#include "util/protocol/MessageHelper.h"
#include "util/protocol/rtmp/RtmpChunkHeader.h"

namespace util
{
    namespace protocol
    {

        class RtmpMessageHeader;

        class RtmpMessageContext
        {
        public:
            RtmpMessageContext();

        public:
            void read_chunk_size(
                boost::uint32_t n);

            void read_acknowledgement(
                boost::uint32_t n);

            void write_chunk_size(
                boost::uint32_t n);

            void write_acknowledgement(
                boost::uint32_t & n);

        public:
            boost::uint32_t read_chunk_size() const
            {
                return read_chunk_size_;
            }

            boost::uint32_t write_chunk_size() const
            {
                return write_chunk_size_;
            }

            RtmpChunkHeader & read_chunk(
                boost::uint16_t cs_id);

            RtmpChunkHeader & write_chunk(
                boost::uint16_t cs_id);

        public:
            void to_chunk(
                RtmpMessageHeader const & msg, 
                RtmpChunkHeader & chunk);

            void from_chunk(
                RtmpMessageHeader & msg, 
                RtmpChunkHeader const & chunk);

        private:
            std::vector<RtmpChunkHeader> read_chunks_;
            std::vector<RtmpChunkHeader> write_chunks_;
            boost::uint32_t read_chunk_size_;
            boost::uint32_t write_chunk_size_;
        };

    } // namespace protocol
} // namespace util

#endif // _UTIL_PROTOCOL_RTMP_RTMP_MESSAGE_CONTEXT_H_
