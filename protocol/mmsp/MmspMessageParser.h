// MmspMessageParser.h

#ifndef _UTIL_PROTOCOL_MMSP_MMSP_MESSAGE_PARSER_H_
#define _UTIL_PROTOCOL_MMSP_MMSP_MESSAGE_PARSER_H_

#include "util/protocol/MessageParser.h"
#include "util/protocol/MessageDefine.h"

namespace util
{
    namespace protocol
    {

        class MmspMessageParser
            : public MessageParser
        {
        public:
            MmspMessageParser();

        public:
            virtual void parse(
                boost::asio::const_buffer const & buf);

        private:
            MessageDefine data_def_;
        };

    } // namespace protocol
} // namespace util

#endif // _UTIL_PROTOCOL_MMSP_MMSP_MESSAGE_PARSER_H_
