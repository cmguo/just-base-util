// EventEx.h

#ifndef _UTIL_EVENT_EVENT_EX_H_
#define _UTIL_EVENT_EVENT_EX_H_

#include "util/event/Event.h"

namespace util
{
    namespace event
    {

        template <typename E>
        class EventEx
            : public Event
        {
        protected:
            EventEx(
                char const * name)
                : Event(name)
            {
            }

        private:
            virtual std::string get_value(
                std::string const & key) const;
        };

    } // namespace event
} // namespace util

#endif // _UTIL_EVENT_EVENT_EX_H_
