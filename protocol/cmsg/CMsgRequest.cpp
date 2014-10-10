// CMsgRequest.cpp

#include "util/Util.h"
#include "util/protocol/cmsg/CMsgRequest.h"
#include "util/protocol/cmsg/CMsgError.h"

#include <sstream>

namespace util
{
    namespace protocol
    {

        CMsgRequestHead::CMsgRequestHead(
            size_t method, 
            std::string const & path, 
            size_t version)
            : method(method)
            , path(path)
            , version(version)
            , protocol_(NULL)
        {
        }

        bool CMsgRequestHead::get_line(
            std::string & line) const
        {
            std::ostringstream oss;
            oss << protocol_->method_strings[method];
            oss << " ";
            oss << path;
            oss << " " << protocol_->name << "/";
            oss << (version >> 8);
            oss << ".";
            oss << (version & 0xff);
            line = oss.str();
            return true;
        }

        bool CMsgRequestHead::set_line(
            std::string const & line)
        {
            std::string str;
            std::istringstream iss(line);
            iss >> str;
            size_t i = 0;
            for (; protocol_->method_strings[i]; ++i) {
                if (protocol_->method_strings[i] == str) {
                    break;
                }
            }
            method = i;
            iss.ignore(1);
            iss >> path;
            iss.ignore(1);
            int tmp;
            iss.ignore(5); // RTSP
            iss >> tmp;
            version = tmp << 8;
            iss.ignore(1);
            iss >> tmp;
            version |= tmp & 0xff;
            return !!iss;
        }

        CMsgRequest::CMsgRequest(
            CMsgRequestHead & head)
            : CMsgPacket(head)
            , head_(head_)
        {
        }

        CMsgRequest::CMsgRequest(
            CMsgRequest const & r, 
            CMsgRequestHead & head)
            : CMsgPacket(r, head)
            , head_(head)
        {
        }

    } // namespace protocol
} // namespace util
