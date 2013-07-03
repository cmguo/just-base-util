// Observable.cpp

#include "util/Util.h"
#include "util/event/Observable.h"
#include "util/event/Event.h"

namespace util
{
    namespace event
    {

        void Observable::raise(
            Event const & event) const
        {
            event.raise(*this);
        }

    } // namespace event
} // namespace util
