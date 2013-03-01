// MessageBase.h

#ifndef _UTIL_PROTOCOL_MESSAGE_BASE_H_
#define _UTIL_PROTOCOL_MESSAGE_BASE_H_

#include "util/protocol/MessageDefine.h"

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
                StreamBuffer & buf, 
                void *);

            void to_data(
                StreamBuffer & buf, 
                void *) const;

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
