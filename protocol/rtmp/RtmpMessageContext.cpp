// RtmpMessageOneContext.cpp

#include "util/Util.h"
#include "util/protocol/rtmp/RtmpMessageContext.h"
#include "util/protocol/rtmp/RtmpMessage.h"

namespace util
{
    namespace protocol
    {

        RtmpMessageOneContext::RtmpMessageOneContext()
            : chunk_size_(128)
            , seq_(0)
        {
            streams_.resize(1, true);
        }

        void RtmpMessageOneContext::chunk_size(
            boost::uint32_t n)
        {
            chunk_size_ = n;
        }

        void RtmpMessageOneContext::acknowledgement(
            boost::uint32_t n)
        {
            ack_ = n;
        }

        RtmpChunkMessage & RtmpMessageOneContext::chunk(
            boost::uint16_t cs_id)
        {
            if (chunks_.size() <= cs_id) {
                size_t n = chunks_.size();
                chunks_.resize(cs_id + 1);
                for (size_t i = n; i < chunks_.size(); ++i) {
                    chunks_[i].cs_id(i);
                }
            }
            return chunks_[cs_id];
        }

        void RtmpMessageOneContext::stream_begin(
            boost::uint32_t i)
        {
            if (streams_.size() <= i) {
                streams_.resize(i + 1, false);
            }
            streams_[i] = true;
        }

        void RtmpMessageOneContext::stream_end(
            boost::uint32_t i)
        {
            if (streams_.size() <= i) {
                streams_.resize(i + 1, false);
            }
            streams_[i] = false;
        }

        bool RtmpMessageOneContext::stream_status(
            boost::uint32_t i)
        {
            if (streams_.size() <= i) {
                streams_.resize(i + 1, false);
            }
            return streams_[i];
        }

        void RtmpMessageWriteContext::to_chunk(
            RtmpMessageHeader const & msg, 
            RtmpChunkHeader & chunk)
        {
            RtmpChunkHeader & wchunk(RtmpMessageOneContext::chunk(msg.chunk));
            chunk.calc_timestamp = msg.timestamp;
            chunk.message_length = msg.length;
            chunk.message_type_id = msg.type;
            chunk.message_stream_id = msg.stream;
            chunk = wchunk.dec(chunk);
            seq_ += msg.length;
        }

        void RtmpMessageReadContext::from_chunk(
            RtmpMessageHeader & msg, 
            RtmpChunkHeader const & chunk)
        {
            msg.chunk = chunk.cs_id();
            RtmpChunkHeader & rchunk(RtmpMessageOneContext::chunk(msg.chunk));
            RtmpChunkHeader const & chunk2 = 
                rchunk.add(chunk);
            msg.timestamp = chunk2.calc_timestamp;
            msg.type = chunk2.message_type_id;
            msg.length = chunk2.message_length;
            msg.stream = chunk2.message_stream_id;
            seq_ += msg.length;
        }

    } // namespace protocol
} // namespace util
