// RtspPacket.h

#ifndef _UTIL_PROTOCOL_RTSP_RTSP_PACKET_H_
#define _UTIL_PROTOCOL_RTSP_RTSP_PACKET_H_

#include "util/protocol/rtsp/RtspHead.h"

#include <util/serialization/SplitMember.h>
#include <util/archive/ArchiveBuffer.h>

#include <boost/asio/streambuf.hpp>

namespace util
{
    namespace protocol
    {

        class RtspPacket
        {
        protected:
            RtspPacket(
                RtspHead & head);

            RtspPacket(
                RtspPacket const & r, 
                RtspHead & head);

            RtspPacket & operator=(
                RtspPacket const & r);

        public:
            RtspHead & head()
            {
                return *head_;
            }

            boost::asio::streambuf & data()
            {
                return data_;
            }

            void clear_data()
            {
                data_.reset();
            }

            SERIALIZATION_SPLIT_MEMBER();

            template <typename Archive>
            void load(
                Archive & ar)
            {
                StreamBuffer & buf = static_cast<StreamBuffer &>(*ar.rdbuf());
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
            void save(
                Archive & ar) const
            {
                StreamBuffer & buf = static_cast<StreamBuffer &>(*ar.rdbuf());
                util::archive::ArchiveBuffer<char> abuf(buf.prepare(4096));
                std::ostream os(&abuf);
                head_->get_content(os);
                os << const_cast<boost::asio::streambuf *>(&data_);
                buf.commit(abuf.size());
            }

        private:
            RtspHead * head_;
            boost::asio::streambuf data_;
        };

    } // namespace protocol
} // namespace util

#endif // _UTIL_PROTOCOL_RTSP_RTSP_PACKET_H_
