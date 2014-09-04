// RtspPacket.cpp

#include "util/Util.h"
#include "util/protocol/rtsp/RtspResponse.h"

#include <sstream>

namespace util
{
    namespace protocol
    {

        RtspResponseHead::RtspResponseHead(
            size_t err_code, 
            size_t version)
            : version(version)
            , err_code(err_code)
        {
        }

        RtspResponseHead::RtspResponseHead(
            size_t err_code, 
            std::string const & err_msg, 
            size_t version)
            : version(version)
            , err_code(err_code)
            , err_msg(err_msg)
        {
        }

        bool RtspResponseHead::get_line(
            std::string & line) const
        {
            std::ostringstream oss;
            oss << "RTSP/";
            oss << (version >> 8);
            oss <<  ".";
            oss << (version & 0xff);
            oss << " ";
            oss << err_code;
            oss <<  " ";
            oss << err_msg;
            line = oss.str();
            return true;
        }

        bool RtspResponseHead::set_line(
            std::string const & line)
        {
            std::istringstream iss(line);
            iss.ignore(4); // RTSP/1.0
            int tmp;
            if (iss.get() == '/') {
                iss >> tmp;
                version = tmp << 8;
                iss.ignore(1);
                iss >> tmp;
                version |= tmp & 0xff;
                iss.ignore(1);
            }
            iss >> err_code;
            iss.ignore(1);
            std::getline(iss, err_msg);
            return !!iss;
        }

        RtspResponse::RtspResponse()
            : RtspPacket(head_)
        {
        }

        RtspResponse::RtspResponse(
            RtspResponse const & r)
            : RtspPacket(r, head_)
            , head_(r.head_)
        {
        }

    } // namespace protocol
} // namespace util
