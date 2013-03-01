// RtmpProtocolControlMessage.h

#ifndef _UTIL_PROTOCOL_RTMP_RTMP_PROTOCOL_CONTROL_MESSAGE_H_
#define _UTIL_PROTOCOL_RTMP_RTMP_PROTOCOL_CONTROL_MESSAGE_H_

#include "util/protocol/rtmp/RtmpChunkMessage.h"

namespace util
{
    namespace protocol
    {

        struct RtmpProtocolControlMessageSetChunkSize
            : RtmpChunkMessageData<RtmpProtocolControlMessageSetChunkSize, RCMT_SetChunkSize>
        {
        public:
            boost::uint32_t chunk_size;

        public:
            RtmpProtocolControlMessageSetChunkSize(
                boost::uint32_t chunk_size = 128)
                : chunk_size(chunk_size)
            {
            }

            template <typename Archive>
            void serialize(
                Archive & ar)
            {
                ar & chunk_size;
            }
        };

        struct RtmpProtocolControlMessageAbortMessage
            : RtmpChunkMessageData<RtmpProtocolControlMessageAbortMessage, RCMT_AbortMessage>
        {
        public:
            boost::uint32_t chunk_stream_id;

        public:
            RtmpProtocolControlMessageAbortMessage(
                boost::uint32_t chunk_stream_id = 0)
                : chunk_stream_id(chunk_stream_id)
            {
            }

            template <typename Archive>
            void serialize(
                Archive & ar)
            {
                ar & chunk_stream_id;
            }
        };

        struct RtmpProtocolControlMessageAcknowledgement
            : RtmpChunkMessageData<RtmpProtocolControlMessageAcknowledgement, RCMT_Acknowledgement>
        {
        public:
            boost::uint32_t sequence_number;

        public:
            RtmpProtocolControlMessageAcknowledgement(
                boost::uint32_t sequence_number = 0)
                : sequence_number(sequence_number)
            {
            }

            template <typename Archive>
            void serialize(
                Archive & ar)
            {
                ar & sequence_number;
            }
        };

        struct RtmpProtocolControlMessageWindowAcknowledgementSize
            : RtmpChunkMessageData<RtmpProtocolControlMessageWindowAcknowledgementSize, RCMT_WindowAcknowledgementSize>
        {
        public:
            boost::uint32_t acknowledgement_window_size;

        public:
            RtmpProtocolControlMessageWindowAcknowledgementSize(
                boost::uint32_t acknowledgement_window_size = 0)
                : acknowledgement_window_size(acknowledgement_window_size)
            {
            }

            template <typename Archive>
            void serialize(
                Archive & ar)
            {
                ar & acknowledgement_window_size;
            }
        };

        struct RtmpProtocolControlMessageSetPeerBandwidth
            : RtmpChunkMessageData<RtmpProtocolControlMessageSetPeerBandwidth, RCMT_SetPeerBandwidth>
        {
        public:
            boost::uint32_t acknowledgement_window_size;
            boost::uint8_t limit_type;

        public:
            RtmpProtocolControlMessageSetPeerBandwidth(
                boost::uint32_t acknowledgement_window_size = 0, 
                boost::uint8_t limit_type = 0)
                : acknowledgement_window_size(acknowledgement_window_size)
                , limit_type(limit_type)
            {
            }

            template <typename Archive>
            void serialize(
                Archive & ar)
            {
                ar & acknowledgement_window_size;
                ar & limit_type;
            }
        };

    } // namespace protocol
} // namespace util

#endif // _UTIL_PROTOCOL_RTMP_RTMP_PROTOCOL_CONTROL_MESSAGE_H_
