// Observable.h

#ifndef _UTIL_EVENT_OBSERVABLE_H_
#define _UTIL_EVENT_OBSERVABLE_H_

#include "util/event/Event.h"

namespace util
{
    namespace event
    {

        class Observable
        {
        public:
            Observable(
                std::string const & name);

        public:
            void register_event(
                Event & event);

            template <typename T>
            void on(
                std::string const & e, 
                T const & l)
            {
                std::map<std::string, Event *>::const_iterator iter =
                    events_.find(e);
                if (iter != events_.end())
                    iter->second->on(l);
            }

            template <typename T>
            void un(
                std::string const & e, 
                T const & l)
            {
                std::map<std::string, Event *>::const_iterator iter =
                    events_.find(e);
                if (iter != events_.end())
                    iter->second->un(l);
            }

        public:
            static Observable & get(
                std::string const & name);

        public:
            friend bool operator==(
                Observable const & l, 
                Observable const & r)
            {
                return &l == &r;
            }

        protected:
            void raise(
                Event const & e) const;

        private:
            std::map<std::string, Event *> events_;
        };

    } // namespace event
} // namespace util

#endif // _UTIL_EVENT_OBSERVABLE_H_
