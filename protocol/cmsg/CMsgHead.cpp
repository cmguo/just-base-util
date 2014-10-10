// CMsgHead.cpp

#include "util/Util.h"
#include "util/protocol/cmsg/CMsgHead.h"
#include "util/protocol/cmsg/CMsgField.h"
#include "util/protocol/cmsg/CMsgError.h"
#define CMSG_DEFINE_FIELD
#include "util/protocol/cmsg/CMsgFields.h"

#include <framework/string/Algorithm.h>

#include <iostream>

namespace util
{
    namespace protocol
    {

        CMsgHead::CMsgHead()
        {
        }

        CMsgHead::~CMsgHead()
        {
        }

        CMsgHead & CMsgHead::operator=(
            CMsgHead const & r)
        {
            (std::map<std::string, std::string> &)(*this) = r;
            return *this;
        }

        std::string const & CMsgHead::operator[](
            std::string const & name) const
        {
            const_iterator iter = find(name);
            if (iter != end()) {
                return iter->second;
            }
            static std::string empty;
            return empty;
        }

        void CMsgHead::get_content(
            std::ostream & os) const
        {
            boost::system::error_code ec;
            get_content(os, ec);
        }

        void CMsgHead::get_content(
            std::ostream & os, 
            boost::system::error_code & ec) const
        {
            std::string line;
            bool ret = get_line(line);
            (void)ret;
            assert(ret);
            os << line << "\r\n";
            for (const_iterator i = begin(); i != end(); ++i) {
                os << i->first << ": " << i->second << "\r\n";
            }
            os << "\r\n";
            ec.clear();
        }

        void CMsgHead::set_content(
            std::istream & is)
        {
            boost::system::error_code ec;
            set_content(is, ec);
            assert(!ec);
        }

        void CMsgHead::set_content(
            std::istream & is, 
            boost::system::error_code & ec)
        {
            std::string line;
            std::getline(is, line);
            assert(is);
            framework::string::trim(line);
            bool ret = set_line(line);
            if (!ret) {
                ec = cmsg_error::format_error;
                return;
            }
            clear();
            std::string field;
            while (std::getline(is, field) && field != "\r") {
                framework::string::trim(field);
                if (field.empty())
                    break;
                std::string::size_type p = field.find(':');
                if (p != std::string::npos) {
                    std::string n = field.substr(0, p);
                    framework::string::trim(n);
                    std::string v = field.substr(p + 1);
                    framework::string::trim(v);
                    insert(std::make_pair(n, v));
                }
            }
            ec.clear();
        }

    } // namespace protocol
} // namespace util
