// RtmpChunkHeader.h

#ifndef _UTIL_PROTOCOL_RTMP_RTMP_CHUNK_HEADER_H_
#define _UTIL_PROTOCOL_RTMP_RTMP_CHUNK_HEADER_H_

#include <util/serialization/NumberBits24.h>

#include <framework/system/NumberBits24.h>

namespace util
{
    namespace protocol
    {

        struct RtmpChunkBasicHeader
        {
        public:
            union
            {
#ifdef   BOOST_BIG_ENDIAN
                boost::uint8_t cs_id0 : 6;
                boost::uint8_t fmt : 2;
#else
                boost::uint8_t fmt : 2;
                boost::uint8_t cs_id0 : 6;
#endif
                boost::uint8_t one_byte;
            };
            boost::uint8_t cs_id1;
            boost::uint16_t cs_id2;

        public:
            RtmpChunkBasicHeader(
                boost::uint8_t fmt = 0, 
                boost::uint16_t cs_id = 2)
                : one_byte((fmt << 6) | ((boost::uint8_t)cs_id & 0x3f))
                , cs_id1(0)
                , cs_id2(0)
            {
                if (cs_id0 != cs_id) {
                    cs_id1 = (boost::uint8_t)cs_id;
                    if (cs_id1 != cs_id) {
                        cs_id0 = 1;
                        cs_id1 = 0;
                        cs_id2 = cs_id;
                    } else {
                        cs_id0 = 0;
                    }
                }
            }
            
            boost::uint8_t size() const
            {
                return cs_id0 == 0 ? 2 : (cs_id0 == 1 ? 3 : 1);
            }

            boost::uint16_t cs_id() const
            {
                return cs_id0 == 0 ? cs_id1 : (cs_id0 == 1 ? cs_id2 : cs_id0);
            }

            template <typename Archive>
            void serialize(
                Archive & ar)
            {
                ar & one_byte;
                if (cs_id0 == 0) {
                    ar & cs_id1;
                } else if (cs_id0 == 1) {
                    ar & cs_id2;
                }
            }
        };

        struct RtmpChunkMessageHeader
        {
            framework::system::UInt24 timestamp; // timestamp or timestamp delta
            framework::system::UInt24 message_length;
            boost::uint8_t message_type_id;
            boost::uint32_t message_stream_id;

            RtmpChunkMessageHeader()
                : message_type_id(0)
                , message_stream_id(0)
            {

            }

            template <typename Archive>
            void serialize(
                Archive & ar)
            {
                boost::uint16_t fmt = 
                    (boost::uint16_t)(intptr_t)ar.context();
                switch (fmt)
                {
                case 0:
                    ar & timestamp;
                    ar & message_length;
                    ar & message_type_id;
                    ar & message_stream_id;
                    break;
                case 1:
                    ar & timestamp;
                    ar & message_length;
                    ar & message_type_id;
                    break;
                case 2:
                    ar & timestamp;
                    break;
                case 3:
                    break;
                }
            }
        };

        struct RtmpChunkHeader
            : RtmpChunkBasicHeader
            , RtmpChunkMessageHeader
        {
            boost::uint32_t extended_timestamp;
            boost::uint32_t calc_timestamp;

            RtmpChunkHeader()
                : extended_timestamp(0)
                , calc_timestamp(0)
            {
            }

            boost::uint32_t real_timestamp() const
            {
                return timestamp == 0x00ffffff ? extended_timestamp : (boost::uint32_t)timestamp;
            }

            RtmpChunkHeader const & add(
                RtmpChunkHeader const & r)
            {
                switch (r.fmt) {
                case 0:
                    timestamp = r.timestamp;
                    message_length = r.message_length;
                    message_type_id = r.message_type_id;
                    message_stream_id = r.message_stream_id;
                    extended_timestamp = r.extended_timestamp;
                    calc_timestamp = r.real_timestamp();
                    break;
                case 1:
                    timestamp = r.timestamp;
                    message_length = r.message_length;
                    message_type_id = r.message_type_id;
                    extended_timestamp = r.extended_timestamp;
                    calc_timestamp += r.real_timestamp();
                    break;
                case 2:
                    timestamp = r.timestamp;
                    extended_timestamp = r.extended_timestamp;
                    calc_timestamp += r.real_timestamp();
                    break;
                case 3:
                    calc_timestamp += real_timestamp();
                    break;
                }
                return *this;
            }

            RtmpChunkHeader const & dec(
                RtmpChunkHeader const & r)
            {
                if (r.message_stream_id == message_stream_id) {
                    if (r.message_type_id == message_type_id) {
                        if (r.message_length == message_length) {
                            if (r.calc_timestamp == calc_timestamp + real_timestamp()) {
                                fmt = 3;
                            } else {
                                fmt = 2;
                                extended_timestamp = r.calc_timestamp - calc_timestamp;
                            }
                        }
                    } else {
                        fmt = 1;
                        message_length = r.message_length;
                        message_type_id = r.message_type_id;
                        extended_timestamp = r.calc_timestamp - calc_timestamp;
                    }
                } else {
                    fmt = 0;
                    message_length = r.message_length;
                    message_type_id = r.message_type_id;
                    message_stream_id = r.message_stream_id;
                    extended_timestamp = r.calc_timestamp;
                }
                calc_timestamp = r.calc_timestamp;
                if (extended_timestamp & 0xff000000) {
                    timestamp = 0x00ffffff;
                } else {
                    timestamp = extended_timestamp;
                }
                return *this;
            }

            template <typename Archive>
            void serialize(
                Archive & ar)
            {
                RtmpChunkBasicHeader::serialize(ar);
                void * ctx = ar.context();
                ar.context((void *)fmt);
                RtmpChunkMessageHeader::serialize(ar);
                ar.context(ctx);

                if (timestamp == 0x00ffffff) {
                    ar & extended_timestamp;
                } else {
                    extended_timestamp = timestamp;
                }
            }
        };

    } // namespace protocol
} // namespace util

#endif // _UTIL_PROTOCOL_RTMP_RTMP_CHUNK_HEADER_H_
