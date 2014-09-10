// RtspPacket.h

#ifndef _UTIL_PROTOCOL_RTSP_RTSP_PACKET_H_
#define _UTIL_PROTOCOL_RTSP_RTSP_PACKET_H_

#include "util/protocol/rtsp/RtspHead.h"

#include <util/serialization/SplitMember.h>

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

            RtspHead const & head() const
            {
                return *head_;
            }

            boost::asio::streambuf & data()
            {
                return data_;
            }

            boost::asio::streambuf const & data() const
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
                Archive & ar);

            template <typename Archive>
            void save(
                Archive & ar) const;

        private:
            RtspHead * head_;
            boost::asio::streambuf data_;
        };

    } // namespace protocol
} // namespace util

#endif // _UTIL_PROTOCOL_RTSP_RTSP_PACKET_H_
