// MmspMessageHelper.h

#ifndef _UTIL_PROTOCOL_MMSP_MMSP_MESSAGE_HELPER_H_
#define _UTIL_PROTOCOL_MMSP_MMSP_MESSAGE_HELPER_H_

#include "util/protocol/MessageHelper.h"

namespace util
{
    namespace protocol
    {

        class MmspMessageHelper
            : public MessageHelper<MmspMessageTraits>
        {
        public:
            MmspMessageHelper(
                i_archive_t & ar, 
                header_type & header, 
                context_t * ctx)
                : MessageHelper<MmspMessageTraits>(ar, header, ctx)
            {
            }

            MmspMessageHelper(
                o_archive_t & ar, 
                header_type const & header, 
                context_t * ctx)
                : MessageHelper<MmspMessageTraits>(ar, header, ctx)
            {
            }

            ~MmspMessageHelper()
            {
                boost::uint32_t n = data_size();
                MmspMessageHeader::chunk_size(n);
                pad(n, 0);
            }
        };

    } // namespace protocol
} // namespace util

#endif // _UTIL_PROTOCOL_MMSP_MMSP_MESSAGE_HELPER_H_
