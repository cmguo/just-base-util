// RtspFieldRange.h

#ifndef _UTIL_PROTOCOL_RTSP_RTSP_FIELD_RANGE_H_
#define _UTIL_PROTOCOL_RTSP_RTSP_FIELD_RANGE_H_

#include "util/protocol/mine/MineFieldRange.h"

namespace util
{
    namespace protocol
    {
        namespace rtsp_field
        {

            struct PrefixNpt
            {
                char const * operator()() const
                {
                    return "npt";
                }
            };

            typedef mine_field::RangeUnit<float> RangeUnit;
            typedef mine_field::Range<float, PrefixNpt> Range;

        }

    } // namespace protocol
} // namespace util

#endif // _UTIL_PROTOCOL_RTSP_RTSP_FIELD_RANGE_H_
