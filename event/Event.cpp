// Event.cpp

#include "util/Util.h"
#include "util/event/Event.h"
#include "util/event/Event.h"

#include <boost/function_equal.hpp>

#include <algorithm>

namespace util
{
    namespace event
    {

        Event::Event(
            char const * name)
            : name_(name)
        {
        }

        void Event::on(
            EventListener const & l)
        {
            assert(std::find(listeners_.begin(), listeners_.end(), l) == listeners_.end());
            listeners_.push_front(l);
        }

        void Event::un(
            EventListener const & l)
        {
            listeners_.remove(l);
        }

        std::string Event::get_value(
            std::string const & key) const
        {
            return "";
        }

        void Event::raise(
            Observable const & sender) const
        {
            for (std::list<EventListener>::const_iterator i = listeners_.begin(); i != listeners_.end();) {
                (*i++)(sender, *this);
            }
        }

    } // namespace event
} // namespace util
