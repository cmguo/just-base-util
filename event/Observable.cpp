// Observable.cpp

#include "util/Util.h"
#include "util/event/Observable.h"
#include "util/event/Event.h"

#include <boost/function_equal.hpp>

namespace util
{
    namespace event
    {

        void Observable::on(
            EventId const & e, 
            Listener const & l)
        {
            std::list<Listener> & lst = listeners_[&e];
            lst.push_back(l);
        }

        struct ListenerEqual
        {
        public:
            ListenerEqual(
                Listener const & r)
                : r_(r)
            {
            }

            bool operator()(
                Listener const & l)
            {
                return memcmp(&l, &r_, sizeof(l)) == 0;
            }

        private:
            Listener const & r_;
        };

        void Observable::un(
            EventId const & e, 
            Listener const & l)
        {
            std::list<Listener> & lst = listeners_[&e];
            lst.remove_if(ListenerEqual(l));
        }

        void Observable::raise(
            Event const & e)
        {
            std::list<Listener> & lst = listeners_[e.id_];
            for (std::list<Listener>::const_iterator i = lst.begin(); i != lst.end();) {
                (*i++)(e);
            }
        }

    } // namespace event
} // namespace util
