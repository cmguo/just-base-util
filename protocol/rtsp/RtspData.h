// RtspData.h

#ifndef _UTIL_PROTOCOL_RTSP_RTSP_DATA_H_
#define _UTIL_PROTOCOL_RTSP_RTSP_DATA_H_

namespace util
{
    namespace protocol
    {

        struct RtspDataHeader
        {
        public:
            boost::uint8_t dollar;
            boost::uint8_t interleaved;
            boost::uint16_t length;

            static boost::uint8_t const HEAD_SIZE = 4;

        public:
            RtspDataHeader()
                : dollar('$')
                , interleaved(0)
                , length(0)
            {
            }
            
        public:
            template <typename Archive>
            void serialize(
                Archive & ar)
            {
                ar & dollar;
                ar & interleaved;
                ar & length;
            }
        };

    } // namespace protocol
} // namespace util

#endif // _UTIL_PROTOCOL_RTSP_RTSP_DATA_H_
