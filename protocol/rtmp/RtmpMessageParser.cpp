// RtmpMessageParser.cpp

#include "util/Util.h"
#include "util/protocol/rtmp/RtmpMessageParser.h"
#include "util/protocol/rtmp/RtmpMessage.h"

namespace util
{
    namespace protocol
    {

        RtmpMessageParser::RtmpMessageParser()
            : chunk_size_(128)
        {

        }

        void RtmpMessageParser::set_chunk_size(
            boost::uint32_t n)
        {
            chunk_size_ = n;
        }

        void RtmpMessageParser::acknowledgement(
            boost::uint32_t n)
        {
        }

        void RtmpMessageParser::to_chunk(
            RtmpMessageHeader const & msg, 
            RtmpChunkHeader & chunk)
        {
            if (chunks_.size() <= msg.chunk) {
                chunks_.resize(msg.chunk + 1);
            }
            chunk.calc_timestamp = msg.timestamp;
            chunk.message_length = msg.length;
            chunk.message_type_id = msg.type;
            chunk.message_stream_id = msg.stream;
            chunk = chunks_[msg.chunk].dec(chunk);
        }

        void RtmpMessageParser::from_chunk(
            RtmpMessageHeader & msg, 
            RtmpChunkHeader const & chunk)
        {
            msg.chunk = chunk.cs_id();
            if (chunks_.size() <= msg.chunk) {
                chunks_.resize(msg.chunk + 1);
            }
            RtmpChunkHeader const & chunk2 = 
                chunks_[msg.chunk].add(chunk);
            msg.timestamp = chunk2.calc_timestamp;
            msg.type = chunk2.message_type_id;
            msg.length = chunk2.message_length;
            msg.stream = chunk2.message_stream_id;
        }

        void RtmpMessageParser::parse(
            boost::asio::const_buffer const & buf)
        {

        }

    } // namespace protocol
} // namespace util
