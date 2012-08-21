// TransferBuffers.h

#ifndef _UTIL_STREAM_TRANSFER_BUFFERS_H_
#define _UTIL_STREAM_TRANSFER_BUFFERS_H_

#include "util/buffers/CycleBuffers.h"

namespace util
{
    namespace stream
    {

        template <
            typename Elem = char, 
            typename Allocator = std::allocator<Elem>, 
            typename Traits = std::char_traits<Elem>
        >
        class BasicTransferBuffers
            : private std::vector<Elem, Allocator>
            , public util::buffers::CycleBuffers<boost::asio::mutable_buffers_1, Elem, Traits>
        {
        public:
            typedef boost::asio::mutable_buffers_1 buffers_type;

            typedef util::buffers::CycleBuffers<buffers_type, Elem, Traits> cycle_buffers_type;

        public:
            BasicTransferBuffers(
                size_t capacity)
                : std::vector<Elem, Allocator>(capacity)
                , cycle_buffers_type(boost::asio::buffer((std::vector<Elem, Allocator> &)*this))
            {
            }
        };

        typedef BasicTransferBuffers<> TransferBuffers;

    } // namespace stream
} // namespace util

#endif // _UTIL_STREAM_TRANSFER_BUFFERS_H_
