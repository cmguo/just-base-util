// Event.cpp

#include "util/Util.h"
#include "util/event/Event.h"
#include "util/event/Event.h"

#include <boost/function_equal.hpp>

namespace util
{
    namespace event
    {

        void Event::on(
            Listener const & l)
        {
            listeners_.push_back(l);
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

        void Event::un(
            Listener const & l)
        {
            listeners_.remove_if(ListenerEqual(l));
        }

        void Event::raise(
            Observable const & sender) const
        {
            for (std::list<Listener>::const_iterator i = listeners_.begin(); i != listeners_.end();) {
                (*i++)(sender, *this);
            }
        }

    } // namespace event
} // namespace util
