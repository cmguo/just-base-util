// MmspTcpMessage.h

#ifndef _UTIL_PROTOCOL_MMSP_MMSP_FORMAT_H_
#define _UTIL_PROTOCOL_MMSP_MMSP_FORMAT_H_

#include "util/archive/LittleEndianBinaryIArchive.h"
#include "util/archive/LittleEndianBinaryOArchive.h"

namespace util
{
    namespace protocol
    {

        typedef util::archive::LittleEndianBinaryIArchive<boost::uint8_t> MsspIArchive;
        typedef util::archive::LittleEndianBinaryOArchive<boost::uint8_t> MsspOArchive;

    } // namespace protocol
} // namespace util

#endif // _UTIL_PROTOCOL_MMSP_MMSP_FORMAT_H_
