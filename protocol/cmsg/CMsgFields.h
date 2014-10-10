// CMsgFieldDef.h

#ifndef _UTIL_PROTOCOL_CMSG_CMSG_FIELDS_H_
#define _UTIL_PROTOCOL_CMSG_CMSG_FIELDS_H_

#include "util/protocol/cmsg/CMsgFieldDef.h"
#include "util/protocol/cmsg/CMsgFieldRange.h"
#include "util/protocol/cmsg/CMsgFieldConnection.h"

namespace util
{
    namespace protocol
    {

        namespace cmsg_field
        {

            CMSG_DECLARE_FIELD(content_length, "Content-Length", boost::uint64_t);
            CMSG_DECLARE_FIELD(connection, "Connection", Connection);
            CMSG_DECLARE_FIELD(range, "Range", Range);
            CMSG_DECLARE_FIELD(content_range, "Content-Range", ContentRange);

        }

    } // namespace protocol
} // namespace util

#endif // _UTIL_PROTOCOL_CMSG_CMSG_FIELDS_H_
