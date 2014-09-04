// MineHeadT.h

#ifndef _UTIL_PROTOCOL_MINE_MINE_HEAD_H_
#define _UTIL_PROTOCOL_MINE_MINE_HEAD_H_

#include "util/protocol/mine/MineStringField.h"

#include <boost/optional.hpp>

#include <istream>
#include <ostream>

namespace util
{
    namespace protocol
    {

        struct MineHead
        {
        public:
            typedef std::map<std::string, std::vector<std::string> > fields_map;
            typedef fields_map::const_iterator fields_map_iterator;
            typedef fields_map::value_type field_type;

        public:
            boost::optional<boost::uint64_t> content_length;
            boost::optional<std::string> host;
            boost::optional<std::string> location;
            std::vector<std::string> pragma;

        public:
            MineHead() {};

            virtual ~MineHead() {}

        public:
            template <typename Archive>
            void serialize(
                Archive & ar);

        protected:
            virtual bool get_line(
                std::string & line) const = 0;

            virtual bool set_line(
                std::string const & line) = 0;

        protected:
            fields_map other_fields_;
        };

        template <
            typename Head
        >
        class MineHeadT
            : public MineHead
        {
        public:
            void add_fields(
                fields_map const & fields);

        public:
            void set_field(
                std::string const & name, 
                std::string const & value);

            void delete_field(
                std::string const & name);

            MineStringField operator[](
                std::string const & name)
            {
                MineStringField field(name);
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

        private:
            Head & This()
            {
                return static_cast<Head &>(*this);
            }

            Head const & This() const
            {
                return static_cast<Head const &>(*this);
            }

        private:
            void check_handler(
                MineStringField & field);

            void set_field(
                MineStringField const & field);

            void get_field(
                MineStringField & field) const;
        };

    } // namespace protocol
} // namespace util

#endif // _UTIL_PROTOCOL_MINE_MINE_HEAD_H_
