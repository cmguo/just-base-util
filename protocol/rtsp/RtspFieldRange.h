// RtspFieldRange.h

#ifndef _UTIL_PROTOCOL_RTSP_RTSP_FIELD_RANGE_H_
#define _UTIL_PROTOCOL_RTSP_RTSP_FIELD_RANGE_H_

#include "util/protocol/cmsg/CMsgFieldRange.h"

namespace util
{
    namespace protocol
    {
        namespace rtsp_field
        {

            struct RangeNptTraits
            {
                typedef float type;

                char const * prefix() const
                {
                    return "npt";
                }
            };

            typedef cmsg_field::RangeUnitT<float> RangeUnit;
            typedef cmsg_field::RangeT<RangeNptTraits> Range;

        }

    } // namespace protocol
} // namespace util

#endif // _UTIL_PROTOCOL_RTSP_RTSP_FIELD_RANGE_H_
