// CMsgField.h

#ifndef _UTIL_PROTOCOL_CMSG_CMSG_FIELD_H_
#define _UTIL_PROTOCOL_CMSG_CMSG_FIELD_H_

#include <framework/string/Format.h>
#include <framework/string/Parse.h>

namespace util
{
    namespace protocol
    {

        template <typename Field>
        class CMsgField
        {
        public:
            typedef std::map<std::string, std::string> fields_map;

            typedef Field value_t;

        public:
            CMsgField(
                fields_map & map, 
                char const * name)
                : map_(map)
                , name_(name)
            {
            }

        public:
            value_t get_or(
                value_t def) const
            {
                using framework::string::from_string;
                fields_map::const_iterator iter = map_.find(name_);
                value_t value(def);
                if (iter != map_.end()) {
                    from_string(iter->second, value);
                }
                return value;
            }

            value_t get() const
            {
                return get_or(value_t());
            }

            bool is_set() const
            {
                return map_.find(name_) != map_.end();
            }

            void set(
                value_t value)
            {
                using framework::string::to_string;
                to_string(map_[name_], value);
            }

            void reset(
                value_t value)
            {
                set(value);
            }

            void reset()
            {
                map_.erase(name_);
            }

        public:
            operator value_t() const
            {
                return get();
            }

            CMsgField & operator=(
                value_t value)
            {
                set(value);
                return *this;
            }

        private:
            fields_map & map_;
            char const * name_;
        };

    } // namespace protocol
} // namespace util

#endif // _UTIL_PROTOCOL_CMSG_CMSG_FIELD_H_
