// CMsgPacket.hpp

#ifndef _UTIL_PROTOCOL_CMSG_CMSG_PACKET_HPP_
#define _UTIL_PROTOCOL_CMSG_CMSG_PACKET_HPP_

#include "util/protocol/cmsg/CMsgPacket.h"
#include "util/archive/ArchiveBuffer.h"
#include "util/archive/Ctype.h"
#include "util/buffers/StreamBuffer.h"

namespace util
{
    namespace protocol
    {

        template <typename Archive>
        void CMsgPacket::load(
            Archive & ar)
        {
            typedef typename Archive::char_type char_type;
            typedef typename Archive::traits_type traits_type;
            util::buffers::StreamBuffer<char_type, traits_type> buf;
            {
                 std::basic_ostream<char_type, traits_type> os(&buf);
                 os << std::noskipws << ar.rdbuf();
            }
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
        void CMsgPacket::save(
            Archive & ar) const
        {
            typedef typename Archive::char_type char_type;
            typedef typename Archive::traits_type traits_type;
            util::buffers::StreamBuffer<char_type, traits_type> buf;
            util::archive::ArchiveBuffer<char> abuf(buf.prepare(4096));
            std::ostream os(&abuf);
            head_->get_content(os);
            util::archive::ArchiveBuffer<char> abuf2(data_.data());
            os << &abuf2;
            buf.commit(abuf.size());
            {
                 std::basic_istream<char_type, traits_type> is(&buf);
                 is >> std::noskipws >> ar.rdbuf();
            }
        }

    } // namespace protocol
} // namespace util

#endif // _UTIL_PROTOCOL_CMSG_CMSG_PACKET_HPP_
