// RtmpMessageContext.h

#ifndef _UTIL_PROTOCOL_RTMP_RTMP_MESSAGE_CONTEXT_H_
#define _UTIL_PROTOCOL_RTMP_RTMP_MESSAGE_CONTEXT_H_

#include "util/protocol/MessageHelper.h"
#include "util/protocol/rtmp/RtmpChunkHeader.h"
#include "util/protocol/rtmp/RtmpChunkMessage.h"

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

            void read_stream(
                boost::uint32_t i, 
                bool b);

            void write_stream(
                boost::uint32_t i, 
                bool b);

        public:
            boost::uint32_t read_chunk_size() const
            {
                return read_chunk_size_;
            }

            boost::uint32_t write_chunk_size() const
            {
                return write_chunk_size_;
            }

            RtmpChunkMessage & read_chunk(
                boost::uint16_t cs_id);

            RtmpChunkHeader & write_chunk(
                boost::uint16_t cs_id);

            bool read_stream(
                boost::uint32_t i);

            bool write_stream(
                boost::uint32_t i);

        public:
            void to_chunk(
                RtmpMessageHeader const & msg, 
                RtmpChunkHeader & chunk);

            void from_chunk(
                RtmpMessageHeader & msg, 
                RtmpChunkHeader const & chunk);

        private:
            boost::uint32_t read_chunk_size_;
            boost::uint32_t write_chunk_size_;
            std::vector<RtmpChunkMessage> read_chunks_;
            std::vector<RtmpChunkHeader> write_chunks_;
            std::vector<bool> read_streams_;
            std::vector<bool> write_streams_;
        };

    } // namespace protocol
} // namespace util

#endif // _UTIL_PROTOCOL_RTMP_RTMP_MESSAGE_CONTEXT_H_
