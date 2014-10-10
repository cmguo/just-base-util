// CMsgPacket.h

#ifndef _UTIL_PROTOCOL_CMSG_CMSG_PACKET_H_
#define _UTIL_PROTOCOL_CMSG_CMSG_PACKET_H_

#include "util/protocol/cmsg/CMsgHead.h"

#include <util/serialization/Serialization.h>
#include <util/serialization/SplitMember.h>

#include <boost/asio/streambuf.hpp>

namespace util
{
    namespace protocol
    {

        class CMsgPacket
        {
        protected:
            CMsgPacket(
                CMsgHead & head);

            CMsgPacket(
                CMsgPacket const & r, 
                CMsgHead & head);

        public:
            CMsgHead & head()
            {
                return *head_;
            }

            CMsgHead const & head() const
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

            void clear()
            {
                head_->clear();
                clear_data();
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
            CMsgPacket & operator=(
                CMsgPacket const & r);

        private:
            CMsgHead * head_;
            boost::asio::streambuf data_;
        };

    } // namespace protocol
} // namespace util

#endif // _UTIL_PROTOCOL_CMSG_CMSG_PACKET_H_
