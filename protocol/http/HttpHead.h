// HttpHead.h

#ifndef _UTIL_PROTOCOL_HTTP_HTTP_HEAD_H_
#define _UTIL_PROTOCOL_HTTP_HTTP_HEAD_H_

#include "util/protocol/http/HttpStringField.h"
#include "util/protocol/http/HttpFieldRange.h"
#include "util/protocol/http/HttpFieldConnection.h"

#include <boost/optional.hpp>

#include <istream>
#include <ostream>

namespace util
{
    namespace protocol
    {

        class HttpHead
        {
        public:
            typedef std::map<std::string, std::vector<std::string> > fields_map;
            typedef fields_map::const_iterator fields_map_iterator;
            typedef fields_map::value_type field_type;

        public:
            HttpHead();

            HttpHead(
                size_t version);

            virtual ~HttpHead();

        public:
            void add_fields(
                fields_map const & fields);

        public:
            void set_field(
                std::string const & name, 
                std::string const & value);

            void delete_field(
                std::string const & name);

            HttpStringField operator[](
                std::string const & name)
            {
                HttpStringField field(name);
                get_field(field);
                return field;
            }

        public:
            void get_content(
                std::ostream & os) const;

            void set_content(
                std::istream & is);

            void get_content(
                std::ostream & os, 
                boost::system::error_code & ec) const;

            void set_content(
                std::istream & is, 
                boost::system::error_code & ec);

        public:
            static size_t const version_1_0 = 0x100;
            static size_t const version_1_1 = 0x101;

        public:
            size_t version;

            boost::optional<boost::uint64_t> content_length;
            boost::optional<std::string> host;
            boost::optional<std::string> content_encoding;
            boost::optional<std::string> transfer_encoding;
            boost::optional<std::string> location;
            boost::optional<http_field::Range> range;
            boost::optional<http_field::ContentRange> content_range;
            boost::optional<http_field::Connection> connection;
            std::vector<std::string> pragma;

        private:
            void check_handler(
                HttpStringField & field);

            void set_field(
                HttpStringField const & field);

            void get_field(
                HttpStringField & field) const;

        private:
            virtual bool get_line(
                std::string & line) const = 0;

            virtual bool set_line(
                std::string const & line) = 0;

        private:
            fields_map other_fields_;
        };

    } // namespace protocol
} // namespace util

#endif // _UTIL_PROTOCOL_HTTP_HTTP_HEAD_H_
