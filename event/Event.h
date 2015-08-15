// Event.h

#ifndef _UTIL_EVENT_EVENT_H_
#define _UTIL_EVENT_EVENT_H_

#include "util/event/EventListener.h"

namespace util
{
    namespace event
    {

        class Event
        {
        public:
            virtual ~Event() {}

        public:
            template <typename T>
            void on(
                T const & l)
            {
                on(EventListener(l));
            }

            template <typename T>
            void un(
                T const & l)
            {
                un(EventListener(l));
            }

        public:
            friend bool operator==(
                Event const & l, 
                Event const & r)
            {
                return &l == &r;
            }

            Event & operator+=(
                EventListener const & l)
            {
                on(l);
                return *this;
            }

            Event & operator-=(
                EventListener const & l)
            {
                un(l);
                return *this;
            }

        protected:
            Event()
            {
            }

        private:
            void on(
                EventListener const & l);

            void un(
                EventListener const & l);

        private:
            // noncopyable
            Event(
                Event const &);

            Event & operator=(
                Event const &);

        private:
            friend class Observable;

            void raise(
                Observable const & sender) const;

        private:
            std::list<EventListener> listeners_;
        };

    } // namespace event
} // namespace util

#endif // _UTIL_EVENT_EVENT_H_
