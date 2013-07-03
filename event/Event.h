// Event.h

#ifndef _UTIL_EVENT_EVENT_H_
#define _UTIL_EVENT_EVENT_H_

#include <boost/function.hpp>

namespace util
{
    namespace event
    {

        class Observable;
        class Event;

        typedef boost::function<void (
            Observable const & sender, 
            Event const &)
        > Listener;

        class Event
        {
        public:
            virtual ~Event() {}

        public:
            void on(
                Listener const & l);

            void un(
                Listener const & l);

        public:
            friend bool operator==(
                Event const & l, 
                Event const & r)
            {
                return &l == &r;
            }

        protected:
            Event()
            {
            }

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
            std::list<Listener> listeners_;
        };

    } // namespace event
} // namespace util

#endif // _UTIL_EVENT_EVENT_H_
