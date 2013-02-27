// RtmpUserControlMessage.h

#ifndef _UTIL_PROTOCOL_RTMP_RTMP_USER_CONTROL_MESSAGE_H_
#define _UTIL_PROTOCOL_RTMP_RTMP_USER_CONTROL_MESSAGE_H_

#include "util/protocol/rtmp/RtmpChunkMessage.h"

namespace util
{
    namespace protocol
    {

        /* Section 6.2
           User Control messages SHOULD use message stream ID 0 (known as the
           control stream) and, when sent over RTMP Chunk Stream, be sent on
           chunk stream ID 2. User Control messages are effective at the point
           they are received in the stream; their timestamps are ignored.
         */

        enum RtmpUserControlMessageType
        {
            RUCE_StreamBegin = 0, 
            RUCE_StreamEOF = 1, 
            RUCE_StreamDry = 2, 
            RUCE_SetBufferLength = 3, 
            RUCE_StreamIsRecorded = 4, 
            RUCE_PingRequest = 6, 
            RUCE_PingResponse = 7, 
        };

        template <
            boost::uint32_t Type
        >
        struct RtmpUserControlMessageData
        {
            static boost::uint32_t const StaticEventType = Type;
        };

        struct RtmpUserControlStreamBegin
            : RtmpUserControlMessageData<RUCE_StreamBegin>
        {
        public:
            boost::uint32_t stream_id;

        public:
            RtmpUserControlStreamBegin(
                boost::uint32_t stream_id = 0)
                : stream_id(stream_id)
            {
            }

            template <typename Archive>
            void serialize(
                Archive & ar)
            {
                ar & stream_id;
            }
        };

        struct RtmpUserControlStreamEOF
            : RtmpUserControlMessageData<RUCE_StreamEOF>
        {
        public:
            boost::uint32_t stream_id;

        public:
            RtmpUserControlStreamEOF(
                boost::uint32_t stream_id = 0)
                : stream_id(stream_id)
            {
            }

            template <typename Archive>
            void serialize(
                Archive & ar)
            {
                ar & stream_id;
            }
        };

        struct RtmpUserControlStreamDry
            : RtmpUserControlMessageData<RUCE_StreamDry>
        {
        public:
            boost::uint32_t stream_id;

        public:
            RtmpUserControlStreamDry(
                boost::uint32_t stream_id = 0)
                : stream_id(stream_id)
            {
            }

            template <typename Archive>
            void serialize(
                Archive & ar)
            {
                ar & stream_id;
            }
        };

        struct RtmpUserControlSetBufferLength
            : RtmpUserControlMessageData<RUCE_SetBufferLength>
        {
        public:
            boost::uint32_t stream_id;
            boost::uint32_t buffer_length; // milliseconds

        public:
            RtmpUserControlStreamDry(
                boost::uint32_t stream_id = 0, 
                boost::uint32_t buffer_length = 0)
                : stream_id(stream_id)
                , buffer_length(buffer_length)
            {
            }

            template <typename Archive>
            void serialize(
                Archive & ar)
            {
                ar & stream_id;
                ar & buffer_length;
            }
        };

        struct RtmpUserControlStreamIsRecorded
            : RtmpUserControlMessageData<RUCE_StreamIsRecorded>
        {
        public:
            boost::uint32_t stream_id;

        public:
            RtmpUserControlStreamIsRecorded(
                boost::uint32_t stream_id = 0)
                : stream_id(stream_id)
            {
            }

            template <typename Archive>
            void serialize(
                Archive & ar)
            {
                ar & stream_id;
            }
        };

        struct RtmpUserControlPingRequest
            : RtmpUserControlMessageData<RUCE_PingRequest>
        {
        public:
            boost::uint32_t timestamp;

        public:
            RtmpUserControlPingRequest(
                boost::uint32_t timestamp = 0)
                : timestamp(timestamp)
            {
            }

            template <typename Archive>
            void serialize(
                Archive & ar)
            {
                ar & timestamp;
            }
        };

        struct RtmpUserControlPingResponse
            : RtmpUserControlMessageData<RUCE_PingResponse>
        {
        public:
            boost::uint32_t timestamp;

        public:
            RtmpUserControlPingResponse(
                boost::uint32_t timestamp = 0)
                : timestamp(timestamp)
            {
            }

            template <typename Archive>
            void serialize(
                Archive & ar)
            {
                ar & timestamp;
            }
        };

        struct RtmpUserControlMessage
            : RtmpChunkMessageData<RCMT_UserControl>
        {
            boost::uint16_t event_type;
            boost::uint16_t event_size;
            boost::uint8_t event_data[2];

            RtmpUserControlMessage()
            {
            }

            template
            template <typename Archive>
            void serialize(
                Archive & ar)
            {
                ar & event_type;
                ar & framework::;
            }
        };

    } // namespace protocol
} // namespace util

#endif // _UTIL_PROTOCOL_RTMP_RTMP_USER_CONTROL_MESSAGE_H_
