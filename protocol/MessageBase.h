// MessageBase.h

#ifndef _UTIL_PROTOCOL_MESSAGE_BASE_H_
#define _UTIL_PROTOCOL_MESSAGE_BASE_H_

#include "util/protocol/MessageTraits.h"

namespace util
{
    namespace protocol
    {

        class MessageBase
        {
        public:
            MessageBase();

            ~MessageBase();

        public:
            MessageBase(
                MessageBase const & r);

            MessageBase & operator=(
                MessageBase const & r);

        public:
            void from_data(
                boost::asio::streambuf & buf, 
                MessageParser &);

            void to_data(
                boost::asio::streambuf & buf, 
                MessageParser &) const;

            void reset();

            void reset(
                MessageDefine const * def);

            bool empty() const;

        protected:
            MessageDefine const * def_;
        };

    } // namespace protocol
} // namespace util

#endif // _UTIL_PROTOCOL_MESSAGE_BASE_H_
