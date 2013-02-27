// RtmpMessageParser.h

#ifndef _UTIL_PROTOCOL_RTMP_RTMP_MESSAGE_PARSER_H_
#define _UTIL_PROTOCOL_RTMP_RTMP_MESSAGE_PARSER_H_

#include "util/protocol/MessageParser.h"
#include "util/protocol/rtmp/RtmpChunkHeader.h"

namespace util
{
    namespace protocol
    {

        class RtmpMessageHeader;

        class RtmpMessageParser
            : public MessageParser
        {
        public:
            RtmpMessageParser();

        public:
            void set_chunk_size(
                boost::uint32_t n);

            void acknowledgement(
                boost::uint32_t n);

        public:
            void to_chunk(
                RtmpMessageHeader const & msg, 
                RtmpChunkHeader & chunk);

            void from_chunk(
                RtmpMessageHeader & msg, 
                RtmpChunkHeader const & chunk);

        public:
            virtual void parse(
                boost::asio::const_buffer const & buf);

        private:
            std::vector<RtmpChunkHeader> chunks_;
            boost::uint32_t chunk_size_;
        };

    } // namespace protocol
} // namespace util

#endif // _UTIL_PROTOCOL_RTMP_RTMP_MESSAGE_PARSER_H_
