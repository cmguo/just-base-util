// CMsgResponse.cpp

#include "util/Util.h"
#include "util/protocol/cmsg/CMsgResponse.h"

#include <sstream>

namespace util
{
    namespace protocol
    {

        CMsgResponseHead::CMsgResponseHead(
            size_t version, 
            size_t err_code, 
            std::string const & err_msg)
            : version(version)
            , err_code(err_code)
            , err_msg(err_msg)
        {
        }

        bool CMsgResponseHead::get_line(
            std::string & line) const
        {
            std::ostringstream oss;
            oss << protocol_->name << "/";
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

        bool CMsgResponseHead::set_line(
            std::string const & line)
        {
            std::istringstream iss(line);
            iss.ignore(4); // RTSP
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

        CMsgResponse::CMsgResponse(
            CMsgResponseHead & head)
            : CMsgPacket(head)
            , head_(head)
        {
        }

        CMsgResponse::CMsgResponse(
            CMsgResponse const & r, 
            CMsgResponseHead & head)
            : CMsgPacket(r, head)
            , head_(head)
        {
        }

    } // namespace protocol
} // namespace util
