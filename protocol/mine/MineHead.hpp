// MineHead.hpp

#ifndef _UTIL_PROTOCOL_MINE_MINE_HEAD_HPP_
#define _UTIL_PROTOCOL_MINE_MINE_HEAD_HPP_

#include "util/protocol/mine/MineHead.h"
#include "util/protocol/mine/MineError.h"
#include "util/protocol/mine/MineHeadIArchive.h"
#include "util/protocol/mine/MineHeadOArchive.h"
#include "util/protocol/mine/MineHeadVisitor.h"

#include "util/serialization/stl/vector.h"

namespace util
{
    namespace protocol
    {

        template <typename Archive>
        void inline MineHead::serialize(
            Archive & ar)
        {
            using namespace util::serialization;
            ar 
                & SERIALIZATION_NVP_NAME("Host", host)
                & SERIALIZATION_NVP_NAME("Content-Length", content_length)
                & SERIALIZATION_NVP_NAME("Location", location)
                & SERIALIZATION_NVP_NAME("Pragma", pragma)
                & other_fields_;
        }

        template <typename Head>
        void MineHeadT<Head>::add_fields(
            fields_map const & fields)
        {
            if (!fields.empty()) {
                MineHeadIArchive ia(fields);
                ia >> This();
            }
        }

        template <typename Head>
        void MineHeadT<Head>::set_field(
            std::string const & name, 
            std::string const & value)
        {
            MineStringField field(name, value);
            set_field(field);
        }

        template <typename Head>
        void MineHeadT<Head>::delete_field(
            std::string const & name)
        {
            MineStringField field(name);
            set_field(field);
        }

        template <typename Head>
        void MineHeadT<Head>::get_content(
            std::ostream & os) const
        {
            boost::system::error_code ec;
            get_content(os, ec);
        }

        template <typename Head>
        void MineHeadT<Head>::get_content(
            std::ostream & os, 
            boost::system::error_code & ec) const
        {
            std::string line;
            bool ret = get_line(line);
            (void)ret;
            assert(ret);
            os << line << "\r\n";
            {
                MineHeadOArchive oa(os);
                oa << This();
                assert(oa);
            }
            os << "\r\n";
            ec.clear();
        }

        template <typename Head>
        void MineHeadT<Head>::set_content(
            std::istream & is)
        {
            boost::system::error_code ec;
            set_content(is, ec);
            assert(!ec);
        }

        template <typename Head>
        void MineHeadT<Head>::set_content(
            std::istream & is, 
            boost::system::error_code & ec)
        {
            other_fields_.clear();
            std::string line;
            std::getline(is, line);
            assert(is);
            framework::string::trim(line);
            bool ret = set_line(line);
            if (!ret) {
                ec = mine_error::format_error;
                return;
            }
            MineHeadIArchive ia(is);
            ia >> This();
            if (!ia) {
                ec = mine_error::format_error;
                return;
            }
            assert(ia);
        }

        template <typename Head>
        std::auto_ptr<MineStringField::Handler> MineHeadT<Head>::handler(
            std::string const & name) const
        {
            MineHeadVisitor visitor(name);
            visitor.visit(const_cast<Head &>(This()));
            return visitor.handler();
        }

    } // namespace protocol
} // namespace util

#endif // _UTIL_PROTOCOL_MINE_MINE_HEAD_HPP_
