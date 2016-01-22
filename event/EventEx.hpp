// EventEx.hpp

#ifndef _UTIL_EVENT_EVENT_EX_HPP_
#define _UTIL_EVENT_EVENT_EX_HPP_

#include "util/event/EventEx.h"
#include "util/datagraph/WalkArchive.h"
#include "framework/string/Format.h"
#include "framework/string/FormatStl.h"

namespace util
{
    namespace event
    {

        class EventValue
            : public util::datagraph::WalkArchive<EventValue>
        {
        public:
            template <typename T>
            std::string get(
                T const & t,
                std::string const & key)
            {
                value_.clear();
                set_path(key);
                (*this) << t;
                return value_;
            }

            template <typename T>
            void found(
                T const & t)
            {
                value_ = framework::string::format(t);
            }

        private:
            std::string value_;
        };

        template <typename E>
        std::string EventEx<E>::get_value(
            std::string const & key) const
        {
            return EventValue().get(static_cast<E const &>(*this), key);
        }

    } // namespace event
} // namespace util

namespace util
{
    namespace serialization
    {

        template<
            class _T
        >
        struct is_stringlized<util::event::EventValue, _T>
            : has_to_string<_T>
        {
        };

    }
}

#endif // _UTIL_EVENT_EVENT_EX_HPP_
