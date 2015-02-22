// HttpFieldRange.h

#ifndef _UTIL_PROTOCOL_HTTP_HTTP_FIELD_RANGE_H_
#define _UTIL_PROTOCOL_HTTP_HTTP_FIELD_RANGE_H_

#include "util/protocol/cmsg/CMsgFieldRange.h"

namespace util
{
    namespace protocol
    {
        namespace http_field
        {

            struct RangeByteTraits
            {
                typedef boost::uint64_t type;

                static boost::uint64_t const adjust_end = 1;

                char const * prefix() const
                {
                    return "bytes";
                }
            };

            typedef cmsg_field::RangeUnitT<RangeByteTraits> RangeUnit;
            typedef cmsg_field::RangeT<RangeByteTraits> Range;
            typedef cmsg_field::ContentRangeT<RangeByteTraits> ContentRange;

        }

    } // namespace protocol
} // namespace util

#endif // _UTIL_PROTOCOL_HTTP_HTTP_FIELD_RANGE_H_
