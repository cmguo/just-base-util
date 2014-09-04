// RtspPacket.cpp

#include "util/Util.h"
#include "util/protocol/rtsp/RtspPacket.h"

namespace util
{
    namespace protocol
    {

        RtspPacket::RtspPacket(
            RtspHead & head)
            : head_(&head)
        {
        }

        RtspPacket::RtspPacket(
            RtspPacket const & r, 
            RtspHead & head)
            : head_(&head)
        {
            *this = r;
        }

        RtspPacket & RtspPacket::operator=(
            RtspPacket const & r)
        {
            // head_²»±ä
            data_.reset();
            memcpy(
                boost::asio::buffer_cast<void *>(data_.prepare(r.data_.size())), 
                boost::asio::buffer_cast<void const *>(r.data_.data()), 
                r.data_.size());
            data_.commit(r.data_.size());
            return *this;
        }

    } // namespace protocol
} // namespace util
