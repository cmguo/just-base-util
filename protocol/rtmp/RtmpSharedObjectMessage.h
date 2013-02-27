// RtmpUserControlMessage.h

#ifndef _UTIL_PROTOCOL_RTMP_RTMP_USER_CONTRIMESSAGE_H_
#define _UTIL_PROTOCOL_RTMP_RTMP_USER_CONTRIMESSAGE_H_

namespace util
{
    namespace protocol
    {

        // Section 6.2
        // Section 6.2

        enum RtmpUserControlEventType
        {
            RUCE_Use = 1, 
            RUCE_Release = 2, 
            RUCE_RequestChange = 3, 
            RUCE_Change = 4, 
            RUCE_Success = 5, 
            RUCE_SendMessage = 6, 
            RUCE_Status = 7, 
            RUCE_Clear = 8, 
            RUCE_Remove = 9, 
            RUCE_RequestRemove = 10, 
            RUCE_UseSuccess = 11, 
        };

        template <
            boost::uint32_t ID
        >
        struct RtmpUserControlEventData
        {
            static boost::uint32_t const StaticId = ID;
        };

        struct RtmpSetChunkSize
            : RtmpMessageData<RMT_SetChunkSize>
        {
        public:
            boost::uint32_t chunk_size;

        public:
            RtmpSetChunkSize(
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

        struct RtmpAbortMessage
            : RtmpMessageData<RMT_AbortMessage>
        {
        public:
            boost::uint32_t chunk_stream_id;

        public:
            RtmpAbortMessage(
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

        struct Acknowledgement
            : RtmpMessageData<RMT_Acknowledgement>
        {
        public:
            boost::uint32_t sequence_number;

        public:
            RtmpAbortMessage(
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

        struct WindowAcknowledgementSize
            : RtmpMessageData<RMT_WindowAcknowledgementSize>
        {
        public:
            boost::uint32_t acknowledgement_window_size;

        public:
            RtmpAbortMessage(
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

        struct SetPeerBandwidth
            : RtmpMessageData<RMT_SetPeerBandwidth>
        {
        public:
            boost::uint32_t acknowledgement_window_size;
            boost::uint8_t limit_type;

        public:
            RtmpAbortMessage(
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

#endif // _UTIL_PROTOCOL_RTMP_RTMP_CHUNK_MESSAGE_H_
