// RtmpMessage.h

#ifndef _UTIL_PROTOCOL_RTMP_RTMP_MESSAGE_HPP_
#define _UTIL_PROTOCOL_RTMP_RTMP_MESSAGE_HPP_

#include "util/protocol/Message.hpp"
#include "util/protocol/rtmp/RtmpMessageParser.h"

namespace util
{
    namespace protocol
    {

        template <typename Archive>
        void RtmpMessageHeader::save(
            Archive & ar) const
        {
            RtmpMessageParser * parser = (RtmpMessageParser *)ar.context();
            RtmpChunkHeader chunk;
            parser->to_chunk(*this, chunk);
            ar & chunk;
        }

        template <typename Archive>
        void RtmpMessageHeader::load(
            Archive & ar)
        {
            RtmpMessageParser * parser = (RtmpMessageParser *)ar.context();
            RtmpChunkHeader chunk;
            ar & chunk;
            parser->from_chunk(*this, chunk);
        }

    } // namespace protocol
} // namespace util

#endif // _UTIL_PROTOCOL_RTMP_RTMP_MESSAGE_HPP_
