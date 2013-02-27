// RtmpCommandMessage.h

#ifndef _UTIL_PROTOCOL_RTMP_RTMP_USER_CONTROL_MESSAGE_H_
#define _UTIL_PROTOCOL_RTMP_RTMP_USER_CONTROL_MESSAGE_H_

#include "util/protocol/rtmp/RmtpAmfType.h"

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

        struct RtmpCommandMessage
            : RtmpChunkMessageData<RtmpCommandMessage, RCMT_CommandMessage>
        {
            RtmpAmfValue ComandName;
            RtmpAmfValue TransactionID;
            RtmpAmfValue CommandObject;
            std::vector<RtmpAmfValue> OptionalArguments;

            SERIALIZATION_SPLIT_MEMBER();

            template <typename Archive>
            void load(
                Archive & ar)
            {
                ar & ComandName;
                ar & TransactionID;
                ar & CommandObject;

                RtmpAmfValue Argument;
                OptionalArguments.clear();
                while (ar & Argument) {
                    OptionalArguments.push_back(Argument);
                }
            }

            template <typename Archive>
            void save(
                Archive & ar) const
            {
                ar & ComandName;
                ar & TransactionID;
                ar & CommandObject;

                for (size_t i = 0; i < OptionalArguments.size(); ++i) {
                    ar & OptionalArguments[i];
                }
            }
        };

    } // namespace protocol
} // namespace util

#endif // _UTIL_PROTOCOL_RTMP_RTMP_USER_CONTROL_MESSAGE_H_
