// Event.h

#ifndef _UTIL_EVENT_EVENT_H_
#define _UTIL_EVENT_EVENT_H_

namespace util
{
    namespace event
    {

        struct EventId
        {
        };

        class Observable;

        class Event
        {
        public:
            virtual ~Event() {}

        public:
            template <typename E>
            E * as()
            {
                assert(&E::id == id_);
                return (&E::id == id_) ? (E *)(this) : NULL;
            }

            template <typename E>
            E const * as() const
            {
                assert(&E::id == id_);
                return (&E::id == id_) ? (E const *)(this) : NULL;
            }

        protected:
            Event(
                EventId const & id)
                : id_(&id)
            {
            }

        private:
            friend class Observable;

            EventId const * id_;
        };

        template <typename E>
        class EventBase
            : public Event
        {
        public:
            static EventId id;

        public:
            EventBase()
                : Event(id)
            {
            }
        };

        template <typename E>
        EventId EventBase<E>::id;

    } // namespace event
} // namespace util

#endif // _UTIL_EVENT_EVENT_H_
