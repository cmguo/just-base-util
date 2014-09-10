// RtspPacket.hpp

#ifndef _UTIL_PROTOCOL_RTSP_RTSP_PACKET_HPP_
#define _UTIL_PROTOCOL_RTSP_RTSP_PACKET_HPP_

#include "util/protocol/rtsp/RtspPacket.h"
#include "util/protocol/mine/MineHead.hpp"
#include "util/archive/ArchiveBuffer.h"
#include "util/buffers/StreamBuffer.h"

namespace util
{
    namespace protocol
    {

        template <typename Archive>
        void RtspPacket::load(
            Archive & ar)
        {
            util::buffers::StreamBuffer<boost::uint8_t> & buf = 
                static_cast<util::buffers::StreamBuffer<boost::uint8_t> &>(*ar.rdbuf());
            util::archive::ArchiveBuffer<char> abuf(buf.data());
            std::istream is(&abuf);
            head_->set_content(is);
            if (!is) {
                ar.fail();
            }
            std::ostream os(&data_);
            os << &abuf;
            buf.consume(buf.size());
        }

        template <typename Archive>
        void RtspPacket::save(
            Archive & ar) const
        {
            util::buffers::StreamBuffer<boost::uint8_t> & buf = 
                static_cast<util::buffers::StreamBuffer<boost::uint8_t> &>(*ar.rdbuf());
            util::archive::ArchiveBuffer<char> abuf(buf.prepare(4096));
            std::ostream os(&abuf);
            head_->get_content(os);
            util::archive::ArchiveBuffer<char> abuf2(data_.data());
            os << &abuf2;
            buf.commit(abuf.size());
        }

    } // namespace protocol
} // namespace util

#endif // _UTIL_PROTOCOL_RTSP_RTSP_PACKET_HPP_
