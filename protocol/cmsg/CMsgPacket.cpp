// CMsgPacket.cpp

#include "util/Util.h"
#include "util/protocol/cmsg/CMsgPacket.h"

namespace util
{
    namespace protocol
    {

        CMsgPacket::CMsgPacket(
            CMsgHead & head)
            : head_(&head)
        {
        }

        CMsgPacket::CMsgPacket(
            CMsgPacket const & r, 
            CMsgHead & head)
            : head_(&head)
        {
            data_.reset();
            memcpy(
                boost::asio::buffer_cast<void *>(data_.prepare(r.data_.size())), 
                boost::asio::buffer_cast<void const *>(r.data_.data()), 
                r.data_.size());
            data_.commit(r.data_.size());
        }

    } // namespace protocol
} // namespace util
