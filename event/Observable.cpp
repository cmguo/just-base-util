// Observable.cpp

#include "util/Util.h"
#include "util/event/Observable.h"
#include "util/event/Event.h"

namespace util
{
    namespace event
    {

        static std::map<std::string, Observable *> & ob_map()
        {
            static std::map<std::string, Observable *> s_map;
            return s_map;
        }

        Observable::Observable(
            std::string const & name)
        {
            ob_map()[name] = this;
        }

        Observable & Observable::get(
            std::string const & name)
        {
            std::map<std::string, Observable *> const & s_map(ob_map());
            std::map<std::string, Observable *>::const_iterator iter = 
                s_map.find(name);
            if (iter == s_map.end()) {
                static Observable ob("");
                return ob;
            } else {
                return *iter->second;
            }
        }

        void Observable::register_event(
            Event & event)
        {
            events_[event.name()] = &event;
        }

        void Observable::raise(
            Event const & event) const
        {
            event.raise(*this);
        }

    } // namespace event
} // namespace util
