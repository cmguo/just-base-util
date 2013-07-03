// Observable.h

#ifndef _UTIL_EVENT_OBSERVABLE_H_
#define _UTIL_EVENT_OBSERVABLE_H_

namespace util
{
    namespace event
    {

        class Event;

        class Observable
        {
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
        };

    } // namespace event
} // namespace util

#endif // _UTIL_EVENT_OBSERVABLE_H_
