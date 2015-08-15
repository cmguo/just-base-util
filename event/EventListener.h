// EventListener.h

#ifndef _UTIL_EVENT_EVENT_LISTENER_H_
#define _UTIL_EVENT_EVENT_LISTENER_H_

#include <boost/function_equal.hpp>

#include <utility>
#include <memory>

namespace util
{
    namespace event
    {

        class Observable;
        class Event;

        class EventListener
        {
        public:
            typedef void result_t;

        public:
            EventListener()
            {
            }

            template <typename Listener>
            EventListener(
                Listener const & listener)
                : listener_(create(listener))
            {
            }

            EventListener(
                EventListener const & r)
                : listener_(const_cast<std::auto_ptr<ListenerBase> &>(r.listener_))
            {
            }

            ~EventListener()
            {
                if (listener_.get())
                    listener_.release()->free();
            }

        public:
            void operator()(
                Observable const & ob,
                Event const & event) const
            {
                return listener_->call(ob, event);
            }

            bool operator==(
                EventListener const & r)
            {
                return listener_.get()->equal(*r.listener_);
            }

        private:
            class ListenerBase
            {
            public:
                void call(
                    Observable const & ob,
                    Event const & event) const
                {
                    return call_(*this, ob, event);
                }

                bool equal(ListenerBase const & r) const
                {
                    return equal_ == r.equal_ && equal_(*this, r);
                }

                void free() const
                {
                    free_(*this);
                }

            private:
                typedef void (*call_t)(
                    ListenerBase const & listener, 
                    Observable const & ob,
                    Event const & event);

                typedef bool (*equal_t)(
                    ListenerBase const & listener1, 
                    ListenerBase const & listener2);

                typedef void (*free_t)(
                    ListenerBase const & listener);

            protected:
                ListenerBase(
                    call_t call, 
                    equal_t equal, 
                    free_t free)
                    : call_(call)
                    , equal_(equal)
                    , free_(free)
                {
                }

                call_t call_;
                equal_t equal_;
                free_t free_;
            };

            template <typename Listener>
            class ListenerT
                : public ListenerBase
            {
            public:
                ListenerT(
                    Listener const & h)
                    : ListenerBase(&ListenerT::call, &ListenerT::equal, &ListenerT::free)
                    , listener_(h)
                {
                }

            private:
                static void call(
                    ListenerBase const & listener, 
                    Observable const & ob,
                    Event const & event)
                {
                    ListenerT const & h = static_cast<ListenerT const &>(listener);
                    h.listener_(ob, event);
                }

                static bool equal(
                    ListenerBase const & listener1, 
                    ListenerBase const & listener2)
                {
                    ListenerT const & h1 = static_cast<ListenerT const &>(listener1);
                    ListenerT const & h2 = static_cast<ListenerT const &>(listener2);
                    return function_equal(h1.listener_, h2.listener_);
                }

                static void free(
                    ListenerBase const & listener)
                {
                    ListenerT const & h = static_cast<ListenerT const &>(listener);
                    delete &h;
                }
                
            private:
                Listener listener_;
            };

        private:
            template <typename Listener>
            static ListenerT<Listener> * create(
                Listener const & listener)
            {
                return new ListenerT<Listener>(listener);
            }

        private:
            std::auto_ptr<ListenerBase> listener_;
        };

    } // namespace event
} // namespace util

#endif // _UTIL_EVENT_EVENT_LISTENER_H_
