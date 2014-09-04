// RtspPacket.cpp

#include "util/Util.h"
#include "util/protocol/rtsp/RtspRequest.h"
#include "util/protocol/rtsp/RtspError.h"

using namespace framework::system::logic_error;

#include <sstream>

namespace framework
{
    namespace string
    {
        std::string to_string(
            util::protocol::RtspRequestHead::MethodEnum const & e)
        {
            return util::protocol::RtspRequestHead::method_str[e];
        }

        boost::system::error_code from_string(
            util::protocol::RtspRequestHead::MethodEnum & e, 
            std::string const & str)
        {
            for (int i = 0; i < util::protocol::RtspRequestHead::invalid_method; ++i) {
                if (util::protocol::RtspRequestHead::method_str[i] == str) {
                    e = (util::protocol::RtspRequestHead::MethodEnum)i;
                    return succeed;
                }
            }
            return invalid_argument;
        }
    }
}

namespace util
{
    namespace protocol
    {

        std::string const RtspRequestHead::method_str[] = {
            "DESCRIBE", 
            "ANNOUNCE", 
            "GET_PARAMETER", 
            "SET_PARAMETER", 
            "OPTIONS", 
            "REDIRECT", 
            "SETUP", 
            "PLAY", 
            "RECORD", 
            "PAUSE", 
            "TEARDOWN", 
            "invalid_method"
        };

        RtspRequestHead::RtspRequestHead(
            MethodEnum method)
            : method(method)
        {
            version = 0x00000100; // 1.0
        }

        RtspRequestHead::RtspRequestHead(
            MethodEnum method, 
            std::string const & path, 
            size_t version)
            : method(method)
            , path(path)
            , version(version)
        {
        }

        bool RtspRequestHead::get_line(
            std::string & line) const
        {
            std::ostringstream oss;
            oss << method_str[method];
            oss << " ";
            oss << path;
            oss << " RTSP/";
            oss << (version >> 8);
            oss << ".";
            oss << (version & 0xff);
            line = oss.str();
            return true;
        }

        bool RtspRequestHead::set_line(
            std::string const & line)
        {
            std::string str;
            std::istringstream iss(line);
            iss >> str;
            size_t i = 0;
            for (; i < invalid_method; ++i) {
                if (method_str[i] == str) {
                    break;
                }
            }
            method = (MethodEnum)i;
            iss.ignore(1);
            iss >> path;
            iss.ignore(1);
            int tmp;
            iss.ignore(5);
            iss >> tmp;
            version = tmp << 8;
            iss.ignore(1);
            iss >> tmp;
            version |= tmp & 0xff;
            return !!iss;
        }

        RtspRequest::RtspRequest()
            : RtspPacket(head_)
        {
        }

        RtspRequest::RtspRequest(
            RtspRequest const & r)
            : RtspPacket(r, head_)
            , head_(r.head_)
        {
        }

    } // namespace protocol
} // namespace util
