// RtspFields.cpp

#include "util/Util.h"
#define CMSG_DEFINE_FIELD
#include "util/protocol/rtsp/RtspFields.h"

namespace util
{
    namespace protocol
    {

        namespace rtsp_field
        {
            float const RangeNptTraits::adjust_end = 0.0f;
        }

    } // namespace protocol
} // namespace util
