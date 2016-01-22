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

            std::string const & name() const
            {
                return name_;
            }

            virtual std::string get_value(
                std::string const & key) const;

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

            std::string operator[](
                std::string const & key)
            {
                return get_value(key);
            }

        protected:
            Event(
                char const * name = "");

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
            std::string name_;
            std::list<EventListener> listeners_;
        };

    } // namespace event
} // namespace util

#endif // _UTIL_EVENT_EVENT_H_
