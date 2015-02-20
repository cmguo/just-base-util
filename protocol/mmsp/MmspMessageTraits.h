// MmspMessageTraits.h

#ifndef _UTIL_PROTOCOL_MMSP_MMSP_MESSAGE_TRAITS_H_
#define _UTIL_PROTOCOL_MMSP_MMSP_MESSAGE_TRAITS_H_

#include "util/protocol/MessageTraits.h"

#include <util/archive/LittleEndianBinaryIArchive.h>
#include <util/archive/LittleEndianBinaryOArchive.h>

namespace util
{
    namespace protocol
    {

        class MmspMessageHeader;
        class MmspMessageHelper;

        struct MmspMessageTraits
            : MessageTraits<MmspMessageTraits>
        {
            typedef boost::uint32_t id_type;

            typedef MmspMessageHeader header_type;

            typedef util::archive::LittleEndianBinaryIArchive<boost::uint8_t> i_archive_t;

            typedef util::archive::LittleEndianBinaryOArchive<boost::uint8_t> o_archive_t;

            static size_t const max_size = 200;

            typedef MmspMessageHelper helper_t;
        };

    } // namespace protocol
} // namespace util

#endif // _UTIL_PROTOCOL_MMSP_MMSP_MESSAGE_TRAITS_H_
