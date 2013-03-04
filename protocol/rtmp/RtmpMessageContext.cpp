// RtmpMessageContext.cpp

#include "util/Util.h"
#include "util/protocol/rtmp/RtmpMessageContext.h"
#include "util/protocol/rtmp/RtmpMessage.h"

namespace util
{
    namespace protocol
    {

        RtmpMessageContext::RtmpMessageContext()
            : read_chunk_size_(128)
            , write_chunk_size_(128)
        {
        }

        void RtmpMessageContext::read_chunk_size(
            boost::uint32_t n)
        {
            read_chunk_size_ = n;
        }

        void RtmpMessageContext::read_acknowledgement(
            boost::uint32_t n)
        {
        }

        void RtmpMessageContext::write_chunk_size(
            boost::uint32_t n)
        {
            write_chunk_size_ = n;
        }

        void RtmpMessageContext::write_acknowledgement(
            boost::uint32_t & n)
        {
        }

        RtmpChunkMessage & RtmpMessageContext::read_chunk(
            boost::uint16_t cs_id)
        {
            if (read_chunks_.size() <= cs_id) {
                size_t n = read_chunks_.size();
                read_chunks_.resize(cs_id + 1);
                for (size_t i = n; i < read_chunks_.size(); ++i) {
                    read_chunks_[i].cs_id(i);
                }
            }
            return read_chunks_[cs_id];
        }

        RtmpChunkHeader & RtmpMessageContext::write_chunk(
            boost::uint16_t cs_id)
        {
            if (write_chunks_.size() <= cs_id) {
                size_t n = write_chunks_.size();
                write_chunks_.resize(cs_id + 1);
                for (size_t i = n; i < write_chunks_.size(); ++i) {
                    write_chunks_[i].cs_id(i);
                }
            }
            return write_chunks_[cs_id];
        }

        void RtmpMessageContext::to_chunk(
            RtmpMessageHeader const & msg, 
            RtmpChunkHeader & chunk)
        {
            RtmpChunkHeader & wchunk(write_chunk(msg.chunk));
            chunk.calc_timestamp = msg.timestamp;
            chunk.message_length = msg.length;
            chunk.message_type_id = msg.type;
            chunk.message_stream_id = msg.stream;
            chunk = wchunk.dec(chunk);
        }

        void RtmpMessageContext::from_chunk(
            RtmpMessageHeader & msg, 
            RtmpChunkHeader const & chunk)
        {
            msg.chunk = chunk.cs_id();
            RtmpChunkHeader & rchunk(read_chunk(msg.chunk));
            RtmpChunkHeader const & chunk2 = 
                rchunk.add(chunk);
            msg.timestamp = chunk2.calc_timestamp;
            msg.type = chunk2.message_type_id;
            msg.length = chunk2.message_length;
            msg.stream = chunk2.message_stream_id;
        }

    } // namespace protocol
} // namespace util
