// CMsgProtocol.h

#ifndef _UTIL_PROTOCOL_CMSG_CMSG_PROTOCOL_H_
#define _UTIL_PROTOCOL_CMSG_CMSG_PROTOCOL_H_

namespace util
{
    namespace protocol
    {

        struct CMsgProtocol
        {
            char const * name;
            char const * const * method_strings;
        };

    } // namespace protocol
} // namespace util

#endif // _UTIL_PROTOCOL_CMSG_CMSG_PROTOCOL_H_
