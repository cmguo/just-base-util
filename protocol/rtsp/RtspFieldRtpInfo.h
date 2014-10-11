// RtspFieldRtpInfo.h

#ifndef _UTIL_PROTOCOL_RTSP_RTSP_FIELD_RTP_INFO_H_
#define _UTIL_PROTOCOL_RTSP_RTSP_FIELD_RTP_INFO_H_

#include <framework/string/Slice.h>

#include <iterator>

namespace util
{
    namespace protocol
    {
        namespace rtsp_field
        {

            struct RtpInfoItem
            {
                std::string url;
                boost::uint32_t seq;
                boost::uint32_t rtptime;

                RtpInfoItem()
                    : seq(0)
                    , rtptime(0)
                {
                }

                bool from_string(
                    std::string const & str)
                {
                    using framework::string::map_find;
                    map_find(str, "url", url, ";");
                    map_find(str, "seq", seq, ";");
                    map_find(str, "rtptime", rtptime, ";");
                    return true;
                }

                std::string to_string() const
                {
                    std::ostringstream oss;
                    oss << "url=" << url
                        << ";seq=" << seq
                        << ";rtptime=" << rtptime;
                    return oss.str();
                }
            };
            
            class RtpInfo
                : public std::vector<RtpInfoItem>
            {
            public:
                bool from_string(
                    std::string const & str)
                {
                    std::vector<std::string> items;
                    using framework::string::slice;
                    slice<std::string>(str, std::back_inserter(items), ",");
                    resize(items.size());
                    for (size_t i = 0; i < items.size(); ++i) {
                        at(i).from_string(items[i]);
                    }
                    return true;
                }

                std::string to_string() const
                {
                    std::ostringstream oss;
                    for (size_t i = 0; i < size(); ++i) {
                        if (i > 0)
                            oss << ",";
                        oss << at(i).to_string();
                    }
                    return oss.str();
                }
            };
            
        }

    } // namespace protocol
} // namespace util

#endif // _UTIL_PROTOCOL_RTSP_RTSP_FIELD_RTP_INFO_H_
