// MmspMessageData.h

#ifndef _UTIL_PROTOCOL_MMSP_MMSP_MESSAGE_DATA_H_
#define _UTIL_PROTOCOL_MMSP_MMSP_MESSAGE_DATA_H_

#include "util/protocol/mmsp/MmspMessageTraits.h"
#include "util/protocol/MessageData.h"

namespace util
{
    namespace protocol
    {

        template <
            typename T, 
            MmspMessageTraits::id_type id
        >
        struct MmspMessageData
            : MessageData<MmspMessageTraits, T, id>
        {
        };

    } // namespace protocol
} // namespace util

#endif // _UTIL_PROTOCOL_MMSP_MMSP_MESSAGE_DATA_H_
