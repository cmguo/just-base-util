// RtspMessageParser.h

#ifndef _UTIL_PROTOCOL_RTSP_RTSP_MESSAGE_PARSER_H_
#define _UTIL_PROTOCOL_RTSP_RTSP_MESSAGE_PARSER_H_

#include "util/protocol/MessageParser.h"
#include "util/protocol/MessageDefine.h"

namespace util
{
    namespace protocol
    {

        class RtspMessageParser
            : public MessageParser
        {
        public:
            RtspMessageParser();

        public:
            virtual void parse(
                boost::asio::const_buffer const & buf);

        private:
            MessageDefine data_def_;
        };

    } // namespace protocol
} // namespace util

#endif // _UTIL_PROTOCOL_RTSP_RTSP_MESSAGE_PARSER_H_
