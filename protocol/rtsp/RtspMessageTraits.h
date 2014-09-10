// RtspMessageTraits.h

#ifndef _UTIL_PROTOCOL_RTSP_RTSP_MESSAGE_TRAITS_H_
#define _UTIL_PROTOCOL_RTSP_RTSP_MESSAGE_TRAITS_H_

#include "util/protocol/MessageTraits.h"

#include <util/archive/BigEndianBinaryIArchive.h>
#include <util/archive/BigEndianBinaryOArchive.h>

namespace util
{
    namespace protocol
    {

        class RtspMessageHeader;
        class RtspMessageHelper;
        class RtspHeadIArchive;
        class RtspHeadOArchive;
        struct RtspMessageContext;

        struct RtspMessageTraits
            : MessageTraits
        {
            typedef boost::uint8_t id_type;

            typedef RtspMessageHeader header_type;

            typedef util::archive::BigEndianBinaryIArchive<boost::uint8_t> i_archive_t;

            typedef util::archive::BigEndianBinaryOArchive<boost::uint8_t> o_archive_t;

            static size_t const max_size = 800;

            typedef RtspMessageContext context_t;

            typedef RtspMessageHelper helper_t;
        };

    } // namespace protocol
} // namespace util

#endif // _UTIL_PROTOCOL_RTSP_RTSP_MESSAGE_TRAITS_H_
